/*
 * Arduino Web Weather.
 * Collect data from a temperature and humidity sensor, store
 * them in a CSV file on an SD card and serve it over the web.
 *
 * Copyright (c) 2012 Davide Alberani <da@erlug.linux.it>
 *
 * Requirements:
 * - RTClib:
 *   https://github.com/adafruit/RTClib
 *   or
 *   https://github.com/alberanid/RTClib
 * - Arduino DHT22:
 *   https://github.com/nethoncho/Arduino-DHT22
 *   or
 *   https://github.com/alberanid/Arduino-DHT22
 *
 * Output:
 * data (temperature in Celsius degrees, humidity in percentual) are
 * saved in a file on the SD card (data_log.csv, by default) in this
 * format:
 *    dateTime,temperature,humidity,exitCode
 * where dateTime is in ISO8601 format (space separated) and the exitCode
 * is the return of the sensor reading (0 for success).
 *
 * TODO /ideas:
 * - add another sensors.
 * - easier to customize CSV lines.
 * - serve a web page and display data on a paginated grid (using a
 *   javascript from another server).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <SPI.h>
#include <Wire.h>  
#include <DHT22.h>

#include <SD.h>
#include <Dhcp.h>
#include <EthernetServer.h>
#include <Ethernet.h>

#include <RTClib.h>


// Set the MAC and IP addresses of your Ethernet Shield.
byte mac[] = {0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA};
IPAddress ip(192, 168, 0, 42);

// Pin used to communicate with the SD Card.
const int SD_PIN = 4;

// TCP port on which the web server is listening.
const int HTTP_PORT = 80;

// Digital pin of the DHT22/SHT15/RHT03 temperature
// and humidity sensor (to be connected to its SDA pin).
const int DHT22_PIN = 7;

// Collect information each X ms.
const unsigned long SENSE_DELAY = 60000L;
// Check if there are web requests every X ms.
const int HTTP_SERVE_DELAY = 500;
// Filename used to save the data (opened in append mode).
char* dataLogFile = "/data_log.csv";


/* NOTHING TO CONFIGURE BELOW: GO AWAY! */


// Last time data were read and last time we've check
// if there were web requests.
unsigned long lastDataReadTime, lastWebCheckTime = millis();

// Setup a DHT22 instance.
DHT22 myDHT22(DHT22_PIN);

// Real Time Clock instance.
RTC_DS1307 RTC;

// Web server instance.
EthernetServer server(HTTP_PORT);

// Store the datetime.
char dt[20] = "";


void setup() {
  Serial.begin(9600);
  if (!SD.begin(SD_PIN)) {
    Serial.println("unable to open microSD card");
  }
  // Uncomment to remove the file at every boot.
  //SD.remove(dataLogFile);
  Ethernet.begin(mac, ip);
  server.begin();
  Wire.begin();
  RTC.begin();

  // Uncomment to adjust the RTC using the
  // host date and time.
  //RTC.adjust(DateTime(__DATE__, __TIME__));

  // Wait some time, since the humidity and
  // temperature sensor is slow to start. :-)
  delay(4000);
}


void loop() {
  unsigned long int currentTime = millis();
  
  if ((currentTime - lastWebCheckTime) > HTTP_SERVE_DELAY) {
    lastWebCheckTime = currentTime;
    handleHTTP();
  }
  if ((currentTime - lastDataReadTime) < SENSE_DELAY) {
    return;
  }
  lastDataReadTime = currentTime;

  DHT22_ERROR_t errorCode = myDHT22.readData();
  int temp = myDHT22.getTemperatureCInt();
  int humidity = myDHT22.getHumidityInt();

  char buf[48];
  updateDatetime();
  /* Create the entry that will be appended at the end
     of the file. */
  sprintf(buf, "%s,%hi.%01hi,%i.%01i,%d\n",
          dt, temp / 10, abs(temp % 10),
          humidity / 10, humidity % 10, errorCode);
  Serial.println(String(buf));
  File dataLog = SD.open(dataLogFile, FILE_WRITE);
  dataLog.write(buf);
  dataLog.close();
}


/* Update the dt global variable with the current datetime. */
void updateDatetime() {
  DateTime now = RTC.now();

  sprintf(dt, "%d-%02d-%02d %02d:%02d:%02d",
      now.year(), now.month(), now.day(),
      now.hour(), now.minute(), now.second());
  dt[19] = '\0';
}


/* Serve the CSV file to the client. */
boolean serveCSV(EthernetClient client) {
  client.println("HTTP/1.0 200 OK");
  client.println("Content-Type: text/csv");
  client.println("Connnection: close");
  client.println("Content-disposition: attachment;filename=humidity-temp.csv");
  client.println();
  File dl = SD.open(dataLogFile, FILE_READ);
  char c = dl.read();
  while (dl.available()) {
    if (c) {
      client.write(c);
    } else {
      break;
    }
    c = dl.read();
  }
  dl.close();
  return true;
}


/* Check if we've to reply to an HTTP request.
   Portions of code take from http://arduino.cc/en/Tutorial/WebServer */
boolean handleHTTP() {
  EthernetClient client = server.available();
  if (!client) {
    return false;
  }
  boolean blankLine = true;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      // Blank line after a newline; it's safe to reply.
      if (c == '\n' && blankLine) {
        serveCSV(client);
        break;
      }
      if (c == '\n') {
        blankLine = true;
      } else if (c != '\r') {
        blankLine = false;
      }
    }
  }
  // It seems to be a good idea, for some reason...
  delay(1);
  client.stop();
  return true;
}


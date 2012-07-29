Arduino Web Weather
===================

Arduino Web Weather collects data from a temperature and humidity sensor, stores them in a CSV file on an SD card and serves this file over the web.


Electronic
----------

Minimum requirements are a DTH22 (or equivalent) temperature
and humidity sensor, an Ethernet Shield and an RTC module.
Oh, an Arduino UNO will help. :-P

I've used:
  https://www.sparkfun.com/products/10167 \\
  https://www.sparkfun.com/products/99 \\
  https://www.sparkfun.com/products/9026

The web-weather.fzz file contains the Fritzing circuit diagram (in the __media__ directory, a jpeg export of the diagram).

Software
--------

Just copy the __web\_weather__ directory into your ~/sketchbook directory.
Most of it is commented, if you can understand my English.

Multimedia
----------

In the media directory there are some pictures that may help you build the project.

How it works
============

 - copy the web\_weather directory into your ~/sketchbook directory
 - it needs these libraries: *RTClib* ( https://github.com/alberanid/RTClib ) and *DHT22* ( https://github.com/alberanid/Arduino-DHT22 )
 - put a microSD card in the slot of the Ethernet Shield.
 - modify the __web\_weather.ino__ file accordingly to your needs.
 - upload the program to your Arduino UNO board.
 - connect the ethernet cable to your router or switch.
 - fetch the data with something like: wget http://192.168.0.42/ -O humidity-temp.csv

Author
======

Davide Alberani <da@erlug.linux.it> (C) 2012
http://www.mimante.net/

License
=======

The code is covered by the GPL 3 or later license.
The pictures and the circuit diagram are covered by a by-sa/3.0
creative commons license.


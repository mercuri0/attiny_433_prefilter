you be warned: this could potentionally damage your raspberry pi and also other hardware. i take no responsability for anny damages! try at your own risk!

433 rf filter howto:

1) install avrgcc:
	sudo apt-get install gcc-avr avr-libc

2) install modified avrdude:
	wget http://project-downloads.drogon.net/files/avrdude_5.10-4_armhf.deb
	wget http://project-downloads.drogon.net/files/avrdude-doc_5.10-4_all.deb
	sudo dpkg -i avrdude_5.10-4_armhf.deb
	sudo dpkg -i avrdude-doc_5.10-4_all.deb

3) compile and program attiny:
	make all

*)calculate fuses:
	http://www.engbedded.com/fusecalc/

*)pinout:

name       |       RPI       | ATTINY45 | 433RECEIVER
           |                 |          |
MOSI       |       19        |    5     |      -
MISO       |       21        |    6     |      -
SCK        |       23        |    7     |      -
RESET      |       24        |    1     |      -
PI_IN      | see pilight cfg |    3     |      -
REC_OUT    |       -         |    2     |   DATA_OUT


*) to change the pins create an .avrduderc file in your home directory containing (the numbering is wiringpi numbering!):

programmer
  id    = "gpio";
  desc  = "Use sysfs interface to bitbang GPIO lines";
  type  = gpio;
  reset = 8;
  sck   = 11;
  mosi  = 10;
  miso  = 9;
;

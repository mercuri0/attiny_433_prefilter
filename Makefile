TARGET=433_prefilter_v1

MCU=attiny45
AVRDUDEMCU=t45
CC=/usr/bin/avr-gcc
CFLAGS= -mmcu=$(MCU) -DF_CPU=16000000UL
OBJ2HEX=/usr/bin/avr-objcopy
AVRDUDE=avrdude

program : $(TARGET).hex
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -P gpio -c gpio -b 10000 -U flash:w:$(TARGET).hex

compile :
	$(CC) $(CFLAGS) -c $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).o
	$(OBJ2HEX) -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

program_wd : $(TARGET)_wd.hex
	sudo $(AVRDUDE) -p $(AVRDUDEMCU) -P gpio -c gpio -b 10000 -U flash:w:$(TARGET)_wd.hex

compile_wd :
	$(CC) $(CFLAGS) -c $(TARGET)_wd.c
	$(CC) $(CFLAGS) -o $(TARGET)_wd.elf $(TARGET)_wd.o
	$(OBJ2HEX) -j .text -j .data -O ihex $(TARGET)_wd.elf $(TARGET)_wd.hex

clean :
	rm -f *.hex *.obj *.o *.elf

all: clean compile erase program

all_wd: clean compile_wd erase_wd program_wd

############################## ATTiny25/45/85 ###############################
#	http://www.engbedded.com/fusecalc/
#	http://www.atmel.com/images/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf 
#ATMega*5 FUSE_L (Fuse low byte):
# 0xef = 1 1 1 0   1 1 1 1
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (clock selection -> crystal @ 12 MHz)
#        | |  +--------------- SUT 1..0 (BOD enabled, fast rising power)
#        | +------------------ CKOUT (clock output on CKOUT pin -> disabled)
#        +-------------------- CKDIV8 (divide clock by 8 -> don't divide)
# ATMega*5 FUSE_H (Fuse high byte):
# 0xdd = 1 1 0 1   1 1 0 1
#        ^ ^ ^ ^   ^ \-+-/ 
#        | | | |   |   +------ BODLEVEL 2..0 (brownout trigger level -> 2.7V)
#        | | | |   +---------- EESAVE (preserve EEPROM on Chip Erase -> not preserved)
#        | | | +-------------- WDTON (watchdog timer always on -> disable)
#        | | +---------------- SPIEN (enable serial programming -> enabled)
#        | +------------------ DWEN (debug wire enable)
#        +-------------------- RSTDISBL (disable external reset -> enabled)
#
# lfuse:w:0xe1:m = 16MHz internal pll clock
#
erase:
	# 16 MHz:
	$(AVRDUDE) -p $(AVRDUDEMCU) -P gpio -c gpio -e -U lfuse:w:0xe1:m -U hfuse:w:0xdf:m

erase_wd:
	# 16 MHz, Watchdot enabled, Brown out 1.8V:
	$(AVRDUDE) -p $(AVRDUDEMCU) -P gpio -c gpio -e -U lfuse:w:0xe1:m -U hfuse:w:0xce:m -U efuse:w:0xff:m

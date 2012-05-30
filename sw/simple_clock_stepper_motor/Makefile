# AVR-GCC Makefile
PROJECT = simple_clock_stepper_motor
SOURCES = simple_clock_stepper_motor.c
CC = avr-gcc
OBJCOPY = avr-objcopy
MMCU = atmega328p
DUDE_MMCU = m328p
DF_CPU = 16000000UL
OPTMZ = -Os

CFLAGS = $(OPTMZ) -DF_CPU=$(DF_CPU) -mmcu=$(MMCU) -Wall -Werror


$(PROJECT).hex: $(PROJECT).out 
	$(OBJCOPY) -j .text -O ihex -R .eeprom $(PROJECT).out $(PROJECT).hex

$(PROJECT).out: $(SOURCES)
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)

upload: $(PROJECT).hex
	avrdude -v -p $(DUDE_MMCU) -c avrispmkII -P usb -U flash:w:$(PROJECT).hex

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex



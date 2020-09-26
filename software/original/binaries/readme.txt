avrdude -c usbasp -p m328p -V -U lfuse:w:0xff:m -U hfuse:w:0xdb:m -U efuse:w:0xfd:m
avrdude -c usbasp -p m328p -U flash:w:<name>.hex

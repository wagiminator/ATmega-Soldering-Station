avrdude -c usbtiny -p m328p -V -U lfuse:w:0xff:m -U hfuse:w:0xdb:m -U efuse:w:0xfd:m
avrdude -c usbtiny -p m328p -U flash:w:<name>.hex

re2: rotary encoder with 2 increments/step
re4: rotary encoder with 4 increments/step

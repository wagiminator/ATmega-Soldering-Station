*重要提示 Important note

版本号含有"t"的代表是测试版本，不稳定不推荐上传
A version number with a "t" indicates a test version, and unstable uploads are not recommended

版本号含有多个"+" 代表是改进版本
The version number contains multiple "+" to indicate an improved version

Upload:
固件上传指令
avrdude -c usbtiny -p m328p -V -U lfuse:w:0xff:m -U hfuse:w:0xdb:m -U efuse:w:0xfd:m
avrdude -c usbtiny -p m328p -U flash:w:<name>.hex

Libraries:
用到的库文件
https://github.com/createskyblue/Arduboy2/
https://github.com/mblythe86/C-PID-Library/tree/master/PID_v1

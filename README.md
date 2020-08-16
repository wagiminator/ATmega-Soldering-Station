# T12 Soldering Station based on ATmega328p

!!! ERROR in the current revision: DO NOT USE D4 18V ZENER OR INPUT VOLTAGE MORE THAN 19V !!!

# 1. Overview #

T12 Quick Heating Soldering Station featuring
- Temperature measurement of the tip
- Temperature control via rotary encoder
- Boost mode by short pressing rotary encoder switch
- Setup menu by long pressing rotary encoder switch
- Handle movement detection (by checking ball switch)
- Iron unconnected detection (by idenfying invalid temperature readings)
- Time driven sleep/power off mode if iron is unused (movement detection)
- Measurement of input voltage, Vcc and ATmega's internal temperature
- Information display on OLED
- Buzzer
- Calibrating and managing different soldering tips
- Storing user settings into the EEPROM

Project Video: https://youtu.be/I9ATDxvQ1Bc

Video from John Glavinos (electronics4all): https://youtu.be/4YDcWfOQmz4

Project Files at EasyEDA: https://easyeda.com/wagiminator/z-solderingstation-smd-v2

# Soldering Station v2.0: #

![IMG_20190731_190146_x.jpg](https://image.easyeda.com/pullimage/iSYD3M8FjNd8ndt1cC7noTcUAdRtz9oFH6hKmoNl.jpeg)

![IMG_20190731_182904_x.jpg](https://image.easyeda.com/pullimage/kgryPQpReu7YGh16ewXIt2tQha6rTmhtARw9nPZh.jpeg)

# Soldering Station v2.5: #

![IMG_20200621_132138_x2.jpg](https://image.easyeda.com/pullimage/8LPUnbOCw2uFf1ozHYkTESbvH1LjlA1zKstsrS5v.jpeg)

![IMG_20200621_121350_x.jpg](https://image.easyeda.com/pullimage/5ZW6mQs1im9htCg3NJSryApeGUiSlwTFXnvxJSLq.jpeg)

# Program v1.6L UI Upgrade

1. Beautify the detail page and increase the power bar
2. Menus have ICONS
3. Better scrolling animation
4. Get rid of the U8g graphics library and use the ArduBoy graphics Library (Lite) to improve the frame rate and reduce memory usage to make graphics possible
5. Add flip display Settings to adapt to the habits of different people
6. Add multilingual support for Chinese and English

![](https://image.lceda.cn/pullimage/dD94Lz6baKfVFCMcqzb7acjT7sRY7NwJ7NaJmOt9.gif)

![](https://image.lceda.cn/pullimage/cfhvmAOQPhwq7iCPhLvNDeRbOcbccIucCHhU2Abj.gif)

![](https://image.lceda.cn/pullimage/ij3I4sFpNrQPefPdkgIqGjraDEWEPJYjb4974HlC.gif)

![](https://image.lceda.cn/pullimage/OsLI0MkxMrv7w0GgVg1tZPt6CMrcyhoFSeOnWrzp.gif)

# 2. Power Supply Specification Requirements #

Choose a power supply with an output voltage between 16V and 24V which can provide an output current according to the table below. The power supply must be well stabilized. The current and power is determined by the resistance (R = 8 Ohm) of the heater.

|Voltage (U)|Current (I) = U / R|Power (P) = U² / R|
|-|-|-|
|16 V|2.00 A|32 W|
|17 V|2.13 A|36 W|
|18 V|2.25 A|41 W|
|19 V|2.38 A|45 W|
|20 V|2.50 A|50 W|
|21 V|2.63 A|55 W|
|22 V|2.75 A|61 W|
|23 V|2.88 A|66 W|
|24 V|3.00 A|72 W|

# 3. Temperature Measurement and OpAmp Considerations #

A thermocouple (temperature sensor) is located in the T12 soldering tip. It creates a very small voltage depending on the temperature difference between the hot end and the cold junction (about 40 microvolts per degree Celsius). To measure this, the heater must be switched off since both share the same connections. The low voltage is amplified by the OpAmp and measured by the ADC of the microcontroller. The LMV358 is a very cheap and versatile OpAmp, but not the ideal choice for this task because it has a fairly high input offset voltage and is quite noisy. Although the SolderingStation also works with this OpAmp thanks to the software's smoothing and calibration algorithms, I highly recommend spending a little more money in a better one. The OPA2330AIDR or OPA2333AIDR for instance have the same pinout and can also be used with this board. They provide significantly more accurate and stable temperature measurements.

![SolderingStationV2_LMV358_Noise.png](https://image.easyeda.com/pullimage/Q1peGd04fhp8kQzYYe6WfFqgOCv8MpL39cS0y2rr.png)
![SolderingStationV2_OPA2330_Noise.png](https://image.easyeda.com/pullimage/w2mQcD786pNi5hLEd5bqK2hezrsrUUhtttMRZYih.png)
![SolderingStationV2_LMV358_SettleTime_2.png](https://image.easyeda.com/pullimage/YyUQY5VyZgBz92o5gpugZDDaUfl1TFoNJoGNMEi2.png)
![SolderingStationV2_OPA2330_SettleTime.png](https://image.easyeda.com/pullimage/08cMT2owQsOTnGl71miryML8imI4eP9j3chbK31M.png)

# 4. Heater High Side Switch with N-Channel MOSFET and Charge Pump #

The microcontroller switches the heater on and off via the MOSFET. Since the temperature measurement must be done over the same line and against
ground, the MOSFET has to be placed between the supply voltage and the heater (high-side switch). A P-Channel MOSFET is normally used for this configuration. However, N-Channel MOSFETs usually have a lower resistance (RDS (on)), in the case of the IRLR7843 only 3 milliohms. A low resistance means a higher efficiency and a lower heat development of the MOSFET. For an N-channel MOSFET to function as a high-side switch, an additional circuit is required to maintain a positive GATE-TO-SOURCE voltage after the MOSFET is switched on. This is done using a so-called charge pump consisting of a capacitor and a diode.

![SolderingStationV2_ChargePump.png](https://image.easyeda.com/pullimage/UwAxl6ON0Pn9orkxrE9tfgfZAGFxWH5Xc9pkDvmm.png)

# 5. Building Instructions #

In addition to the components for the PCB you will need the following:

- 3D-printed case
- Aviator Plug (4- or 5-pin depending on your iron handle)
- DC Power Jack (5.5 * 2.1 mm)
- Rocker Switch (KCD1 15 * 10 mm)
- Some wires
- 4 Self-tapping screws (2.3 * 5 mm)

![parts.jpg](https://image.easyeda.com/pullimage/A29gEDpw2Ld6S96m6f88GDKLpD0YJHm0m3R5Tp5r.jpeg)
![IMG_20200621_130414_x.jpg](https://image.easyeda.com/pullimage/NQZMhCwVRTmA4HVYrDjr6y938flXDBzT6woWXocX.jpeg)

Make sure that all parts fit nicely into the case. Solder the wires to the connectors and protect them with heat shrinks. Use thick wires (AWG18) for the power connections. Make all connections according to the schematic down below. Solder the wires directly to the corresponding pads on the pcb. Upload the firmware and screw the pcb on top of the case.

![connections.png](https://image.easyeda.com/pullimage/q0JfyFSQSTueZhpBkhrBhFbxtL1UqRARK1nrKThv.png)

# 6. License #

![license.png](https://i.creativecommons.org/l/by-sa/3.0/88x31.png)

This work is licensed under Creative Commons Attribution-ShareAlike 3.0 Unported License. 
(http://creativecommons.org/licenses/by-sa/3.0/)

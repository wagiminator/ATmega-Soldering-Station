# T12 Soldering Station based on ATmega328p

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

Video from John Glavinos (electronics4all): https://youtu.be/4YDcWfOQmz4

![IMG_20190731_190146_x.jpg](https://image.easyeda.com/pullimage/iSYD3M8FjNd8ndt1cC7noTcUAdRtz9oFH6hKmoNl.jpeg)

![IMG_20190731_182904_x.jpg](https://image.easyeda.com/pullimage/kgryPQpReu7YGh16ewXIt2tQha6rTmhtARw9nPZh.jpeg)

# 2. Temperature Measurement and OpAmp Considerations #

A thermocouple (temperature sensor) is located in the T12 soldering tip. It creates a very small voltage depending on the temperature difference between the hot end and the cold junction (about 40 microvolts per degree Celsius). To measure this, the heater must be switched off since both share the same connections. The low voltage is amplified by the OpAmp and measured by the ADC of the microcontroller. The LMV358 is a very cheap and versatile OpAmp, but not the ideal choice for this task because it has a fairly high input offset voltage and is quite noisy. This noise cannot be smoothed out by a capacitor, because this would also significantly extend the time between the heater being switched off and the temperature measurement. Although the SolderingStation also works with this OpAmp thanks to the software's smoothing and calibration algorithms, I highly recommend spending a little more money in a better one. The OPA2330AIDR for instance has the same pinout and can also be used with this board. It provides significantly more accurate and stable temperature measurements.


![SolderingStationV2_LMV358_Noise.png](https://image.easyeda.com/pullimage/Q1peGd04fhp8kQzYYe6WfFqgOCv8MpL39cS0y2rr.png)
![SolderingStationV2_OPA2330_Noise.png](https://image.easyeda.com/pullimage/w2mQcD786pNi5hLEd5bqK2hezrsrUUhtttMRZYih.png)
![SolderingStationV2_LMV358_SettleTime_2.png](https://image.easyeda.com/pullimage/YyUQY5VyZgBz92o5gpugZDDaUfl1TFoNJoGNMEi2.png)
![SolderingStationV2_OPA2330_SettleTime.png](https://image.easyeda.com/pullimage/08cMT2owQsOTnGl71miryML8imI4eP9j3chbK31M.png)

# 3. Heater High Side Switch with N-Channel MOSFET and Charge Pump #

The microcontroller switches the heater on and off via the MOSFET. Since the temperature measurement must be done over the same line and against
ground, the MOSFET has to be placed between the supply voltage and the heater (high-side switch). A P-Channel MOSFET is normally used for this configuration. However, N-Channel MOSFETs usually have a lower resistance (RDS (on)), in the case of the IRLR7843 only 3 milliohms. A low resistance means a higher efficiency and a lower heat development of the MOSFET. For an N-channel MOSFET to function as a high-side switch, an additional circuit is required to maintain a positive GATE-TO-SOURCE voltage even when the MOSFET is switched on. This is done using a so-called charge pump consisting of a capacitor and a diode.

![SolderingStationV2_ChargePump.png](https://image.easyeda.com/pullimage/UwAxl6ON0Pn9orkxrE9tfgfZAGFxWH5Xc9pkDvmm.png)

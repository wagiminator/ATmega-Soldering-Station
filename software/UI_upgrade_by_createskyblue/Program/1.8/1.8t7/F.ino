void(* resetFunc) (void) = 0;
// reads current rotary encoder value
int getRotary() {
  return (Count >> ROTARY_TYPE);
}

//检查按键
void CheckLastButton() {
  if (lastbutton && digitalRead(BUTTON_PIN)) {
    delay(10);
    lastbutton = false;
  }
}
//返回对应层数的对象数
byte QueryMenuObject() {
  int SUM = 0;
  for (int i = 0; i < (MenuLevel); i++) SUM += Menu_table[i];
  return SUM;
}


// average several ADC readings in sleep mode to denoise
uint16_t denoiseAnalog (byte port) {
  uint16_t result = 0;
  ADCSRA |= bit (ADEN) | bit (ADIF);    // enable ADC, turn off any pending interrupt
  if (port >= A0) port -= A0;           // set port and
  ADMUX = (0x0F & port) | bit(REFS0);   // reference to AVcc
  set_sleep_mode (SLEEP_MODE_ADC);      // sleep during sample for noise reduction
  for (uint8_t i = 0; i < 32; i++) {    // get 32 readings
    sleep_mode();                       // go to sleep while taking ADC sample
    while (bitRead(ADCSRA, ADSC));      // make sure sampling is completed
    result += ADC;                      // add them up
  }
  bitClear (ADCSRA, ADEN);              // disable ADC
  return (result >> 5);                 // devide by 32 and return value
}

// get internal temperature by reading ADC channel 8 against 1.1V reference
float getChipTemp() {
  uint16_t result = 0;
  ADCSRA |= bit (ADEN) | bit (ADIF);    // enable ADC, turn off any pending interrupt
  ADMUX = bit (REFS1) | bit (REFS0) | bit (MUX3); // set reference and mux
  delay(10);                            // wait for voltages to settle
  set_sleep_mode (SLEEP_MODE_ADC);      // sleep during sample for noise reduction
  for (uint8_t i = 0; i < 32; i++) {    // get 32 readings
    sleep_mode();                       // go to sleep while taking ADC sample
    while (bitRead(ADCSRA, ADSC));      // make sure sampling is completed
    result += ADC;                      // add them up
  }
  bitClear (ADCSRA, ADEN);              // disable ADC
  result >>= 2;                         // devide by 4
  return ((result - 2594) / 9.76);      // calculate internal temperature in degrees C
}

// get input voltage in mV by reading 1.1V reference against AVcc
uint16_t getVCC() {
  uint16_t result = 0;
  ADCSRA |= bit (ADEN) | bit (ADIF);    // enable ADC, turn off any pending interrupt
  // set Vcc measurement against 1.1V reference
  ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
  delay(1);                             // wait for voltages to settle
  set_sleep_mode (SLEEP_MODE_ADC);      // sleep during sample for noise reduction
  for (uint8_t i = 0; i < 16; i++) {    // get 16 readings
    sleep_mode();                       // go to sleep while taking ADC sample
    while (bitRead(ADCSRA, ADSC));      // make sure sampling is completed
    result += ADC;                      // add them up
  }
  bitClear (ADCSRA, ADEN);              // disable ADC
  result >>= 4;                         // devide by 16
  return (1125300L / result);           // 1125300 = 1.1 * 1023 * 1000
}


// get supply voltage in mV
uint16_t getVIN() {
  long result;
  result = denoiseAnalog (VIN_PIN);     // read supply voltage via voltage divider
  return (result * Vcc / 179.474);      // 179.474 = 1023 * R13 / (R12 + R13)
}

//ADC中断服务
// ADC interrupt service routine
EMPTY_INTERRUPT (ADC_vect);             // nothing to be done here

//旋转编码器触发中断
// Pin change interrupt service routine for rotary encoder
ISR (PCINT0_vect) {
  uint8_t a = PINB & 1;
  uint8_t b = PIND >> 7 & 1;
  if (a != a0) {              // A changed
    a0 = a;
    if (b != b0) {            // B changed
      b0 = b;
      Count = constrain(Count + ((a == b) ? CountStep : -CountStep), CountMin, CountMax);
      if (ROTARY_TYPE && ((a == b) != ab0)) {
        Count = constrain(Count + ((a == b) ? CountStep : -CountStep), CountMin, CountMax);;
      }
      ab0 = (a == b);
      handleMoved = true;
    }
  }
}

//计算实际温度
// calculates real temperature value according to ADC reading and calibration values
float calculateTemp(float t) {
  return PTemp[0] + RawTemp * PTemp[1] + RawTemp * RawTemp * PTemp[2] + RawTemp * RawTemp * RawTemp * PTemp[3];
}

/*
 * 温度控制
 * mode-> 0:以实际温度数值为基准 1:以ADC数值为基准(非PID模式)
*/
void Thermostat(bool mode) {
  // define Setpoint acoording to current working mode
  if      (InOffMode)   Setpoint = 0;
  else if (InSleepMode) Setpoint = SleepTemp;
  else if (InBoostMode) Setpoint = SetTemp + BoostTemp;
  else                  Setpoint = SetTemp;

  // control the heater (PID or direct)
  if (!mode) {
    gap = abs(Setpoint - CurrentTemp);
    if (PIDenable) {
      Input = CurrentTemp;
      if (gap < 30) ctrl.SetTunings(consKp, consKi, consKd);
      else ctrl.SetTunings(aggKp, aggKi, aggKd);
      ctrl.Compute();
    } else {
      // turn on heater if current temperature is below setpoint
      if ((CurrentTemp + 0.5) < Setpoint) Output = 0; else Output = 255;
    }
  } else {
    gap = 0;  //此模式下不计算温度差
    if ((RawTemp + 0.5) < Setpoint) Output = 0; else Output = 255;
  }
  // set heater PWM
#if UsePMOS
  analogWrite(CONTROL_PIN, 255 - Output);
#else
  analogWrite(CONTROL_PIN, Output);
#endif
}


// creates a short beep on the buzzer
void beep() {
  //体谅一下，没内存了，只能用原始的办法驱动无源蜂鸣器，否则我也不想牺牲宝贵响应资源换内存
  if (BeepEnable) {
    for (uint8_t i = 0; i < 255; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delayMicroseconds(125);
      digitalWrite(BUZZER_PIN, LOW);
      delayMicroseconds(125);
    }
  }
}

//设置旋转编码器的起始值
// sets start values for rotary encoder
void setRotary(int rmin, int rmax, int rstep, int rvalue) {
  CountMin  = rmin << ROTARY_TYPE;
  CountMax  = rmax << ROTARY_TYPE;
  if (RotaryD) CountStep = rstep; else CountStep = -rstep;
  Count     = rvalue << ROTARY_TYPE;
}

// reads temperature, vibration switch and supply voltages
void SENSORCheck() {
  // shut off heater in order to measure temperature
#if UsePMOS
  analogWrite(CONTROL_PIN, 0);
#else
  analogWrite(CONTROL_PIN, 255);
#endif
  delayMicroseconds(Time2settle);             // wait for voltage to settle

  uint16_t temp = denoiseAnalog(SENSOR_PIN);  // read ADC value for temperature
  uint8_t d = digitalRead(SWITCH_PIN);        // check handle vibration switch
  if (d != d0) {
    handleMoved = true;  // set flag if handle was moved
    d0 = d;
  }
  if (! SensorCounter--) Vin = getVIN();      // get Vin every now and then

  // turn on again heater

#if UsePMOS
  analogWrite(CONTROL_PIN, 255 - Output);
#else
  analogWrite(CONTROL_PIN, Output);
#endif


  RawTemp += (temp - RawTemp) * SMOOTHIE;     // stabilize ADC temperature reading
  CurrentTemp = calculateTemp(RawTemp);

  // stabilize displayed temperature when around setpoint
  if ((ShowTemp != Setpoint) || (abs(ShowTemp - CurrentTemp) > 5)) ShowTemp = CurrentTemp;
  if (abs(ShowTemp - Setpoint) <= 1) ShowTemp = Setpoint;
  // set state variable if temperature is in working range; beep if working temperature was just reached
  gap = abs(SetTemp - CurrentTemp);
  if (gap < 10) {
    if (!IsWorky && BeepIfWorky) beep();
    IsWorky = true;
    BeepIfWorky = false;
  } else {
    IsWorky = false;
    BeepIfWorky = true;
  }
  // checks if tip is present or currently inserted
  if (ShowTemp > 500) TipIsPresent = false;   // tip removed ?
  if (!TipIsPresent && (ShowTemp < 500)) {    // new tip inserted ?
    //关闭加热
#if UsePMOS
    analogWrite(CONTROL_PIN, 0);
#else
    analogWrite(CONTROL_PIN, 255);
#endif
    beep();                                   // beep for info
    TipIsPresent = true;                      // tip is present now
    ChangeTipScreen();                        // show tip selection screen
    UpdateEEPROM();                           // update setting in EEPROM
    handleMoved = true;                       // reset all timers
    RawTemp  = denoiseAnalog(SENSOR_PIN);     // restart temp smooth algorithm
    c0 = LOW;                                 // switch must be released
    setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, SetTemp);  // reset rotary encoder
  }
}



// check rotary encoder; set temperature, toggle boost mode, enter setup menu accordingly
void ROTARYCheck() {
  // set working temperature according to rotary encoder value
  SetTemp = getRotary();

  // check rotary encoder switch
  uint8_t c = digitalRead(BUTTON_PIN);
  if ( !c && c0 ) {
    beep();
    Buttonmillis = millis();
    while ( (!digitalRead(BUTTON_PIN)) && ((millis() - Buttonmillis) < 500) );
    if ((millis() - Buttonmillis) >= 500) SetupScreen();
    else {
      InBoostMode = !InBoostMode;
      if (InBoostMode) Boostmillis = millis();
      handleMoved = true;
    }
  }
  c0 = c;

  // check timer when in boost mode
  if (InBoostMode && TimeOfBoost) {
    GoneSeconds = (millis() - Boostmillis) / 1000;
    if (GoneSeconds >= TimeOfBoost) {
      InBoostMode = false;              // stop boost mode
      beep();                           // beep if boost mode is over
      BeepIfWorky = true;               // beep again when working temperature is reached
    }
  }
}


// check and activate/deactivate sleep modes
void SLEEPCheck() {
  if (handleMoved) {                    // if handle was moved
    if (InSleepMode) {                  // in sleep or off mode?
      if ((CurrentTemp + 20) < SetTemp) // if temp is well below setpoint

        // then start the heater right now

#if UsePMOS
        analogWrite(CONTROL_PIN, 255);
#else
        analogWrite(CONTROL_PIN, 0);
#endif

      beep();                           // beep on wake-up
      BeepIfWorky = true;               // beep again when working temperature is reached
    }
    handleMoved = false;                // reset handleMoved flag
    InSleepMode = false;                // reset sleep flag
    InOffMode   = false;                // reset off flag
    Sleepmillis = millis();             // reset sleep timer
  }

  // check time passed since the handle was moved
  GoneMinutes = (millis() - Sleepmillis) / 60000;
  if ( (!InSleepMode) && (Time2sleep > 0) && (GoneMinutes >= Time2sleep) ) {
    InSleepMode = true;
    beep();
  }
  if ( (!InOffMode)   && (Time2off   > 0) && (GoneMinutes >= Time2off  ) ) {
    InOffMode   = true;
    beep();
  }
}

void SetOLEDLightLevel(uint8_t level) {
  arduboy.sendLCDCommand(0x81);  //向SSD1306发送指令：设置内部电阻微调
  arduboy.sendLCDCommand(level); //微调范围（0-255）
}

ISR(WDT_vect)  //WDT中断，8秒一次
{
  WDTCSR |= _BV(WDIE);
}

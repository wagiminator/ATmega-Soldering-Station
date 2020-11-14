
// reads user settings from EEPROM; if EEPROM values are invalid, write defaults
void getEEPROM() {
  uint16_t identifier = (EEPROM.read(0) << 8) | EEPROM.read(1);
  if (identifier == EEPROM_IDENT) {
    DefaultTemp = (EEPROM.read(2) << 8) | EEPROM.read(3);
    SleepTemp   = (EEPROM.read(4) << 8) | EEPROM.read(5);
    BoostTemp   =  EEPROM.read(6);
    Time2sleep  =  EEPROM.read(7);
    Time2off    =  EEPROM.read(8);
    TimeOfBoost =  EEPROM.read(9);
    MainScrType =  EEPROM.read(10);
    PIDenable   =  EEPROM.read(11);
    BeepEnable  =  EEPROM.read(12);
    CurrentTip  =  EEPROM.read(13);
    NumberOfTips = EEPROM.read(14);
    LANG = EEPROM.read(15);
    FlipState = EEPROM.read(16);
    UnderVoltage = EEPROM.read(17);
    Password = (EEPROM.read(18) << 8) | EEPROM.read(19);
    RotaryD = EEPROM.read(20);
    uint8_t i, j;
    uint16_t Counter = 21;
    for (i = 0; i < NumberOfTips; i++) {
      for (j = 0; j < TIPNAMELENGTH; j++) {
        TipName[i][j] = EEPROM.read(Counter++);
      }
      //0 8 16 24
      for (j = 0; j < 4; j++) {
        EEPROM.get(Counter+=4,PTemp[i][j]);
      }
    }
  }
  else {
    EEPROM.update(0, EEPROM_IDENT >> 8); EEPROM.update(1, EEPROM_IDENT & 0xFF);
    updateEEPROM();
  }
}


// writes user settings to EEPROM using updade function to minimize write cycles
void updateEEPROM() {
  EEPROM.update( 2, DefaultTemp >> 8);
  EEPROM.update( 3, DefaultTemp & 0xFF);
  EEPROM.update( 4, SleepTemp >> 8);
  EEPROM.update( 5, SleepTemp & 0xFF);
  EEPROM.update( 6, BoostTemp);
  EEPROM.update( 7, Time2sleep);
  EEPROM.update( 8, Time2off);
  EEPROM.update( 9, TimeOfBoost);
  EEPROM.update(10, MainScrType);
  EEPROM.update(11, PIDenable);
  EEPROM.update(12, BeepEnable);
  EEPROM.update(13, CurrentTip);
  EEPROM.update(14, NumberOfTips);
  EEPROM.update(15, LANG);
  EEPROM.update(16, FlipState);
  EEPROM.update(17, UnderVoltage);
  EEPROM.update(18, Password >> 8);
  EEPROM.update(19, Password & 0xFF);
  EEPROM.update(20, RotaryD);
  
  uint8_t i, j;
  uint16_t Counter = 21;
  for (i = 0; i < NumberOfTips; i++) {
    for (j = 0; j < TIPNAMELENGTH; j++) EEPROM.update(Counter++, TipName[i][j]);
    for (j = 0; j < 4; j++) EEPROM.put(Counter+=4, PTemp[i][j]);
  }
}

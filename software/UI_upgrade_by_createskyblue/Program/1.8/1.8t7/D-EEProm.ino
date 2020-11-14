void ViewEEPRom() {
  setRotary(0, 1023, 16, 0);
  lastbutton = (!digitalRead(BUTTON_PIN));
  arduboy.setTextSize(1);
  do {
    arduboy.clear();
    for (byte y = 0; y < 8; y++) {
      for (byte x = 0; x < 2; x++) {
        if (2 * y + x + getRotary() > 1024) break;
        arduboy.setCursor(x * 64, y * 8);
        arduboy.print(2 * y + x + getRotary());
        arduboy.print(F("->"));
        arduboy.print(EEPROM[2 * y + x + getRotary()]);
        arduboy.setCursor(x * 64 + 54, y * 8);
        arduboy.print((char)EEPROM[2 * y + x + getRotary()]);
      }
    }
    CheckLastButton();
    arduboy.display();
  } while (digitalRead(BUTTON_PIN) || lastbutton);
}
/* 从EEPROM读取烙铁头参数
    传入：烙铁头的序号
    作用：把对应序号的烙铁头参数直接载入内存
    原理：
      1 计算地址
      2 读取数据
*/
void GetEEPRomTip(byte n) {
  if (n <= TIPMAX) {
    uint16_t Counter = 21;
    //跳转到目标地址
    Counter += (TIPNAMELENGTH + 1) * n;
    Counter += 5 * 4 * n;
    //从EEPRom读取数据并载入内存
    for (int i = 0; i < (TIPNAMELENGTH + 1); i++) TipName[i] = EEPROM.read(Counter++);
    for (int i = 0; i < 4; i++) EEPROM.get(Counter += 4, PTemp[i]);
  }
}

/* 把烙铁头参数写入EEPRom
    传入：烙铁头的序号
    作用：把对应序号的烙铁头参数从运行内存载入到EEPRom
    原理：
      1 计算地址
      2 写入EEProm
*/
void PutEEPRomTip(byte n) {
  if (n <= TIPMAX) {
    uint16_t Counter = 21;
    //跳转到目标地址
    Counter += (TIPNAMELENGTH + 1) * n;
    Counter += 5 * 4 * n;
    //从EEPRom读取数据并载入内存
    for (int i = 0; i < (TIPNAMELENGTH + 1); i++) EEPROM.update(Counter++, TipName[i]);
    for (int i = 0; i < 4; i++) EEPROM.put(Counter += 4, PTemp[i]);
  }
}

/* 从EEPROM删除某一项烙铁头参数
    传入：烙铁头序号
    作用：把序号后的烙铁头数据位覆盖移到序号的位置，实现删除数据的目的
*/
void DelEEPRomTip(byte n) {
  if (n <= TIPMAX) {
    uint16_t Counter = 21;
    //跳转到目标地址
    Counter += (TIPNAMELENGTH + 1) * n;
    Counter += 5 * 4 * n;
    //开始覆盖移动后面的数据
    byte moveN = NumberOfTips - n - 1;
    while (moveN--) {
      for (int i = Counter; i < Counter + (TIPNAMELENGTH + 1) + 4 * 5; i++) EEPROM[i] = EEPROM[i + (TIPNAMELENGTH + 1) + 4 * 5];
      Counter += (TIPNAMELENGTH + 1) + 4 * 5;
    }
  }
}

// reads user settings from EEPROM; if EEPROM values are invalid, write defaults
void GetEEPROM() {
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
  }
  else {
    EEPROM.update(0, EEPROM_IDENT >> 8); EEPROM.update(1, EEPROM_IDENT & 0xFF);
    UpdateEEPROM();
  }
  GetEEPRomTip(CurrentTip);
}


// writes user settings to EEPROM using updade function to minimize write cycles
void UpdateEEPROM() {
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
  PutEEPRomTip(CurrentTip);
}


// draws the main screen
void MainScreen() {
  if (!(arduboy.nextFrame())) return;
  //状态
  byte SysState;
  if (ShowTemp > 500) SysState = 0;
  else if (InOffMode) {
    SysState = 1;
    DrawIntensiveComputing();
  } else if (InSleepMode) SysState = 2;
  else if (InBoostMode) SysState = 3;
  else if (IsWorky) SysState = 4;
  else if (Output < 180) SysState = 5;
  else SysState = 6;
  if (SysState != 1) {
    if (MainScrType) {
      arduboy.invert(1);
      arduboy.clear();
      //详细信息页
      arduboy.fillRect(0, 0, 128, 64, 1); //白底
      arduboy.setTextSize(1);
      SetTextColor(0);
      //预设名
      //arduboy.drawSlowXYBitmap(0, 0, Tag, 16, 16, 0);
      arduboy.setCursor(2, 4); arduboy.print(TipName);
      //显示状态
      arduboy.setCursor(53, 1);
      arduboy.setTextSize(2);
      switch (LANG) {
        case 0: arduboy.drawSlowXYBitmap(95, 1, S_table[SysState], 28, 14, 0); break;  //中文 Chinese
        default: //英文 English
          switch (SysState) {
            case 1: arduboy.print(F("OFF")); break;
            case 2: arduboy.print(F("SLEEP")); break;
            case 3: arduboy.print(F("BOOST")); break;
            case 4: arduboy.print(F("WORKY")); break;
            case 5: arduboy.print(F("HEAT")); break;
            case 6: arduboy.print(F("HOLD")); break;
            default: arduboy.print(F("ERROR")); break;
          } break;
      }

      //运行状态图标
      arduboy.drawSlowXYBitmap(74, 1, C_table[SysState], 14, 14, 0);
      arduboy.setTextSize(1);

      //显示探头温度
      DrawNumRect(0, 15, 5, ShowTemp);

      //状态条
      DrawStatusBar(0);

      //状态图标 以及 信息
      //自动报警-图标闪烁
      if (getChipTemp() < 80 || ((millis() * 4) / 1000) % 2) {
        arduboy.setCursor(91, 20); arduboy.print(getChipTemp(), 1); arduboy.print((char)248); arduboy.print(F("C")); //芯片温度
        arduboy.drawSlowXYBitmap(42, 1, Chip, 14, 14, 0);
      }
      if ((float)Vin / 100 > UnderVoltage || ((millis() * 4) / 1000) % 2) {
        arduboy.setCursor(91, 28); arduboy.print((float)getVIN() / 1000, 2); arduboy.print(F("V")); //输入电压
        arduboy.drawSlowXYBitmap(58, 1, Lightning, 14, 14, 0);
      }
      arduboy.setCursor(91, 36); arduboy.print((float)getVCC() / 1000, 3); arduboy.print(F("V")); //芯片电压
      arduboy.setCursor(91, 44); arduboy.print(RawTemp, 0); //烙铁头热偶原始数据




      arduboy.display();
      //警报声
      if (getChipTemp() > 80 && ((millis() * 4) / 1000) % 2 || (float)Vin / 100 < UnderVoltage && ((millis() * 4) / 1000) % 2) {
        beep();
        arduboy.invert(0);
      };

    } else {
      //arduboy.invert(1);
      arduboy.setTextSize(6);
      SetTextColor(1);
      arduboy.clear();
      if (getChipTemp() > 80 && ((millis() * 4) / 1000) % 2) beep();
      if ((float)Vin / 100 < UnderVoltage && ((millis() * 4) / 1000) % 2) beep();
      arduboy.setCursor(12, 2);

      if (ShowTemp > 500) arduboy.print(999); else arduboy.print(ShowTemp);
      DrawStatusBar(1);
      arduboy.display();
    }
  }


}
void DrawNumRect(byte x, byte y, byte size, int n) {
  arduboy.setCursor(3 + x, 3 + y);
  arduboy.setTextSize(size);
  if (ShowTemp > 500) arduboy.print(F("000")); else arduboy.print(n);
  arduboy.drawRect(1 + x, 1 + y, 89, 39, 0);
}

//绘制状态条
void DrawStatusBar(bool color) {
  SetTextColor(color);
  arduboy.setTextSize(1);
  //温度条
  //框
  arduboy.drawRect(1, 54, 102, 9, color);
  //条
  if (ShowTemp <= 500) arduboy.fillRect(1, 54, map(ShowTemp, 0, 500, 2, 102), 9, color);
  //指示部分
  arduboy.fillRect(map(Setpoint, 0, 500, 2, 102) - 2, 55, 3, 7, color);
  //先来个白底
  arduboy.fillRect(map(Setpoint, 0, 500, 2, 102) - 3, 63, 5, 1, color);
  arduboy.drawLine(map(Setpoint, 0, 500, 2, 102) - 1, 63, map(Setpoint, 0, 500, 2, 102) - 1, 54, !color);

  //画指示针
  arduboy.drawSlowXYBitmap(map(Setpoint, 0, 500, 2, 102) - 3, 60, Pointer, 5, 4, !color);

  //功率条
  arduboy.drawRect(104, 54, 23, 9, color);
  arduboy.fillRect(104, 54, map(Output, 255, 0, 0, 23), 9, color);

  SetTextColor(!color);
  //////////////进入反色////////////////////////////////
  arduboy.setCursor(2, 55); arduboy.print(Setpoint, 0);//探头预设温度
  arduboy.setCursor(105, 55); arduboy.print(map(Output, 255, 0, 0, 100)); arduboy.print(F("%")); //功率百分比
  SetTextColor(color);
}

//设置画笔颜色
void SetTextColor(bool color) {
  arduboy.setTextBackground(!color);
  arduboy.setTextColor(color);
}


// setup screen
void SetupScreen() {
  arduboy.invert(0);
  //关闭加热
#if UsePMOS
  analogWrite(CONTROL_PIN, 0);
#else
  analogWrite(CONTROL_PIN, 255);
#endif
  beep();
  uint16_t SaveSetTemp = SetTemp;
  uint8_t selection = 0;
  bool repeat = true;

  while (repeat) {
    //arduboy.invert(0);
    MenuLevel = 0; //默认打开主菜单
    selection = MenuScreen(selection);
    switch (selection) {
      case 0:   MenuLevel = 1; TipScreen();  repeat = false; break;
      case 1:   MenuLevel = 2; TempScreen();  break;
      case 2:   MenuLevel = 3; TimerScreen();  break;
      case 3:   MenuLevel = 4; PIDenable = MenuScreen(PIDenable); break;
      case 4:   ScreenSet(); break;
      case 5:   MenuLevel = 6; BeepEnable = MenuScreen(BeepEnable); break;
      case 6:   RotarySet(); break;
      case 7:   UnderVoltageSet(); break;
      case 8:   PasswordSet(); break;
      case 9:   MenuLevel = 8; LANG = MenuScreen(LANG); if (LANG == 2 && !LANG_JP_State) LANG = 1; break;
      case 10:   QRcodeScreen(); break;
      default:  repeat = false; break;
    }
  }
  UpdateEEPROM();
  handleMoved = true;
  SetTemp = SaveSetTemp;
  setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, SetTemp);
}

//设置滚轮方向
void RotarySet() {
  bool Exit = false;
  float rad;
  byte ExitBar = 0;
  bool color = 0;
  SetTextColor(!color);
  lastbutton = (!digitalRead(BUTTON_PIN));
  setRotary(60, 420, 18, 240);
  do {
    arduboy.setTextSize(1);
    Setpoint = getRotary();
    ShowTemp = Setpoint;

    arduboy.clear();

    arduboy.fillRect(0, 0, 128, 64, color);

    //仪表盘
    arduboy.fillCircle(24 + 12 * cos(rad), 24 - 12 * sin(rad), 5, !color);
    arduboy.drawCircle(24, 24, 20, !color);

    //右上角的模式提示文本
    arduboy.setCursor(55, 1);
    arduboy.setTextSize(2);
    arduboy.print(F("MODE "));
    if (RotaryD) {
      arduboy.print(F("-"));
      rad = ((Setpoint - 60) / 360.0) * (2 * PI) - PI / 2;
    } else {
      arduboy.print(F("+"));
      rad = ((-Setpoint - 60) / 360.0) * (2 * PI) + PI * 1 / 6;
    }
    //右侧数字显示
    DrawNumRect(55, 18, 4, ShowTemp);
    //下端的状态条
    DrawStatusBar(!color);
    //退出条
    arduboy.fillRect(0, 0, ExitBar, 4, !color);
    arduboy.display();
    CheckLastButton();
    if (!digitalRead(BUTTON_PIN) && !lastbutton) {
      ExitBar += 8;
      arduboy.display();
      if (ExitBar > 127) Exit = true;
    } else {
      if (ExitBar > 1 && ExitBar < 64) {
        RotaryD = !RotaryD;
        setRotary(60, 420, 18, 240);
        beep();
      }
      ExitBar = 0;
    }

  } while (!Exit);
  beep(); beep();
}
//显示设置菜单
// tip settings screen
void ScreenSet() {
  uint8_t selection = 0;
  bool repeat = true;
  while (repeat) {
    MenuLevel = 9;
    selection = MenuScreen(selection);
    switch (selection) {
      case 0:   MenuLevel = 5; MainScrType = MenuScreen(MainScrType);   break;
      case 1:   FlipState = !FlipState; break;
      default:  repeat = false;      break;
    }
    Flip(FlipState); //设置屏幕反转状态
  }
  UpdateEEPROM();
}

//预设设置菜单
// tip settings screen
void TipScreen() {
  uint8_t selection = 0;
  bool repeat = true;
  while (repeat) {
    MenuLevel = 1;
    selection = MenuScreen(selection);
    switch (selection) {
      case 0:   ChangeTipScreen();   break;
      case 1:   CalibrationScreen(); break;
      case 2:   InputNameScreen();   break;
      case 3:   DeleteTipScreen();   break;
      case 4:   AddTipScreen();      break;
      default:  repeat = false;      break;
    }
  }
}

//温控设置菜单
// temperature settings screen
void TempScreen() {
  uint8_t selection = 0;
  bool repeat = true;
  while (repeat) {
    selection = MenuScreen(selection);
    switch (selection) {
      case 0:   setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, DefaultTemp);
        DefaultTemp = InputScreen(18); break;
      case 1:   setRotary(20, 200, TEMP_STEP, SleepTemp);
        SleepTemp = InputScreen(19); break;
      case 2:   setRotary(10, 100, TEMP_STEP, BoostTemp);
        BoostTemp = InputScreen(20); break;
      default:  repeat = false; break;
    }
  }
  UpdateEEPROM();
}


//定时器设置菜单
// timer settings screen
void TimerScreen() {
  uint8_t selection = 0;
  bool repeat = true;
  while (repeat) {
    selection = MenuScreen(selection);
    switch (selection) {
      case 0:   setRotary(0, 30, 1, Time2sleep);
        Time2sleep = InputScreen(22); break;
      case 1:   setRotary(0, 60, 5, Time2off);
        Time2off = InputScreen(23); break;
      case 2:   setRotary(0, 180, 10, TimeOfBoost);
        TimeOfBoost = InputScreen(24); break;
      default:  repeat = false; break;
    }
  }
  UpdateEEPROM();
}
//英语支持
//i-文本id c-居中
void Print_EN(byte i) {
  char buffer[11];
  byte a = 0;
  strcpy_P(buffer, (char *)pgm_read_word(&(EN_table[i])));
  while (1) {
    if (buffer[a] != '\0') a++; else break;
  }
  arduboy.setCursor((128 - a * 6) / 2, 52);
  arduboy.print(buffer);
}
//菜单界面
// menu screen
uint8_t MenuScreen(uint8_t selected) {
  int SlidingAnimationX;
  uint8_t lastselected = selected;
  setRotary(0, Menu_table[MenuLevel] - 1, 1, selected);
  lastbutton = (!digitalRead(BUTTON_PIN));
  SetTextColor(1);
  do {
    selected = getRotary();
    //非线性滑动动画
    SlidingAnimationX += (selected - lastselected) * 56;
    if (SlidingAnimationX != 0) SlidingAnimationX += 0.55 * (-SlidingAnimationX);
    lastselected = selected;
    arduboy.clear();
    //绘制图标 如果有指定的话
    for (byte i = 0; i < 5; i++) if (selected - 2 + i >= 0 && selected - 2 + i < Menu_table[MenuLevel]) DrawApp(-72 + i * 56 + SlidingAnimationX, selected - 2 + i + QueryMenuObject());
    DrawAppText(selected + QueryMenuObject());

    arduboy.display();
    CheckLastButton();
    //beep(0);
  } while (digitalRead(BUTTON_PIN) || lastbutton);
  beep();
  return selected;
}

//显示APP图标
void DrawApp(int x, byte appID) {
  DrawUIFrame(x, 0, 1);
  drawSlowXYBitmapResize(x + 3, 3, Ico_table[appID], 14, 14, 3, 1);
}

//显示APP对应的文本
void DrawAppText(byte appID) {
  //  arduboy.setCursor(0, 55); arduboy.print(appID);
  if (LANG == 0) arduboy.drawSlowXYBitmap(48, 48, CN_table[appID], 36, 16, 1); else if (LANG == 2 && LANG_JP_State) {
#if LANG_JP_State
    drawText(48, 52, JP_table[appID], pgm_read_byte(&(JP_Length_table[appID])));
#endif
  } else {
    arduboy.setCursor(48, 52);
    arduboy.setTextSize(1);
    Print_EN(appID);
  }
}

void DrawUIFrame(int x, int y, bool color) {
  arduboy.fillRect(x, y, 48, 48, color);
  arduboy.fillRect(x + 3, y + 3, 42, 42, !color); //雕空
  DrawPoint(x, y, 0);
  DrawPoint(x + 47, y, 0);
  DrawPoint(x, y + 47, 0);
  DrawPoint(x + 47, y + 47, 0);
}

void DrawPoint(int x, int y, bool color) {
  arduboy.drawPixel(x + 1, y, color);
  arduboy.drawPixel(x - 1, y, color);
  arduboy.drawPixel(x, y + 1, color);
  arduboy.drawPixel(x, y - 1, color);
  arduboy.drawPixel(x, y, color);
}

//FP 密集运算屏保
void DrawIntensiveComputing() {
  //关闭加热
#if UsePMOS
  analogWrite(CONTROL_PIN, 0);
#else
  analogWrite(CONTROL_PIN, 255);
#endif
  float calculate;
  // while (1) {
  arduboy.clear();

  //随机线条
  DrawIntensiveComputingLine();

  calculate = sin(millis() / 4000.0);
  //模拟噪点
  for (int i = 0; i < calculate * 256 + 256; i++)  arduboy.drawPixel(random(0, 128), random(0, 64), 1);
  //声效
  if ((BeepEnable)) analogWrite(BUZZER_PIN, 64 + calculate * 64 + random(-8, 8));
  //if ((BeepEnable)) tone(BUZZER_PIN, int(calculate * 300) + 500 + random(-50, 50));  //没必要浪费内存

  arduboy.display();
  //delay(60-calculate*30);
  //}
}
//绘制屏保-密集运算线条
void DrawIntensiveComputingLine() {
  for (byte a = 0; a < 4; a++) {
    Line[a] += random(-1, 1);
    if (Line[a] > 128) Line[a] -= 128;
    for (byte b = 0; b < random(3, 6); b++) {
      arduboy.drawFastHLine(0, Line[a] + random(-10, 10), 128, 1); //水平线
      arduboy.drawFastVLine(Line[a] + random(-10, 10), 0, 64, 1); //垂直线
    }
  }
}
//模拟数字滚轮 Numerical scrolling effect
void DisplayNum(int Num) {
  arduboy.setTextSize(6);
  SetTextColor(0);
  //数字滚轮
  for (byte i = 0; i < 3; i++) LastValue[i] = int(Value / pow(10, 2 - i)) % 10;
  Value = Num;
  for (byte i = 0; i < 3; i++) {
    SlidingAnimationY[i] += ((int(Value / pow(10, 2 - i)) % 10) - LastValue[i]) * 50;
    if (SlidingAnimationY[i] != 0) SlidingAnimationY[i] += 0.8 * (-SlidingAnimationY[i]);
    arduboy.fillRect(0 + i * 44, 0, 38, 50, 1); //白底
    for (int ii = -1; ii < 2; ii++) {
      arduboy.setCursor(4 + i * 44, 4 - SlidingAnimationY[i] + ii * 50);
      arduboy.print(LastValue[i] + ii);
    }
  }
  //刻度标
  arduboy.fillRect(0, 0, 128, 2, 0); //上遮罩层
  arduboy.fillRect(0, 48, 128, 16, 0); //下遮罩层
}

//数值输入界面
// input value screen
uint16_t InputScreen(byte appID) {
  lastbutton = (!digitalRead(BUTTON_PIN));
  do {
    arduboy.clear();
    DisplayNum(getRotary());
    DrawAppText(appID);
    arduboy.display();
    CheckLastButton();
  } while (digitalRead(BUTTON_PIN) || lastbutton);
  beep();
  return Value;
}

//设置开机密码
void PasswordSet() {
  Password = InputBigNum(Password, 8);
  beep();
}

void BootPassword() {
  if (Password != 0) {
    do {
      arduboy.clear();
      DrawApp(40, 8);
      DrawAppText(8);
      arduboy.display();
      delay(1000);
      beep(); beep(); beep();
    } while (InputBigNum(0, 8) != Password);
  }
}

//欠压报警设置
// information display screen
void UnderVoltageSet() {
  setRotary(0, 250, 1, UnderVoltage);
  UnderVoltage = InputScreen(7);
  beep();
}


//更换烙铁头预设界面
// change tip screen
void ChangeTipScreen() {
  uint8_t selected = CurrentTip;
  uint8_t lastselected = selected;
  int8_t  arrow = 0;
  if (selected) arrow = 1;
  setRotary(0, NumberOfTips - 1, 1, selected);
  lastbutton = (!digitalRead(BUTTON_PIN));
  do {
    //beep(0);
    arduboy.clear();
    selected = getRotary();
    arrow = constrain(arrow + selected - lastselected, 0, 2);
    lastselected = selected;
    SetTextColor(0);
    arduboy.setTextSize(2);
    arduboy.setCursor(0, 0); arduboy.print(F("Select Tip"));
    SetTextColor(1);
    arduboy.setCursor(0, 16 * (arrow + 1)); arduboy.print(F(">"));
    for (uint8_t i = 0; i < 3; i++) {
      uint8_t drawnumber = selected + i - arrow;
      if (drawnumber < NumberOfTips) {
        arduboy.setCursor(12, 16 * (i + 1));
        GetEEPRomTip(selected + i - arrow);
        arduboy.print(TipName);
      }
    }
    CheckLastButton();
    arduboy.display();
  } while (digitalRead(BUTTON_PIN) || lastbutton);

  beep();
  CurrentTip = selected;
  GetEEPRomTip(CurrentTip);
  beep();
  //控制台指令
  String s = TipName;
  if (s == "EEPROM\0") ViewEEPRom();
  if (s == "RESET.\0") {
    MenuLevel = 7;
    if (MenuScreen(0)) {
      for (int i = 0 ; i < EEPROM.length() ; i++) EEPROM.write(i, 255);
      resetFunc();
    }
  }
  // if (s == "LIGHT.\0")
  ShowPTemp(&PTemp[0]);


}

//温度校准界面
// temperature calibration screen
void CalibrationScreen() {
  float P[4];
  int xx[9];
  for (int CalStep = 0; CalStep < 9; CalStep++) {
    if (CalStep != 0) setRotary(0, 1023, 1, xx[CalStep - 1]); else setRotary(0, 1023, 1, 0);
    BeepIfWorky = true;
    lastbutton = (!digitalRead(BUTTON_PIN));
    do {
      arduboy.clear();
      arduboy.setTextSize(1);
      SENSORCheck();       //读取传感器
      SetTemp = getRotary();
      Thermostat(1);       //加热控制 - ADC数值为基准

      arduboy.setCursor(25, 0);
      SetTextColor(0);
      arduboy.print(F("ADC-> "));
      arduboy.print(RawTemp);
      SetTextColor(1);
      for (byte y = 0; y < 5; y++) {
        for (byte x = 0; x < 2; x++) {
          if (2 * y + x > CalStep) break;
          arduboy.setCursor(x * 64 + 10, y * 8 + 16);
          arduboy.print(CalTemp[2 * y + x]);
          arduboy.print((char)248);
          arduboy.print(F("- "));
          if (2 * y + x == CalStep) arduboy.print(getRotary());
          else arduboy.print(xx[2 * y + x]);
        }
      }
      //进度条
      ProgressBar(CalStep + 1, 0, 9, 0, 60, 128, 4, 1);

      CheckLastButton();
      arduboy.display();
    } while (digitalRead(BUTTON_PIN) || lastbutton);
    xx[CalStep] = getRotary();
    beep(); delay (10);
  }

  //关闭加热
#if UsePMOS
  analogWrite(CONTROL_PIN, 0);
#else
  analogWrite(CONTROL_PIN, 255);
#endif
  delayMicroseconds(Time2settle);             // wait for voltage to settle
  bool pass = true;
  for (int i = 0; i < 8; i++) if (xx[i] + 5 >= xx[i + 1]) pass = false;
  if (pass) {
    MenuLevel = 7;
    polyfit(9, xx, CalTemp, 3, P); //拟合程序
    free(xx);
    ShowPTemp(&P[0]);
    if (MenuScreen(0)) {
      for (uint8_t i = 0; i < 4; i++) PTemp[i] = P[i]; //写入拟合系数
      free(P);
      UpdateEEPROM();
    }
  } else {
    arduboy.clear();
    arduboy.setTextSize(2);
    SetTextColor(0);
    arduboy.setCursor(32, 24);
    arduboy.print(F("Error"));
    arduboy.display();
    delay(5000);
    resetFunc();
  }
  arduboy.clear();
  arduboy.setTextSize(2);
  SetTextColor(0);
  arduboy.setCursor(28, 24);
  arduboy.print(F("Reboot"));
  arduboy.display();
  delay(5000);
  resetFunc();
}
//显示默认烙铁头温度曲线系数
void ShowPTemp(float *p) {
  arduboy.clear();
  arduboy.setTextSize(1);
  for (int i = 0; i < 4; i++) {
    arduboy.setCursor(12, i * 8 + 16);
    arduboy.print(F("P["));
    arduboy.print(i, 7);
    arduboy.print(F("] = "));
    arduboy.print(*p, 7);
    p++;
  }
  arduboy.display();

  lastbutton = (!digitalRead(BUTTON_PIN));
  while (digitalRead(BUTTON_PIN) || lastbutton) CheckLastButton();
  /*
    lastbutton = (!digitalRead(BUTTON_PIN));
    setRotary(50, 450, 1, 0);
    do {
    arduboy.clear();
    for (int y = 0; y < 64; y++) arduboy.drawPixel(map(calculateTemp(map(y, 0, 63, 0, 400)), CalTemp[0], CalTemp[8], 0, 127), y,1);
    arduboy.display();
    CheckLastButton();
    } while (digitalRead(BUTTON_PIN) || lastbutton);*/
}
//命名界面 文本输入界面
// input tip name screen
void InputNameScreen() {
  uint8_t  value;
  for (uint8_t digit = 0; digit < (TIPNAMELENGTH - 1); digit++) {
    lastbutton = (!digitalRead(BUTTON_PIN));
    setRotary(31, 96, 1, 65);
    do {
      value = getRotary();
      if (value == 31) {
        value = 95;
        setRotary(31, 96, 1, 95);
      }
      if (value == 96) {
        value = 32;
        setRotary(31, 96, 1, 32);
      }
      //arduboy.setCursor(0, 0);arduboy.print();
      arduboy.clear();
      arduboy.setTextSize(2);
      arduboy.setCursor(16 * digit, 48); arduboy.print(char(94));
      arduboy.setCursor(0, 32);
      for (uint8_t i = 0; i < digit; i++) arduboy.print(TipName[i]);
      arduboy.setCursor(16 * digit, 32); arduboy.print(char(value));
      CheckLastButton();
      arduboy.display();
    } while (digitalRead(BUTTON_PIN) || lastbutton);
    TipName[digit] = value;
    UpdateEEPROM();
    beep(); delay (10);
  }
  TipName[TIPNAMELENGTH - 1] = 0;
  return value;
}

//删除烙铁头
// delete tip screen
void DeleteTipScreen() {
  MenuLevel = 7;
  if (NumberOfTips == 1) {
    arduboy.clear();
    arduboy.setTextSize(2);
    SetTextColor(0);
    arduboy.setCursor(16, 24);
    arduboy.print(F("Only one"));
    arduboy.display();
    delay(1000);
  }
  else if (MenuScreen(0)) {
    if (CurrentTip == (NumberOfTips - 1)) {
      CurrentTip--;
    }
    else {
      DelEEPRomTip(CurrentTip);
    }
    NumberOfTips--;
  }
}

//新建烙铁 预设
// add new tip screen
void AddTipScreen() {
  if (NumberOfTips < TIPMAX) {
    CurrentTip = NumberOfTips++; InputNameScreen();
    PTemp[0] = TempP1;
    PTemp[1] = TempP2;
    PTemp[2] = TempP3;
    PTemp[3] = TempP4;
    UpdateEEPROM();
  } else {
    arduboy.clear();
    SetTextColor(0);
    arduboy.setCursor(5, 24);
    arduboy.print(F("Out of Mem"));
    arduboy.display();
    delay(1000);
  }

}

int InputBigNum(int InputPW, byte appID) {
  setRotary(0, 900, 100, InputPW);
  InputPW = InputScreen(appID);
  setRotary(InputPW, InputPW + 90, 10, InputPW);
  InputPW = InputScreen(appID);
  setRotary(InputPW, InputPW + 9, 1, InputPW);
  InputPW = InputScreen(appID);
  return InputPW;
}
void QRcodeScreen() {
  arduboy.clear();
  arduboy.drawSlowXYBitmap(40, 0, QRCode, 48, 48, 1);
  arduboy.display();
  delay(100);
  do {
  } while (digitalRead(BUTTON_PIN) || lastbutton);
  beep();
}

//位图缩放 代码片段改自arduboy2
void drawSlowXYBitmapResize(int x, int y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t size, uint8_t color) {
  int16_t xi, yi, byteWidth = (w + 7) / 8;
  for (yi = 0; yi < h; yi ++) {
    for (xi = 0; xi < w; xi ++) {
      if (pgm_read_byte(bitmap + yi * byteWidth + xi / 8) & (128 >> (xi & 7))) {
        arduboy.fillRect(x + xi * size, y + yi * size, size, size, color);
      }
    }
  }
}

//翻转
void Flip(bool Fmode) {
  arduboy.flipVertical(Fmode);
  arduboy.flipHorizontal(Fmode);
}

//展示版本号
void ShowVersion() {
  arduboy.clear();
  arduboy.setTextSize(2);
  arduboy.setCursor(28, 24);
  arduboy.print(F(VERSION));
  arduboy.display();
  delay(700);
  arduboy.clear();
  SetTextColor(0);
  arduboy.setCursor(28, 24);
  arduboy.print(F(VERSION));
  arduboy.display();
  delay(300);
}

/*进度条
   传入：i=值 a=值的最小值 b=值的最大值 x=左上顶点x轴坐标 y=左上顶点y轴坐标 w=宽度 h=高度 c=颜色
*/
void ProgressBar(int i, int a, int b, byte x, byte y, byte w, byte h, bool c) {
  SetTextColor(c);
  arduboy.setTextSize(c);
  //进度标
  arduboy.setCursor(map(i, a, b, 0, 92), 52);
  arduboy.print(((float)i / (b - a)) * 100); arduboy.print(F("%"));
  //进度条
  arduboy.fillRect(x, y, map(i, a, b, 0, w), h, c);
}

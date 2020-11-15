/*License: http://creativecommons.org/licenses/by-sa/3.0/
   采用CC3.0协议共享此程序
   您可以自由地
  共享 — 在任何媒介以任何形式复制、发行本作品
  演绎 — 修改、翻译或以本作品为基础进行创作
  在任何用途下，甚至商业目的。
  本许可协议为“自由文化作品（Free Cultural Works）”所接受。
  只要你遵守许可协议条款，许可人就无法收回你的这些权利。

  Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0)
  This is a human-readable summary of (and not a substitute for) the license. Disclaimer.
  You are free to:
  Share — copy and redistribute the material in any medium or format
  Adapt — remix, transform, and build upon the material
  for any purpose, even commercially.
  This license is acceptable for Free Cultural Works.
  The licensor cannot revoke these freedoms as long as you follow the license terms.
*/
// SolderingStation2
//
// ATmega328-controlled Soldering Station for Hakko T12 Tips.
// 此v1.8t7版本功能简介
// This 1.8t7 version of the code implements:
// - 烙铁头温度实时监测
// - Temperature measurement of the tip
// - PID温度控制
// - Direct or PID control of the heater
// - 可以通过编码器进行控制温度
// - Temperature control via rotary encoder
// - 短按编码器进入短时升温模式
// - Boost mode by short pressing rotary encoder switch
// - 长按编码器进入主菜单
// - Setup menu by long pressing rotary encoder switch
// - 手柄震动检测（需要手柄含有震动传感器）
// - Handle movement detection (by checking ball switch)
// - 手柄连接检测（通过判断烙铁头温度是否能够被读取来实现）
// - Iron unconnected detection (by idenfying invalid temperature readings)
// - 定时器设定 - 随眠，关机时间
// - Time driven sleep/power off mode if iron is unused (movement detection)
// - 可以测量主控芯片电压以及温度
// - Measurement of input voltage, Vcc and ATmega's internal temperature
// - 在OLED上实现信息显示
// - Information display on OLED
// - 支持无源蜂鸣器
// - Buzzer
// - 允许设置不同烙铁头的温度校准值
// - Calibrating and managing different soldering tips
// - 设置被保存在EEPROM，掉电不丢失
// - Storing user settings into the EEPROM
// - 允许热插拔烙铁头，并弹出烙铁头配置选择菜单
// - Tip change detection
// - 芯片过热以及低电压报警
// - Chip overheating and low voltage alarm
// - 更好的实时温度条以及实时功率条
// - Better real-time temperature bar and real-time power bar
// - 拥有过渡动画
// - Have transition animation
// - 内置屏幕保护程序
// - Built in screen saver
// - 屏幕翻转设置
// - Screen flip settings
// - 内置英文、中文、日文语言包
// - Built in English, Chinese and Japanese language pack
// - 开机密码设置
// - System Password
// - 允许设置旋钮方向
// - Knob direction setting menu
// - 内嵌看门狗，宕机自动重启
// - Watchdog timer automatic reset the MCU

// - V1.8t7 New features:
// ------ EEPROM可用性检查机制
// ------ EEPROM availability check

// ------ 允许保存最大30个烙铁头的配置
// ------ 30 preservable soldering tip configurations

// ------ 9段温度曲线拟合
// ------ 9 segments of temperature curve fitting

/*
增加"EEPROM"和"RESET." 指令，可以给标签命名为指令的名字，点击标签即可执行
关于"EEPROM"指令:查看EEPROM内容
关于"RESET."指令：重置EEPROM *警告！ 你会失去所有烙铁头的温度校准数据!!!

Add "EEPROM" and "RESET." to give the label the name of the instruction. Click the label to execute
Regarding "EEPROM" instruction: View EEPROM contents
Regarding "RESET." instruction: RESET EEPROM 
* warning! You will lose all the temperature calibration data for the tip of the soldering iron!!
*/


// Power supply should be in the range of 16V/2A to 24V/3A and well
// stabilized.
//
// For calibration you need a soldering iron tips thermometer. For best results
// wait at least three minutes after switching on the soldering station before
// you start the calibration process.
//
// Controller:  ATmega328p
// Core:        Barebones ATmega (https://github.com/carlosefr/atmega)
// Clockspeed:  16 MHz external
//
// 2019/2020 by Stefan Wagner with great support from John Glavinos
// 2020-8 UI upgrade by LHW-createskyblue 、lihaoyun6
// Project Page: https://easyeda.com/wagiminator


bool lastbutton;
#include <Arduboy2.h>  //注意 这里使用被阉割的图形库，只有绘图功能，并非通用的版本
Arduboy2 arduboy;

#include <EEPROM.h>
#include <PID_v1.h>
#include <avr/sleep.h>
// Firmware version
#define VERSION       "v1.8t7"

// Type of rotary encoder
#define ROTARY_TYPE   1         // 0: 2 increments/step; 1: 4 increments/step

// Pins
#define SENSOR_PIN    A0        // temperature sense
#define VIN_PIN       A1        // input voltage sense
#define BUZZER_PIN     5        // buzzer
#define BUTTON_PIN     6        // rotary encoder switch
#define ROTARY_1_PIN   7        // rotary encoder 1
#define ROTARY_2_PIN   8        // rotary encoder 2
#define CONTROL_PIN    9        // heater MOSFET PWM control
#define SWITCH_PIN    10        // handle vibration switch

// Default temperature control values (recommended soldering temperature: 300-380°C)
#define TEMP_MIN      100       // min selectable temperature
#define TEMP_MAX      400       // max selectable temperature
#define TEMP_DEFAULT  320       // default start setpoint
#define TEMP_SLEEP    150       // temperature in sleep mode
#define TEMP_BOOST     60       // temperature increase in boost mode
#define TEMP_STEP      5       // rotary encoder temp change steps

// Default tip temperature calibration values
#define TEMPCHP       30        // chip temperature while calibration
#define TIPMAX        30         // max number of tips
#define TIPNAMELENGTH 7         // max length of tip names (including termination)
#define TIPNAME       "SZtip"   // default tip name


// Default timer values (0 = disabled)
#define TIME2SLEEP     5        // time to enter sleep mode in minutes
#define TIME2OFF      15        // time to shut off heater in minutes
#define TIMEOFBOOST   40        // time to stay in boost mode in seconds

// Control values
#define TIME2SETTLE   950       // time in microseconds to allow OpAmp output to settle
#define SMOOTHIE      0.05      // OpAmp output smooth factor (1=no smoothing; 0.05 default)
#define PID_ENABLE    true      // enable PID control
#define BEEP_ENABLE   true      // enable/disable buzzer
#define MAINSCREEN    1         // type of main screen (0: big numbers; 1: more infos)

// EEPROM identifier
#define EEPROM_IDENT   0x140C   // to identify if EEPROM was written by this program

//Beep
#define SetBeepCoolTime 200
unsigned long BeepCoolTime;

// Define the aggressive and conservative PID tuning parameters
double aggKp = 11, aggKi = 0.5, aggKd = 1;
double consKp = 11, consKi = 3, consKd = 5;

// Default values that can be changed by the user and stored in the EEPROM
uint16_t  DefaultTemp = TEMP_DEFAULT;
uint16_t  SleepTemp   = TEMP_SLEEP;
uint8_t   BoostTemp   = TEMP_BOOST;
uint8_t   Time2sleep  = TIME2SLEEP;
uint8_t   Time2off    = TIME2OFF;
uint8_t   TimeOfBoost = TIMEOFBOOST;
uint8_t   MainScrType = MAINSCREEN;
bool      PIDenable   = PID_ENABLE;
bool      BeepEnable  = BEEP_ENABLE;

//温度拟合
#define TempP1 10.2675914764
#define TempP2 1.7323191166
#define TempP3 -0.0033245713
#define TempP4 0.0000045338
float    PTemp[4] = {TempP1, TempP2, TempP3, TempP4}; //温度拟合系数
const uint16_t CalTemp[9] = {50, 100, 150, 200, 250, 300, 350, 400, 450};
char      TipName[TIPNAMELENGTH] = {TIPNAME};
uint8_t   CurrentTip   = 0;
uint8_t   NumberOfTips = 1;

// Variables for pin change interrupt
volatile uint8_t  a0, b0, c0, d0;
volatile bool     ab0;
volatile int      Count, CountMin, CountMax, CountStep;
volatile bool     handleMoved;

// Variables for temperature control
uint16_t  SetTemp, ShowTemp, gap, Step;
double    Input, Output, Setpoint, RawTemp, CurrentTemp, ChipTemp;

// Variables for voltage readings
uint16_t  Vcc, Vin;

// State variables
bool      InSleepMode = false;
bool      InOffMode   = false;
bool      InBoostMode = false;
bool      InCalibMode = false;
bool      IsWorky     = true;
bool      BeepIfWorky = true;
bool      TipIsPresent = true;
bool      FlipState = false;
// Timing variables
uint32_t  Sleepmillis;
uint32_t  Boostmillis;
uint32_t  Buttonmillis;
uint8_t   GoneMinutes;
uint8_t   GoneSeconds;
uint8_t   SensorCounter = 255;
//串口助手
//int x, y, a, b;
//欠压报警
byte UnderVoltage = 0;
// Control variables
uint16_t  Time2settle = 300;

//模拟数字滚轮 Numerical scrolling effect
int Value;
int LastValue[3];
int SlidingAnimationY[3];

//开机密码 BootPassword
int Password = 0;

//屏幕保护-主题：密集运算
byte Line[4];

//编码器旋转调整方向设定
bool RotaryD = false;

//开机非线性动画
byte BootAnimationY = 64;

// Specify the links and initial PID tuning parameters
PID ctrl(&Input, &Output, &Setpoint, aggKp, aggKi, aggKd, REVERSE);

//NMOS - PMOS软件切换
#define UsePMOS false

//LANG 语言支持
/*  0 - 中文 CHINESE
    1 - 英文 ENGLISH
    2 - 日语 x (此版本不支持)
*/
byte LANG = 1;



void setup() {
  //设置看门狗自动复位
  set_wdt_mod(3, 7);

  //启动arduboy内核
  arduboy.begin();
  arduboy.setFrameRate(15);

  //Serial.begin(115200);

  // set the pin modes
  //设置IO
  pinMode(SENSOR_PIN,   INPUT);
  pinMode(VIN_PIN,      INPUT);
  pinMode(BUZZER_PIN,   OUTPUT);
  pinMode(CONTROL_PIN,  OUTPUT);
  pinMode(ROTARY_1_PIN, INPUT_PULLUP);
  pinMode(ROTARY_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN,   INPUT_PULLUP);
  pinMode(SWITCH_PIN,   INPUT_PULLUP);
#if UsePMOS
  analogWrite(CONTROL_PIN, 0);
#else
  analogWrite(CONTROL_PIN, 255);        // this shuts off the heater
#endif
  digitalWrite(BUZZER_PIN, LOW);        // must be LOW when buzzer not in use

  //初始化ADC
  //setup ADC
  ADCSRA |= bit (ADPS0) | bit (ADPS1) | bit (ADPS2);  // set ADC prescaler to 128
  ADCSRA |= bit (ADIE);                 // enable ADC interrupt
  interrupts ();                        // enable global interrupts

  //初始化编码器IO
  // setup pin change interrupt for rotary encoder
  PCMSK0 = bit (PCINT0);                // Configure pin change interrupt on Pin8
  PCICR  = bit (PCIE0);                 // Enable pin change interrupt
  PCIFR  = bit (PCIF0);                 // Clear interrupt flag

  //从EEPROM读取设置
  // get default values from EEPROM
  GetEEPROM();

  //设置屏幕反转状态
  Flip(FlipState);

  //设置屏幕亮度
  SetOLEDLightLevel(32); //降低屏幕亮度 延长OLED使用寿命

  //初始化屏幕保护动画
  for (byte a = 0; a < 4; a++) Line[a] = 32 * a;

  //密码保护
  BootPassword();
  // read supply voltages in mV
  Vcc = getVCC(); Vin = getVIN();
  // read and set current iron temperature
  SetTemp  = DefaultTemp;
  RawTemp  = denoiseAnalog(SENSOR_PIN);
  ChipTemp = getChipTemp();
  CurrentTemp = calculateTemp(RawTemp);
  // turn on heater if iron temperature is well below setpoint
#if UsePMOS
  if ((CurrentTemp + 20) < DefaultTemp) analogWrite(CONTROL_PIN, 255);
#else
  if ((CurrentTemp + 20) < DefaultTemp) analogWrite(CONTROL_PIN, 0);
#endif
  // tell the PID to range between 0 and the full window size
  ctrl.SetOutputLimits(0, 255);
  // start PID
  ctrl.SetMode(AUTOMATIC);
  // set initial rotary encoder values
  a0 = PINB & 1; b0 = PIND >> 7 & 1; ab0 = (a0 == b0);
  setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, DefaultTemp);
  // reset sleep timer
  Sleepmillis = millis();

  // long beep for setup completion
  beep(); beep();
  ShowVersion();
}

void loop() {
  //HelpMeSerialer();
  ROTARYCheck();      // check rotary encoder (temp/boost setting, enter setup menu)
  SLEEPCheck();       // check and activate/deactivate sleep modes
  SENSORCheck();      // reads temperature and vibration switch of the iron
  Thermostat(0);       // heater control
  MainScreen();       // updates the main page on the OLED
  //beep(0);
}

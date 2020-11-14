//********** 看门狗函数 **********
void set_wdt_mod(uint8_t mod, uint8_t ii) //模式，超时时间
{
  /*
    WDCE
    该位在定时序列中用于更改WDE和预分频器位
    要清除WDE位或更改预分频器位，必须将WDCE置1。
    一旦写入1，硬件将在四个时钟周期后清除WDCE。
    0=16ms,1=32ms,2=64ms,3=128ms,4=250ms
    5=500ms,6=1s,7=2s,8=4s,9=8s
  */
  if (ii > 9 ) ii = 9;
  byte bb;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);

  switch (mod) {
    case 0: //关闭看门狗
      __asm__ __volatile__ ("wdr");      //看门狗复位
      MCUSR = 0x00;                      //看门狗复位标志清零
      WDTCSR = (1 << WDCE) | (1 << WDE); //打开允许修改使能，并WDE置1
      WDTCSR = 0x00;                     //清除所有预分频位
      break;

    case 1: //打开看门狗，设置复位模式
      __asm__ __volatile__ ("wdr");       //看门狗复位
      MCUSR &= ~(1 << WDRF);              //MUUSR中的WDRF清零，清除复位标志
      WDTCSR |= (1 << WDCE) | (1 << WDE); //打开允许修改使能，并WDE置1
      //设置新的看门狗超时值bb
      WDTCSR = bb;                        //8秒时bb=0011 0001
      WDTCSR |= _BV(WDE);                 //设置复位模式
      break;

    case 2: //打开看门狗，设置中断模式
      __asm__ __volatile__ ("wdr");       //看门狗复位
      MCUSR &= ~(1 << WDRF);              //MUUSR中的WDRF清零，清除复位标志
      WDTCSR |= (1 << WDCE) | (1 << WDE); //打开允许修改使能，并WDE置1
      //设置新的看门狗超时值bb
      WDTCSR = bb;                        //8秒时bb=0011 0001
      WDTCSR |= _BV(WDIE);                //设置中断模式
      break;

    case 3: //打开看门狗，设置中断和复位模式
      __asm__ __volatile__ ("wdr");       //看门狗复位
      MCUSR &= ~(1 << WDRF);              //MUUSR中的WDRF清零，清除复位标志
      WDTCSR |= (1 << WDCE) | (1 << WDE); //打开允许修改使能，并WDE置1
      //设置新的看门狗超时值bb
      WDTCSR = bb;                        //8秒时bb=0011 0001
      WDTCSR |= _BV(WDIE);                //设置中断模式
      WDTCSR |= _BV(WDE);                 //设置复位模式
      break;
  }
}

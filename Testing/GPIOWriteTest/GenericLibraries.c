//How to use github (Jacob ignore this pls_)
//git pull; git add . ; git commit -m "message" ; git push


#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include <stdio.h>
#include <lpc17xx_timer.h>

#define SENDING 0x40040
#define RECEIVING 0x20000
#define STOP 0x000000
#define BIT_TIME 3
#define LOW 1
#define HIGH 0
#define WAIT 1
#define DONT_WAIT 0
#define LCD_ADDRESS 0x3B

#define _SPACE 0xA0
#define _A 0xC1
#define _B 0xC2
#define _C 0xC3
#define _D 0xC4
#define _E 0xC5
#define _F 0xC6
#define _G 0xC7
#define _H 0xC8
#define _I 0xC9
#define _J 0xCA
#define _K 0xCB
#define _L 0xCC
#define _M 0xCD
#define _N 0xCE
#define _O 0xCF
#define _P 0xD0
#define _Q 0xD1
#define _R 0xD2
#define _S 0xD3
#define _T 0xD4
#define _U 0xD5
#define _V 0xD6
#define _W 0xD7
#define _X 0xD8
#define _Y 0xD9
#define _Z 0xDA
#define _0 0xB0
#define _1 0xB1
#define _2 0xB2
#define _3 0xB3
#define _4 0xB4
#define _5 0xB5
#define _6 0xB6
#define _7 0xB7
#define _8 0xB8
#define _9 0xB9
#define _a 0x61
#define _b 0x62
#define _c 0x63
#define _d 0x64
#define _e 0x65
#define _f 0x66
#define _g 0x67
#define _h 0x68
#define _i 0x69
#define _j 0x6A
#define _k 0x6B
#define _l 0x6C
#define _m 0x6D
#define _n 0x6E
#define _o 0x6F
#define _p 0x70
#define _q 0x71
#define _r 0x72
#define _s 0x73
#define _t 0x74
#define _u 0x75
#define _v 0x76
#define _w 0x77
#define _x 0x78
#define _y 0x79
#define _z 0x7A
#define _ASTERIX 0xAA
#define _HASH 0xA3

//Address Defines for SAA1064
#define SAA1064_SA0 0x70
#define SAA1064_SA1 0x72
#define SAA1064_SA2 0x74
#define SAA1064_SA3 0x76

//Register Defines for SAA1064
#define SAA1064_CTRL 0x00
#define SAA1064_DIG1 0x01
#define SAA1064_DIG2 0x02
#define SAA1064_DIG3 0x03
#define SAA1064_DIG4 0x04

//Control Register Defines for SAA1064
//Static display (2 digits) or Multiplexed (4 digits)
#define SAA1064_MPX  0x01
//Digits 1 and 2 On
#define SAA1064_B0   0x02
//Digits 3 and 4 On
#define SAA1064_B1   0x04
//Intensity of display
#define SAA1064_INT0 0x00
#define SAA1064_INT1 0x10
#define SAA1064_INT2 0x20
#define SAA1064_INT3 0x30
#define SAA1064_INT4 0x40
#define SAA1064_INT5 0x50
#define SAA1064_INT6 0x60
#define SAA1064_INT7 0x70

//Default Mode: Multiplex On, All Digits On
#define SAA1064_CTRL_DEF (SAA1064_MPX | SAA1064_B0 | SAA1064_B1)

//Pin Defines for SAA1064
#define D_L0                 0x01
#define D_L1                 0x02
#define D_L2                 0x04
#define D_L3                 0x08
#define D_L4                 0x10
#define D_L5                 0x20
#define D_L6                 0x40
#define D_L7                 0x80

const uint8_t SAA1064_SEGM[] = {0x3F,0x06, 0x5B,0x4F,
                                0x66,0x6D,0x7D,0x07,
                                0x7F,0x6F,0x77,0x7C,
                                0x39,0x5E,0x79,0x71};

#define SAA1064_DP              0x80   //Decimal Point
#define SAA1064_MINUS           0x40   //Minus Sign
#define SAA1064_BLNK            0x00   //Blank Digit
#define SAA1064_ALL             0xFF   //All Segments On

#define EIGHT_SEG_ADDRESS 0x38

const char BUFF_FF[1] = {0xFF};
const char BUFF_COL[4] = {0x7F,0xBF,0xDF,0xEF};
const char BUFF_SETUP[11] = {0x00, 0x34, 0x0C, 0x06, 0x35, 0x04, 0x10, 0x42, 0x9F, 0x34, 0x02};
const char BUFF_HELLO_WORLD[11] = {0xC8, 0xC5, 0xCC, 0xCC, 0xCF, 0xA0, 0xD7, 0xCF, 0xD2, 0xCC, 0xC4};
volatile unsigned long SysTickCnt;
static int LCDcount = 0;
volatile uint8_t data[512];
static uint8_t lamp_rgb_address[3] = {0,1,2};
const uint8_t KEY_TO_LCD_LOOKUP[4][4] = {{0xB1, 0xB2, 0xB3, 0xC1}, //1,2,3,A
                            {0xB4, 0xB5, 0xB6, 0xC2},   //4.5.6.B
                            {0xB7, 0xB8, 0xB9, 0xC3},   //7,8,9,C
                            {0xAA, 0xB0, 0xA3, 0xC4}};  //*,0.#,D


void SysTick_Handler (void);
void Delay (unsigned long tick);
void BreakFlagLow(void);
void BreakFlagHigh(void);
void Full_Init(void);
void PinCFG_Init(int funcnum);
void UART_Init2(void);
void I2C_Init2(void);
void get_keypad_press(char* read_buff);
//uint8_t read_keypad_press(void);
uint8_t decode_keypad(uint8_t input);
void read_i2c(char* buffer, int length, int address);
void write_i2c(char* buffer, int length, int address);
void set_basic_data(void);
void send_colours(uint8_t coloursRGB[][3], uint8_t length, uint32_t delay);
void LCD_clear(void);
void printKeyToLCD(int rrcc, int LCDcount);
void init_SEGMENTS();
void SEGMENT_WriteHidden(int value, uint8_t dp_digit, int leading);
void SEGMENT_Write(int int_value, int zeros);
void SEGMENT_WriteFloat(double double_value, int zeros);

void SysTick_Handler (void) {
  SysTickCnt++;
}
void Delay (unsigned long tick) {
  unsigned long systickcnt;
  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}

void BreakFlagLow(void){LPC_UART1->LCR |= 0x40;}
void BreakFlagHigh(void){LPC_UART1->LCR &= 0xBF;}


void init_SEGMENTS(){
  uint8_t data[6];

  data[0] = SAA1064_CTRL;                     // Select Control Reg
  data[1] = SAA1064_CTRL_DEF | SAA1064_INT3;  // Init Control Reg
  data[2] = SAA1064_BLNK;                     // Digit 1: All Segments Off
  data[3] = SAA1064_BLNK;                     // Digit 2: All Segments Off
  data[4] = SAA1064_BLNK;                     // Digit 3: All Segments Off
  data[5] = SAA1064_BLNK;                     // Digit 4: All Segments Off

  //data[2] = SAA1064_ALL;                      // Digit 1: All Segments On
  //data[3] = SAA1064_ALL;                      // Digit 2: All Segments On
  //data[4] = SAA1064_ALL;                      // Digit 3: All Segments On
  //data[5] = SAA1064_ALL;                      // Digit 4: All Segments On

  write_i2c(data, 6, EIGHT_SEG_ADDRESS);
}

void Full_Init(void){
  SysTick_Config(SystemCoreClock/1000000 - 1);

  // Init pins
  PinCFG_Init(1);
  // Init UART
  UART_Init2();
  //Enable TxD pin
  UART_TxCmd((LPC_UART_TypeDef *) LPC_UART1, ENABLE);
  // Init UART FIFO
  UART_FIFO_CFG_Type FIFOCfg;
  UART_FIFOConfigStructInit((UART_FIFO_CFG_Type *)&FIFOCfg); //default configuration
  UART_FIFOConfig((LPC_UART_TypeDef *) LPC_UART1, (UART_FIFO_CFG_Type *) &FIFOCfg);
  // Init I2C
  I2C_Init2();
  // Init LCD
  write_i2c(BUFF_SETUP, 11, LCD_ADDRESS);
  LCD_clear();
  Delay(1000);
  // Init 7 segment display
  init_SEGMENTS();
}
void PinCFG_Init(int funcnum){

  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = funcnum;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 15;
  PINSEL_ConfigPin(&PinCfg);
}
void UART_Init2(void){
  UART_CFG_Type UartCfg;
  UartCfg.Baud_rate = 250000; //4 us = 1 bit, 250000 bps
  UartCfg.Databits = UART_DATABIT_8; //8 data bits
  UartCfg.Stopbits = UART_STOPBIT_2; //2 stop bits
  UartCfg.Parity = UART_PARITY_NONE; //1 start bit
  UART_Init((LPC_UART_TypeDef *) LPC_UART1, &UartCfg);
}
void I2C_Init2(void){
  //configure mbed pins to work as I2C pins.
  PINSEL_CFG_Type pincfg1;
  pincfg1.Funcnum = 3;
  pincfg1.OpenDrain = 0;
  pincfg1.Pinmode = 0;
  pincfg1.Pinnum = 0;
  pincfg1.Portnum = 0;

  PINSEL_CFG_Type pincfg2;
  pincfg2.Funcnum = 3;
  pincfg2.OpenDrain = 0;
  pincfg2.Pinmode = 0;
  pincfg2.Pinnum = 1;
  pincfg2.Portnum = 0;
  PINSEL_ConfigPin(&pincfg1);
  PINSEL_ConfigPin(&pincfg2);

  //Set the I2C controller clock rate (100kbits/s)
  I2C_Init(LPC_I2C1, 100000);

  //Enable I2C bus controller
  I2C_Cmd(LPC_I2C1, ENABLE);
}

void SEGMENT_WriteFloat(double double_value, int zeros){
  double value;
  uint8_t no_of_digit = ceil(log10(double_value));
  uint8_t dp_digit = 0;
  int leading = 1;
  double decimals = double_value - (int)floor(double_value);

  if (zeros == 1){
    //leading is opposite to zeros
    leading = 0;
  }

  uint8_t dec_digit = 4 - no_of_digit;
  decimals = decimals * pow(10, dec_digit);
  double_value = double_value * pow(10, dec_digit);

  if(no_of_digit > 4 || no_of_digit < 0){
    //print("ERROR. MAX VALUE EXCEEDED");
    value = 9999;
    dp_digit = 4;
    leading = 0;
  }

  value = double_value + decimals;

  SEGMENT_WriteHidden(value, dp_digit, leading);
}

void read_i2c(char* buffer, int length, int address){
  I2C_M_SETUP_Type setup;

  setup.sl_addr7bit = address;
  setup.tx_data = NULL;
  setup.tx_length = 0;
  setup.rx_data = buffer;
  setup.rx_length = length;
  setup.retransmissions_max = 0;

  I2C_MasterTransferData(LPC_I2C1, &setup, I2C_TRANSFER_POLLING);
}
void write_i2c(char* buffer, int length, int address){
  I2C_M_SETUP_Type setup;

  setup.sl_addr7bit = address;
  setup.tx_data = buffer;
  setup.tx_length = length;
  setup.rx_data = NULL;
  setup.rx_length = 0;
  setup.retransmissions_max = 0;

  I2C_MasterTransferData(LPC_I2C1, &setup, I2C_TRANSFER_POLLING);
}

void get_keypad_press(char* read_buff){
  write_i2c(BUFF_FF,1, 0x21);
  int col = 0;
  while(1){
    write_i2c(&BUFF_COL[col],1,0x21);
    read_i2c(read_buff, 1, 0x21);
    if ((read_buff[0] & 0x0F) != 0x0F){
        char out = read_buff[0];
        while (1){
          read_i2c(read_buff, 1, 0x21);
          if ((read_buff[0] & 0x0F) == 0x0F){
            read_buff[0] = out;
            return;
          }
        }
    }
    col = (col+1) % 4;
  }
}

void SEGMENT_WriteHidden(int value, uint8_t dp_digit, int leading){
  uint8_t digit_value;
  uint8_t data[6];
  data[0] = SAA1064_DIG1;                     // Select Digit1 Reg

  // limit to valid range
  if (value >= 9999) value = 9999;
  if (value <= -999) value = -999;

  if (value >= 0) {
    // value 0...9999
    digit_value = value/1000; // compute thousands
    value = value % 1000;     // compute remainder
    if ((digit_value==0) && !(dp_digit==1) && leading )
      data[1] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[1] = SAA1064_SEGM[digit_value];
      leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==1) {data[1] = data[1] | SAA1064_DP;} // Set decimal point


    digit_value = value/100;  // compute hundreds
    value = value % 100;      // compute remainder
    if ((digit_value==0) && !(dp_digit==2) && leading)
      data[2] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[2] = SAA1064_SEGM[digit_value];
      leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==2) {data[2] = data[2] | SAA1064_DP;} // Set decimal point

    digit_value = value/10;   // compute tens
    value = value % 10;       // compute remainder
    if ((digit_value==0) && !(dp_digit==3) && leading)
      data[3] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[3] = SAA1064_SEGM[digit_value];
      //leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==3) {data[3] = data[3] | SAA1064_DP;} // Set decimal point

    //digit_value = value;      // compute units
    data[4] = SAA1064_SEGM[value];          // never suppress units zero
    if (dp_digit==4) {data[4] = data[4] | SAA1064_DP;} // Set decimal point

  }
  else {
    // value -999...-1
    value = -value;
    data[1] = SAA1064_MINUS;               // Sign
    if (dp_digit==1) {data[1] = data[1] | SAA1064_DP;} // Set decimal point

    digit_value = value/100;  // compute hundreds
    value = value % 100;      // compute remainder
    if ((digit_value==0) && !(dp_digit==2) && leading)
      data[2] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[2] = SAA1064_SEGM[digit_value];
      leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==2) {data[2] = data[2] | SAA1064_DP;} // Set decimal point

    digit_value = value/10;   // compute tens
    value = value % 10;       // compute remainder
    if ((digit_value==0) && !(dp_digit==3) && leading)
      data[3] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[3] = SAA1064_SEGM[digit_value];
      //leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==3) {data[3] = data[3] | SAA1064_DP;} // Set decimal point

    //digit_value = value;      // compute units
    data[4] = SAA1064_SEGM[value];          // never suppress units zero
    if (dp_digit==4) {data[4] = data[4] | SAA1064_DP;} // Set decimal point
  }

 write_i2c(data, 5, EIGHT_SEG_ADDRESS);
}

void SEGMENT_Write(int int_value, int zeros){
  uint8_t dp_digit = 0;
  int leading = 1;

  if (zeros == 1){
    //leading is opposite to zeros
    leading = 0;
  }
  SEGMENT_WriteHidden(int_value, dp_digit, leading);
}

//sends one whole block of data.
void set_basic_data(void){
  //Set Initial Data
  int i;
  for (i = 0; i < 512; i++){
    data[i] = 0xFF; // bits arrive in backwards order because endianness
  }
}

void send_data_UART(int wait){
  //MTBP
  BreakFlagHigh();
  Delay(2000);
  //BREAK
  BreakFlagLow();
  Delay(2000);
  //MAB
  BreakFlagHigh();
  Delay(40);
  //Send Data
  //First slot
  UART_SendByte(LPC_UART1, 0x00);
  Delay(50);
  //Send 512 slots from data[]
  UART_Send(LPC_UART1, &data, 512, BLOCKING); //BLOCKING or NONE_BLOCKING ?

  if (wait) while (UART_CheckBusy(LPC_UART1)==SET);
}
void send_colours(uint8_t coloursRGB[][3], uint8_t length, uint32_t delay){
  int i;
  for (i = 0; i < length; i++){
    //Set data
    data[lamp_rgb_address[0]] = coloursRGB[i][0];
    data[lamp_rgb_address[1]] = coloursRGB[i][1];
    data[lamp_rgb_address[2]] = coloursRGB[i][2];
    //Send data
    send_data_UART(WAIT);
    Delay(delay);
  }
}

void LCD_clear(void){
  int i;
  char address[2];
  address[0] = 0x00;
  char buff_char[2];
  buff_char[0] = 0x40;
  for(i = 0; i < 16; i++){
    address[1] = 0x80 + i;
    buff_char[1] = 0xA0;
    write_i2c(address, 2, LCD_ADDRESS);
    write_i2c(buff_char, 2, LCD_ADDRESS);
  }
  for(i = 0; i < 16; i++){
    address[1] = 0xC0 + i;
    buff_char[1] = 0xA0;
    write_i2c(address, 2, LCD_ADDRESS);
    write_i2c(buff_char, 2, LCD_ADDRESS);
  }
}

void printKeyToLCD(int rrcc, int LCDcount){
  int j = (rrcc >> 2) & 3;
  int i = rrcc & 3;
  int press = 0;
  char address[2];
  address[0] = 0x00;
  char buff_char[2];
  buff_char[0] = 0x40;

  address[1] = 0x80 + LCDcount;
  buff_char[1] = KEY_TO_LCD_LOOKUP[j][i];
  write_i2c(address, 2, LCD_ADDRESS);
  write_i2c(buff_char, 2, LCD_ADDRESS);
  if (LCDcount == 16){
    LCDcount = 40;
  } else if (LCDcount == 56) {
    LCDcount = 0;
  }
  LCDcount++;
}
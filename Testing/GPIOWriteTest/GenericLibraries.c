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

const char BUFF_FF[1] = {0xFF};
const char BUFF_COL[4] = {0x7F,0xBF,0xDF,0xEF};
volatile unsigned long SysTickCnt;
volatile uint8_t data[512];
static uint8_t lamp_rgb_address[3] = {0,1,2};


void SysTick_Handler (void);
void Delay (unsigned long tick);
void BreakFlagLow(void);
void BreakFlagHigh(void);
void Full_Init(void);
void PinCFG_Init(int funcnum);
void UART_Init2(void);
void I2C_Init2(void);
void get_keypad_press(char* read_buff);
char read_keypad_press(void);
void read_i2c(char* buffer, int length, int address);
void write_i2c(char* buffer, int length, int address);
void send_colours(uint8_t coloursRGB[][3], uint8_t length, uint32_t delay);

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
char read_keypad_press(void){
  char read_buff[1];
  write_i2c(BUFF_FF,1, 0x21);
  int col = 0;
  for (col = 0; col < 4; col++){
    write_i2c(&BUFF_COL[col],1,0x21);
    read_i2c(read_buff, 1, 0x21);
    //save the read buff value when pressed (!=0x0F)
    if (read_buff[0] & 0x0F){
        return read_buff[0];
    }
  }
  return 0x0F;
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

//sends one whole block of data.
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


int main(){
  Full_Init();

  //Set Initial Data
  int i;
  for (i = 0; i < 512; i++){
    data[i] = 0xFF; // bits arrive in backwards order because endianness
  }

  char read_buff[0];

  uint8_t colours[7][3] = { {255,0,255},
                          {255,0,0},
                          {0,0,255},
                          {0,255,0},
                          {255,255,255},
                          {100,100,100},
                          {0,255,255} };

  send_colours(colours, 7, 500000);

  //Main loop
  while(1){
    get_keypad_press(read_buff);

    switch(read_buff[0]&0xF0){
      case 0x70:
        for (i = 0; i < 512; i++){
          data[0] = 0xFF; // bits arrive in backwards order because endianness
          data[1] = 0x00;
          data[2] = 0x00;
        }
        break;
      case 0xB0:
        for (i = 0; i < 512; i++){
          data[0] = 0x00; // bits arrive in backwards order because endianness
          data[1] = 0xFF;
          data[2] = 0x00;
        }
        break;
      case 0xD0:
        for (i = 0; i < 512; i++){
          data[0] = 0x00; // bits arrive in backwards order because endianness
          data[1] = 0x00;
          data[2] = 0xFF;
        }
        break;
      case 0xE0:
        for (i = 0; i < 512; i++){
          data[0] = 0xAA; // bits arrive in backwards order because endianness
          data[1] = 0xAA;
          data[2] = 0xAA;
        }
        break;
    }

    send_data_UART(WAIT);
  }
}

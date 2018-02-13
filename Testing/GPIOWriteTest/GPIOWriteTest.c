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

const char BUFF_FF[1] = {0xFF};
const char BUFF_COL[4] = {0x7F,0xBF,0xDF,0xEF};


TIM_MATCHCFG_Type TIM_MatchConfigStruct;
TIM_TIMERCFG_Type TIM_ConfigStruct;
volatile unsigned long SysTickCnt;
volatile uint8_t data[512];
void SysTick_Handler (void);
void Delay (unsigned long tick);

void SysTick_Handler (void) {
  SysTickCnt++;
}

void Delay (unsigned long tick) {
  unsigned long systickcnt;
  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}

//based on UART_ForceBreak from library
void SendBreakLow(int delay) {
  LPC_UART1->LCR |= 0x40;
  Delay(delay);
  LPC_UART1->LCR &= ~(0x40);
  Delay(delay);
}
//void SendBreakHI

void Init(void){

  //Initialise UART (3 parts)
  // 1/3: Init pins
  PinCFG_Init(1);

  // 2/3: Init UART
  UART_Init2();

  //Enable TxD pin
  UART_TxCmd(LPC_UART1, ENABLE);

  // 3/3: Init UART FIFO
  UART_FIFO_CFG_Type FIFOCfg;
  UART_FIFOConfigStructInit(&FIFOCfg); //default configuration
  UART_FIFOConfig(LPC_UART1, &FIFOCfg);

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
  UART_Init(LPC_UART1, &UartCfg);
}

void Break_Signal(void){



  UART_Init2();
  return;
}

void init_data(){

}

void save_data(){

}
/*
void EL_SERIAL_Init(void)
{
  PcINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 1;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;

  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 2;
  PINSEL_ConfigPin(&PinCfg);

  PinCfg.Pinnum = 3;
  PINSEL_ConfigPin(&PinCfg);

  UART_CFG_Type UARTCfg;
  UART_FIFO_CFG_Type FIFOCfg;
  UART_ConfigStructInit(&UARTCfg);
  UART_FIFOConfigStructInit(&FIFOCfg);

  UART_FIFOConfig(LPC_UART0, &FIFOCfg);
  UART_Init(LPC_UART0, &UARTCfg);

  UART_TxCmd(LPC_UART0, ENABLE);
}


size_t EL_SERIAL_SizeOfString(uint8_t string[])
{
  size_t length = 0;
  while (string[length] != '\0')
    length++;
  return length + 1;
}

void EL_SERIAL_Print(uint8_t string[])
{
  UART_Send(LPC_UART0, string, EL_SERIAL_SizeOfString(string), BLOCKING);
}
*/

/*void send_slot(uint8_t byte){
  //start bit
  GPIO_ClearValue(0,SENDING);
  Delay(BIT_TIME);
  int i;
  for (i = 0; i < 8; i++){
    if (((byte << i) & 0x80) == 0x80){
      GPIO_SetValue(0,SENDING);
    } else {
      GPIO_ClearValue(0,SENDING);
    }
    Delay(BIT_TIME);
  }
  //closing bit
  GPIO_SetValue(0,SENDING);
  Delay(2*BIT_TIME);
}*/

//sends one whole block of data.
void send_data_UART(void){
  //First slot
  UART_SendByte(LPC_UART1, 0x00);
  Delay(50);
  UART_Send(LPC_UART1, &data, 512, BLOCKING); //BLOCKING or NONE_BLOCKING ?

}

void send_data_GPIO(void){
  //MAB: prepare data ahead of time
  // -> no data to be prepared?
  //mode slot
  GPIO_ClearValue(0,SENDING);
  Delay(BIT_TIME*9);
  GPIO_SetValue(0,SENDING);
  Delay(BIT_TIME*4);

  //512 slots
  int c;
  for (c = 0; c < 512; c++){
    // MTBF: prepare data ahead of time
    uint8_t register byte = data[c];

    //start bit
    GPIO_ClearValue(0,SENDING);
    Delay(4);
    // "for-loop" is hard-coded to maximize efficiency.
    if (byte & 0x80) GPIO_SetValue(0, SENDING); else GPIO_ClearValue(0, SENDING); Delay(4);
    if (byte & 0x40) GPIO_SetValue(0, SENDING); else GPIO_ClearValue(0, SENDING); Delay(4);
    if (byte & 0x20) GPIO_SetValue(0, SENDING); else GPIO_ClearValue(0, SENDING); Delay(4);
    if (byte & 0x10) GPIO_SetValue(0, SENDING); else GPIO_ClearValue(0, SENDING); Delay(4);
    if (byte & 0x08) GPIO_SetValue(0, SENDING); else GPIO_ClearValue(0, SENDING); Delay(4);
    if (byte & 0x04) GPIO_SetValue(0, SENDING); else GPIO_ClearValue(0, SENDING); Delay(4);
    if (byte & 0x02) GPIO_SetValue(0, SENDING); else GPIO_ClearValue(0, SENDING); Delay(4);
    if (byte & 0x01) GPIO_SetValue(0, SENDING); else GPIO_ClearValue(0, SENDING); Delay(4);

    //closing bit
    GPIO_SetValue(0,SENDING);
    Delay(4*BIT_TIME);
  }
}

void TimerConfig(void){
  TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
  TIM_ConfigStruct.PrescaleValue = 500000;
  TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM_ConfigStruct);
}

char read_keypad_press(){
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
  I2C_M_SETUP_Type setup;char get_keypad_press(){
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

  setup.sl_addr7bit = address;
  setup.tx_data = buffer;
  setup.tx_length = length;
  setup.rx_data = NULL;
  setup.rx_length = 0;
  setup.retransmissions_max = 0;

  I2C_MasterTransferData(LPC_I2C1, &setup, I2C_TRANSFER_POLLING);
}

void init_I2C(void){
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

int main(){
  TimerConfig();
  SysTick_Config(SystemCoreClock/1000000 - 1);


  int i;
  for (i = 0; i < 512; i++){
    data[i] = 0xFF; // bits arrive in backwards order because endianness
  }

  char read;
  char read_buff[0];
  Init();



/*test for keypad press stuff*/
  init_I2C();


/*
  while(1){
    UART_SendByte(LPC_UART1, 0x00);
    UART_SendByte(LPC_UART1, 0x88);
    UART_SendByte(LPC_UART1, 0xFF);
  }
*/


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
    //Main loop
  while(1){
    //MTBP
    LPC_UART1->LCR &= 0xBF;
    Delay(2000);
    //Customize data

    /*read = get_keypad_press();//translate keypad press to send send data
    if (read == 0x00){
      for (i = 0; i < 512; i++){
        data[i] = 0x00; // bits arrive in backwards order because endianness
      }
    } else {
      for (i = 0; i < 512; i++){
        data[i] = 0xFF; // bits arrive in backwards order because endianness
      }
    }*/
    //BREAK
    LPC_UART1->LCR |= 0x40;
    Delay(2000);
    //MAB
    LPC_UART1->LCR &= 0xBF;
    Delay(40);
    //Send Data
    send_data_UART();
    while (UART_CheckBusy(LPC_UART1)==SET);
    
  }
}

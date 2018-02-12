#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"

#include <stdio.h>
#include <lpc17xx_timer.h>

#define SENDING 0x40040
#define RECEIVING 0x20000
#define STOP 0x000000
#define BIT_TIME 3

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

void Init(void){

  //Initialise UART (3 parts)
  // 1/3: Init pins
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 1;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 15;
  PINSEL_ConfigPin(&PinCfg);

  // 2/3: Init UART
  UART_CFG_Type UartCfg;
  UartCfg.Baud_rate = 250000; //4 us = 1 bit, 250000 bps
  UartCfg.Databits = UART_DATABIT_8; //8 data bits
  UartCfg.Stopbits = UART_STOPBIT_2; //2 stop bits
  UartCfg.Parity = UART_PARITY_NONE; //1 start bit
  UART_Init(LPC_UART1, &UartCfg);

  //Enable TxD pin
  UART_TxCmd(LPC_UART1, ENABLE);

  // 3/3: Init UART FIFO
  UART_FIFO_CFG_Type FIFOCfg;
  UART_FIFOConfigStructInit(&FIFOCfg); //default configuration
  UART_FIFOConfig(LPC_UART1, &FIFOCfg);

}



void init_data(){

}

void save_data(){

}
/*
void EL_SERIAL_Init(void)
{
  PINSEL_CFG_Type PinCfg;
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

int main(){
  TimerConfig();

  SysTick_Config(SystemCoreClock/1000000 - 1);

  //EL_SERIAL_Print("HI");
  //EL_SERIAL_Init();

  //static unsigned int DMXDATA[11] = {STOP, 0xF0000, 0xF0000, 0x30000, STOP, STOP, STOP, STOP, STOP, STOP};


  GPIO_SetDir(0, SENDING, 1);
  GPIO_SetDir(0, RECEIVING, 0);

  int i;
  for (i = 0; i < 512; i++){
    data[i] = 0x01; // bits arrive in backwards order because endianness
  }


  Init(); //Initialise UART
  /*
  while(1){
    UART_SendByte(LPC_UART1, 0x01);
  }
  */
  //Main loop
  while(1){
    /*
    TIM_MatchConfigStruct.MatchChannel = 0;
    TIM_MatchConfigStruct.ResetOnMatch = TRUE;
    TIM_MatchConfigStruct.MatchValue = 1;
    TIM_ConfigMatch(LPC_TIM0,&TIM_MatchConfigStruct);
    */


    //idle mode [set data here]
    GPIO_SetValue(0,SENDING);
    Delay(1000);

    //Break
    GPIO_ClearValue(0,SENDING);
    Delay(880);
    //MAB
    GPIO_SetValue(0,SENDING);
    Delay(100);
    //send_data_GPIO();
    send_data_UART();

  }


}

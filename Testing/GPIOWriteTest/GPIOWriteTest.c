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

void init_data(){

}

void save_data(){

}

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

void send_data(){
  //mode slot
  GPIO_ClearValue(0,SENDING);
  Delay(BIT_TIME*9);
  GPIO_SetValue(0,SENDING);
  Delay(BIT_TIME*4);

  //512 slots
  int c;
  int i;
  for (c = 0; c < 512; c++){
    //send_slot(data[c]);
    //start bit
    GPIO_ClearValue(0,SENDING);
    Delay(3);
    for (i = 0; i < 8; i++){
      if (((data[c] << i) & 0x80) == 0x80){
        GPIO_SetValue(0,SENDING);
      } else {
        GPIO_ClearValue(0,SENDING);
      }
      Delay(2);
    }
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
  EL_SERIAL_Init();
  EL_SERIAL_Print("HI");
  static unsigned int DMXDATA[11] = {STOP, 0xF0000, 0xF0000, 0x30000, STOP, STOP, STOP, STOP, STOP, STOP};
  GPIO_SetDir(0, SENDING, 1);
  GPIO_SetDir(0, RECEIVING, 0);
  int i;
  int j;
  for(j=0;j<256;j++){
    data[j] = j;
  }


  while(1){
    TIM_MatchConfigStruct.MatchChannel = 0;
    TIM_MatchConfigStruct.ResetOnMatch = TRUE;
    TIM_MatchConfigStruct.MatchValue = 1;
    TIM_ConfigMatch(LPC_TIM0,&TIM_MatchConfigStruct);


    //idle mode
    GPIO_SetValue(0,SENDING);

    //configure data
    for (i = 0; i < 512; i++){
      data[i] = data[i % 256];
    }
    Delay(1000);

    //Break
    GPIO_ClearValue(0,SENDING);
    Delay(880);
    //MAB
    GPIO_SetValue(0,SENDING);
    Delay(100);
    //send_data();
  }


}

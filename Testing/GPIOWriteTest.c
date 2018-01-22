#include "lpc17xx_gpio.h"
#include <stdio.h>



#define SENDING 0x40040
#define RECEIVING 0x20000
#define STOP 0x000000


volatile unsigned long SysTickCnt;
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



int main(){
    SysTick_Config(SystemCoreClock/1000 - 1);


  static unsigned int DMXDATA[11] = {STOP, 0xF0000, 0xF0000, 0x30000, STOP, STOP, STOP, STOP, STOP, STOP};
  GPIO_SetDir(0, SENDING, 1);
  GPIO_SetDir(0, RECEIVING, 0);
  int i;
  int b = 0;
  while(1){
    for(i = 0; i < 11 ; i++){
      GPIO_SetValue(0, 0xF);
      Delay(100);
      GPIO_ClearValue(0,0xF);
      GPIO_SetValue(0, 0x0);
      Delay(100);
      GPIO_ClearValue(0,0x0);
    }
  }
}

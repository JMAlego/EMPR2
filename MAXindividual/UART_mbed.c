//Individual project Maxime Franchot EMPR group 4
#define NEXTLINE "\r\n"
#define CLEARLINE "\r                                                                            \r"
#define INPUTSIZE 36
//#include "GenericLibraries.c"
#include "lib/empr_lib_serial.c"
#include "lpc17xx_uart.h"
#include <stdio.h>


void clear_array(uint8_t array[INPUTSIZE]){
  int i;
  for (i = 0; i < INPUTSIZE; i++){
    array[i] = "";
  }
}

void command(uint8_t input[INPUTSIZE]){
  //"switch" on the full command:
  if (strcmp(input,"test")==1){
    EL_SERIAL_Print("\n\rTesting Successful!\n\r");
    clear_array(input);
  } else {
    EL_SERIAL_Print("Command not recognized. Type \"help\" for a list of commands.\n\r");
  }

}


int main() {
  EL_SERIAL_Init();
  EL_SERIAL_Print("\n");
  EL_SERIAL_Print("USB test code\n\r");
  uint8_t in[INPUTSIZE];
  uint8_t inc = 0;
  uint8_t byte[2];
  byte[1] = 0;



  while(1){
    byte[0] = UART_ReceiveByte(LPC_UART0);
    switch(byte[0]){
      case '\r': //ENTER key
        EL_SERIAL_Print(NEXTLINE);
        inc = 0;
        command(in);
        break;
      case '\x08': //BACKSPACE
        break;
        //EL_SERIAL_Print()
      default: {
        //terminal sometimes gets stuck. this unsuccessfully attempts to fix it.
        while(UART_CheckBusy(LPC_UART0));
        EL_SERIAL_Print(byte);
        in[inc] = byte[0];
        inc++;
      }
    }
  }

}

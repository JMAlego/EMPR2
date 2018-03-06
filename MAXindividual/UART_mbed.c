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
    array[i] = 0;
  }
}

uint8_t compare_command(uint8_t input[INPUTSIZE], uint8_t command[INPUTSIZE]){
  int i;
  /*EL_SERIAL_Print(NEXTLINE);
  EL_SERIAL_Print(input);
  EL_SERIAL_Print(NEXTLINE);
  EL_SERIAL_Print(command);*/
  for (i = 0; i < INPUTSIZE; i++){
    if ((input[i] != 0) && (input[i] != command[i])){
      //DEBUG uint8_t output[32];
      //DEBUG sprintf(output, "\"%c\" is different from \"%c\"", input[i], command[i]);
      //DEBUG EL_SERIAL_Print(output);
      return 0;
    }
  }
  //DEBUG EL_SERIAL_Print("Command registered!");
  return 1;
}

void command(uint8_t input[INPUTSIZE]){
  //"switch" on the full command:

  //DEBUG uint8_t output[32];
  //DEBUG sprintf(output, "\n\rCommand registered: %d", compare_command(input,"test"));
  //DEBUG EL_SERIAL_Print(output);

  if (compare_command(input,"test")){
    EL_SERIAL_Print("\n\rTesting Successful!\n\r");
    clear_array(input);
  } else if (compare_command(input, "help")){
    EL_SERIAL_Print("\n\n\rPossible commands: \n\r\t-help\n\r\t-repeat\n\r\t-display\n\r\t-transition\n\r");
  } else {
    EL_SERIAL_Print("\n\rCommand not recognized. Type \"help\" for a list of commands.\n\r");
  }

  clear_array(input);

}


int main() {
  EL_SERIAL_Init();
  EL_SERIAL_Print("\n");
  EL_SERIAL_Print("USB test code\n\r");
  uint8_t in[INPUTSIZE];
  clear_array(in);
  uint8_t inc = 0;
  uint8_t byte[2];
  byte[1] = 0;


  //Main loop: takes in command line input and modifies everything accordingly.
  while(1){
    byte[0] = UART_ReceiveByte(LPC_UART0);
    if (byte[0]){
      switch(byte[0]){
        case 38: //Up Arrow (no effect)
        case 40: //Down Arrow (no effect)
        case 37: inc--; //move insert left
        case 39: inc++; //move insert right
        case '\r': //ENTER key
          EL_SERIAL_Print(NEXTLINE);

          inc = 0;
          command(in);
          break;
        case '\x08': //BACKSPACE
          in[inc--] = " ";
          EL_SERIAL_Print((char)8);
          break;
        default: {
          //terminal sometimes gets stuck. this unsuccessfully attempts to fix it.
          while(UART_CheckBusy(LPC_UART0));

          EL_SERIAL_Print(byte);
          //uint8_t output[32];
          //sprintf(output, "Value of inc: %d. Value inputted: %02d", inc, byte[0]);
          //EL_SERIAL_Print(output);
          in[inc] = byte[0];
          inc++;
        }
      }
    }
  }
}

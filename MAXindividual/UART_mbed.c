//Individual project Maxime Franchot EMPR group 4
#define out_of_sequence_range(num) (command_parameters[num] <= 16) && (command_parameters[num] >= 0)
#define NEXTLINE "\r\n"
#define CLEARLINE "\r                                                                            \r"
#define INPUTSIZE 36
//#include "GenericLibraries.c"
#include "lib/empr_lib_serial.c"
#include "lpc17xx_uart.h"
#include "GenericLibraries.c"
#include <stdio.h>

uint8_t command_parameters[8];



//Settings
enum eTransitionMode{Switch,Black,Fade,FadeBlack};

//Data
uint8_t colour[10][3];
struct Sequence {
  uint8_t colours[16];
  uint8_t length;
  uint8_t repeat;
  uint8_t transition_mode;
  uint8_t transition_speed;
  uint8_t hold_time;
};
struct Sequence sequence[16];


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
    //for special to-be-saved input
    if (command[i] == '/') {
      return 1;
    } else if ((input[i] != 0) && (input[i] != command[i])){
      return 0;
    }
  }
  return 1;
}

uint8_t get_cmd_input(uint8_t input[INPUTSIZE], uint8_t params){
  uint8_t i;
  uint8_t input_count = 0;
  for (i = 0; i < INPUTSIZE; i++){
    uint8_t param = 0;
    //continues scrolling through array incrementing i, but registering the parameters.
    if (input[i] == ' '){
      while ((input[i+1] != ' ') && (input[i+1])){
        i++;
        //translate char to int: (x - 128), and add to number
        param = (param * 10) + (input[i]-48);
      }
      command_parameters[input_count] = param;
      //So for every space, aka parameter, the input_count is incremented.
      input_count++;
    }
  }
  return (input_count == params);
}


void command(uint8_t input[INPUTSIZE]){
  //"switch" on the full command:
  //uint8_t output[32];//DEBUG

  uint8_t output[50];

  //Test
  if (compare_command(input,"test/")){
    EL_SERIAL_Print("Testing light...");
    test_light();
    EL_SERIAL_Print("Testing over.");
  }
  //Help
  else if (compare_command(input, "help/")){
    EL_SERIAL_Print("\n\n\rPossible commands: \n\r\t-help\
    \n\r\t-repeat s r   :  s->sequence number, r->times to repeat\
    \n\r\t-displayseq s :  s->sequence number\
    \n\r\t-displaycol r g b :  r->red value, g->green value, b->blue value\
    \n\r\t-trans s x    :  s->sequence number, \n\r    x:{0-\"Switch\", 1-\"Black\", 2-\"Fade\", 3-\"Fade through black\"} \n\r");
  }
  //Repeat
  else if (compare_command(input, "repeat/")){
    if(get_cmd_input(input,2)){
      if(out_of_sequence_range(0)){
        sequence[command_parameters[0]].repeat = command_parameters[1];
        EL_SERIAL_Print("Set repeat.");
      } else {
        EL_SERIAL_Print("Sequence number out of range 0-16.");
      }
    } else {
      EL_SERIAL_Print("\n\rWrong parameter inputs. [repeat s r   :  s->sequence number, r->times to repeat]\n\r");
    }
  }
  //Add to Sequence
  else if (compare_command(input, "addseq/")){
    if(get_cmd_input(input,2)){
      if(out_of_sequence_range(0)){
        sequence[command_parameters[0]].colours[sequence[command_parameters[0]].length+1] = command_parameters[1];
        sprintf(output, "Added to sequence %d.",command_parameters[0]);
        ++(sequence[command_parameters[0]].length);
        EL_SERIAL_Print(output);
      } else {
        EL_SERIAL_Print("Sequence number out of range 0-16, or colour number out of range 0-9");
      }
    } else {
      EL_SERIAL_Print("\n\rWrong parameter inputs. [repeat s r   :  s->sequence number, r->times to repeat]\n\r");
    }
  }
  //Set colour
  else if (compare_command(input, "setcol/")){

  }
  //Display Sequence
  else if (compare_command(input, "display/")){
    if(get_cmd_input(input,1)){
      if(command_parameters[0] > 16){
        EL_SERIAL_Print("Sequence number out of range 0-16.");
      } else {
        EL_SERIAL_Print("Displaying sequence:");
        display_sequence(command_parameters[0]);
        EL_SERIAL_Print("Sequence over.");
      }
    } else {
      EL_SERIAL_Print("\n\rWrong parameter inputs. [display s   :  s->sequence number]\n\r");
    }
  }
  //Anything else.
  else {
    EL_SERIAL_Print("\n\rCommand not recognized. Type \"help\" for a list of commands.\n\r");
  }
  clear_array(input);
  EL_SERIAL_Print("\n\r>>");
}

void display_sequence(struct Sequence* sequence){
  uint8_t i;
  uint8_t j;
  for (i = 0; i < sequence->repeat; i++){
    for (j = 0; i < sequence->length; j++){
      //send_data_UART(BLOCKING);
      if (sequence->transition_mode == Switch) {
        //delay
        send_data_UART(BLOCKING);
      } else if (sequence->transition_mode == Black) {
        //turn off
        //delay
        //display next colour
      } else if (sequence->transition_mode == Fade) {
        //using delay, loop and slowly go from one colour to another
      } else if (sequence->transition_mode == FadeBlack) {
        //using delay, loop and slowly go to black
        //then, slowly go to next colour
      }
    }
  }
}

void test_light(void){
  uint8_t i;
  for (i = 0; i < 3; i++){
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;

    data[i] = 255;

    send_data_UART(BLOCKING);
    Delay(300000);
  }
}

int main(void) {
  Full_Init();

  EL_SERIAL_Init();
  EL_SERIAL_Print("\n");
  EL_SERIAL_Print("ENTER COMMAND. Type \"help\" for guidance.\n\r>>");
  uint8_t in[INPUTSIZE];
  clear_array(in);
  uint8_t inc = 0;
  uint8_t byte[2];
  byte[1] = 0;

  //test_light();


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
          byte[0] = 0;
          inc = 0;
          command(in);
          break;
        case '\x08': //BACKSPACE
          in[inc--] = " ";
          EL_SERIAL_Print((char)8);
          break;
        default: {
          EL_SERIAL_Print(byte);
          in[inc] = byte[0];
          inc++;
        }
      }
    }
  }
}

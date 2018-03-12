//Individual project Maxime Franchot EMPR group 4
#define in_range(var, max) (var <= max) && (var >= 0)
#define is_numchar(c) in_range((c-48),9)
#define NEXTLINE "\r\n"
#define CLEARLINE "\r                                                                            \r"
#define INPUTSIZE 36
//#include "GenericLibraries.c"
#include "lib/empr_lib_serial.c"
#include "lpc17xx_uart.h"
#include "GenericLibraries.c"
#include <stdio.h>

uint16_t command_parameters[8];

uint8_t output[50];
uint16_t delay = 200000;


//Settings
enum eTransitionMode{Switch,Black,Fade,FadeBlack};

//Data
uint8_t colour[16][3];
struct Sequence {
  uint8_t colours[16];
  uint8_t length;
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
    uint16_t param = 0;
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

void load_and_play_seq(uint8_t seq[], uint8_t start, uint8_t length, uint64_t delay){
  sprintf(output, "Display sequence with length %d from %d\n\r", length, start);
  EL_SERIAL_Print(output);
  if (!length){
    return;
  }

  uint8_t i;
  for (i = start; i < length; i++){
    sprintf(output, "R: %d, G: %d, B: %d", colour[seq[i]][0], colour[seq[i]][1], colour[seq[i]][2]);
    EL_SERIAL_Print(output);
    data[0] = colour[seq[i]][0];
    data[1] = colour[seq[i]][1];
    data[2] = colour[seq[i]][2];

    send_data_UART(BLOCKING);

    Delay(delay);
  }
}

/*
* The following functions work like this:
* give params:char[] input and uint8_t*
* scrolls through input starting at and incrementing i;
* returns -1 if invalid characters are read,
* if all characters are valid, it returns once the number has been read.
*/
int16_t read_num_chars(uint8_t input[INPUTSIZE], uint8_t* i){
  uint16_t num = 0;
  uint8_t empty = 1;
  while (1) {
    //sprintf(output, "Reading char at %d: %d", *i, input[*i]-48);
    //EL_SERIAL_Print(output);
    //if number char (0-9) is read, add digit to 'num'.
    if (((input[*i]-48) <= 9) && ((input[*i]-48) >= 0)){
      //EL_SERIAL_Print("\n\rchar value read.");
      empty = 0;
      num = num*10 + (input[*i]-48);
    }
    //if something else than a digit is read, return.
    else {
      if (empty){
        return -1;
      } else {
        //EL_SERIAL_Print("\n\rother value read.");
        return num;
      }
    }
    ++(*i);
  }
}
//read_seq_val can be used to read colour reference values, as the range is also 0-16.
int8_t read_seq_val(uint8_t input[INPUTSIZE], uint8_t* i){
  uint16_t seq;
  while(1){
    if (is_numchar(input[*i])){
      seq = read_num_chars(input, i);
      if (in_range(seq,15)){
        return seq;
      } else {
        return -1;
      }
    }
    ++(*i);
  }
}
int16_t read_rgb_val(uint8_t input[INPUTSIZE], uint8_t* i){
  uint16_t val;
  while(1){
    switch (input[*i]) {
      case ' ': break;
      default:
        if (is_numchar(input[*i])){
          val = read_num_chars(input, i);
          if (in_range(val,255)){
            return val;
          } else {
            return -1;
          }
        }
    }
    ++(*i);
  }
}

void disp_help(void){
  EL_SERIAL_Print("\n\n\rPossible commands: \n\r\t-help\
  \n\r\t>setcol c r g b --- set colour <c> to {r,g,b} value.\
  \n\r\t>setseq s <sequence> --- set sequence <s> to <sequence>. \
  \n\r\t\t<sequence> format: colour-colour... E.G: c-c-(r,g,b)-c\
  \n\r\t>playseq <sequences> --- play <sequences>.\
  \n\r\t\t<sequences> format: sequence x repeat_times : transition_mode : sequence - (sequence, startcol, endcol)... E.G: s-(s, a, b):t:sx3");
}
void setcol(uint8_t c, uint16_t r, uint16_t g, uint16_t b){
  if(
    in_range(c, 16) &&
    in_range(r,255) &&
    in_range(g,255) &&
    in_range(b,255)
  ){
    colour[c][0] = r;
    data[0] = colour[c][0];
    colour[c][1] = g;
    data[1] = colour[c][1];
    colour[c][2] = b;
    data[2] = colour[c][2];
    sprintf(output, "set colour %d to: [r:%d,g:%d,b:%d]",c,r,g,b);
    EL_SERIAL_Print(output);
    send_data_UART(BLOCKING);
  } else {
    EL_SERIAL_Print("Invalid parameters.\
    \n\r\tc in range 0-16,\
    \n\r\tr,g,b in range 0-255");
  }
}
void setseq(uint8_t input[INPUTSIZE]){

  //read sequence char.
  uint8_t i = 7;
  int8_t s = read_seq_val(input, &i);
  if (s == -1){
    EL_SERIAL_Print("Please enter a valid sequence number: 0-16");
    return;
  }

  //check for space character after sequence val.
  if (input[i] == ' '){
    i++;
  } else {
    EL_SERIAL_Print("Please enter the sequence colour pattern in the form c-c-c...");
    return;
  }

  //read through actual sequence.
  uint8_t scount = 0;
  while(1){
    switch (input[i]) {
      case '-':
        i++;
        scount++;
        break;
      default:
        if(is_numchar(input[i])){
          sequence[s].colours[scount] = read_seq_val(input, &i);
          //sprintf(output, "\n\rSequence %d colour number %d is now colour %d.",s, scount, sequence[s].colours[scount]);
          //EL_SERIAL_Print(output);
        } else if (input[i]==0){
          sequence[s].length = scount;
          sprintf(output, "Sequence %d defined as: %d",s,sequence[s].colours[0]);
          EL_SERIAL_Print(output);
          uint8_t j;
          for (j=0; j<scount; j++){
            sprintf(output, "-%d", sequence[s].colours[j+1]);
            EL_SERIAL_Print(output);
          }
          EL_SERIAL_Print(".");
          return;
        } else {
          EL_SERIAL_Print("Invalid input. Type 'help' for guidance.");
          return;
        }
    }
  }

  /*
  uint8_t i;
  //start reading after "setseq " (7 chars)
  for (i = 7; i < INPUTSIZE; i++) {
    switch (input[i]) {
      case '(':
        if ( != -1){

        }
      default:
        if (in_range((input[i]-48),10)) {
          read_num_chars(input,&i);
        } else if ()
        EL_SERIAL_Print("no parenthesis detected.");
    }
  }*/
}
void playseq(uint8_t input[INPUTSIZE]){
  uint8_t i = 7;

  uint8_t to_play[16]; //array of colour references
  uint8_t to_play_length;
  uint8_t to_play_next; //colour of the next sequence

  //read through text, at every "end of transition", play what has been read.
  while(1){
    //read
    switch (input[i]) {
      case ' ':
        EL_SERIAL_Print("Read space\n\r");
        i++;
        break;
      /*case '(':
        uint8_t i;
        while(input[i] != ')'){

        }
        if ()*/
      default:
        if (is_numchar(input[i])){
          //EL_SERIAL_Print("Read number\n\r");
          uint8_t seq = read_seq_val(input, &i);
          //sprintf(output, "displaying sequence %d: ", colour[sequence[seq].colours[0]][0]);
          //EL_SERIAL_Print(output);

          load_and_play_seq(sequence[seq].colours, 0, sequence[seq].length, 300000);
        } else {
          //EL_SERIAL_Print("Read nothing.\n\r");
          return;
        }
    }
    //exec
  }
}
void set_default_data(void){
  colour[0][0] = 255;
  colour[0][1] = 0;
  colour[0][2] = 0;
  colour[1][0] = 0;
  colour[1][1] = 255;
  colour[1][2] = 0;

  sequence[0].colours[0] = 1;
  sequence[0].colours[1] = 0;
  sequence[0].colours[2] = 1;
  sequence[0].colours[3] = 0;

  sequence[0].length = 4;
}

void command(uint8_t input[INPUTSIZE]){
  //"switch" on the full command:
  //uint8_t output[32];//DEBUG
  //Test
  if (compare_command(input,"test/")){
    EL_SERIAL_Print("Testing light...");
    test_light();
    EL_SERIAL_Print("Testing over.");
  }
  //DEBUG readtests
  else if(compare_command(input, "readtestnum/")){
    uint8_t count = 12;
    sprintf(output, "\n\rnum: %d", read_num_chars(input, &count));
    EL_SERIAL_Print(output);
  }else if(compare_command(input, "readtestseq/")){
    uint8_t count = 12;
    sprintf(output, "\n\rseq:%d", read_seq_val(input, &count));
    EL_SERIAL_Print(output);
  }else if(compare_command(input, "readtestrgb/")){
    uint8_t count = 12;
    sprintf(output, "\n\rrgb:%d", read_rgb_val(input, &count));
    EL_SERIAL_Print(output);
  }
  //Help
  else if (compare_command(input, "help/")){
    disp_help();
  }
  //Set Colour
  else if (compare_command(input, "setcol/")){
    if(get_cmd_input(input, 4)){
      setcol(command_parameters[0],command_parameters[1],command_parameters[2],command_parameters[3]);
    } else {
        EL_SERIAL_Print("Invalid parameters.\
       \n\r\t >setcol c r g b --- set colour <c> to {r,g,b} value.");
    }
  }
  //Set Sequence
  else if (compare_command(input, "setseq/")){
    setseq(input);
  }
  //Play Sequence
  else if (compare_command(input, "playseq/")){
    playseq(input);
  }
  //Set default Data
  else if (compare_command(input, "setdef/")){
    set_default_data();
  }
  /*
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
  }*/
  //Anything else.
  else {
    EL_SERIAL_Print("\n\rCommand not recognized. Type \"help\" for a list of commands.\n\r");
  }
  clear_array(input);
  EL_SERIAL_Print("\n\r>>");
}

/*void display_sequence(struct Sequence* sequence){
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
}*/

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

  /*DEBUG
  set_default_data();
  load_and_play_seq(sequence[0].colours, 0 ,sequence[0].length, 10000);
  */

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

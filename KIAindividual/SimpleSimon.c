#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_rit.h"
#include "lib/empr_lib_utilities.c"
#include "lib/empr_lib_lcd.c"
#include "lib/empr_lib_keypad.c"
#include "lib/empr_lib_serial.c"
#include "GenericLibraries.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Predefine constants etc
#define LCD_ADDRESS 0x3B
#define EIGHT_SEG_ADDRESS 0x38
#define KEYPAD_ADDRESS 0x21
#define SPEAKER 0x80 //PIN7

const int SECOND = 1000000;
const int MILISECOND = 1000;
const int MICROSECOND = 1;

const int colours[4] = {1, 2, 3, 4};

//Function declarations
void init_SEGMENTS();
void SEGMENT_WriteHidden(int value, uint8_t dp_digit, int leading);
void SEGMENT_Write(int int_value, int zeros);
void SEGMENT_WriteFloat(double double_value, int zeros);
void speaker(int colour);
void setLedsWithChar(char led_vals);
void speaker_GPIO(int colour);
void game_loop();
void LCD_clear();

//Function definitions

void speaker_GPIO(int colour){
  //Function to emit a tone for a specified colour
  //4 is lowest in pitch, 1 is highest
  int i = 0;

  switch(colour){
    case 1:
    //Red
    for(i = 0; i < 600; i++){
      GPIO_SetValue(0, SPEAKER);
      Delay(MILISECOND*1);
      GPIO_ClearValue(0, SPEAKER);
      Delay(MILISECOND*1);
    }
    break;

    case 2:
    //Blue
    for(i = 0; i < 300; i++){
      GPIO_SetValue(0, SPEAKER);
      Delay(MILISECOND*2);
      GPIO_ClearValue(0, SPEAKER);
      Delay(MILISECOND*2);
    }
    break;

    case 3:
    //Green
    for(i = 0; i < 200; i++){
      GPIO_SetValue(0, SPEAKER);
      Delay(MILISECOND*3);
      GPIO_ClearValue(0, SPEAKER);
      Delay(MILISECOND*3);
    }
    break;

    case 4:
    //Red and Green
    for(i = 0; i < 150; i++){
      GPIO_SetValue(0, SPEAKER);
      Delay(MILISECOND*4);
      GPIO_ClearValue(0, SPEAKER);
      Delay(MILISECOND*4);
    }
    break;
  }
}

void game_loop(){
  //Main game loop

  //Turn off colour code for the light
  uint8_t zeroes[1][3] = {{0, 0, 0}};
  send_colours(zeroes, 1, 0);

  //Will make rand() more random
  int seed = time(NULL);
  srand(seed);

  //Config the system clock correctly
  SysTick_Config(SystemCoreClock/SECOND - 6);
  //1000000 == second
  //1000 == milisecond


  //Variable declarations
  uint8_t sent[64][3];  //To contain the sequence of sent colours in RGB form
  int sent_int[64];     //To contain the sequence of sent colours in int form
  int receive[64];      //To store the received colour sequence from the user in int form
  char cont = '\0';     //To store the keypad press
  char colour_choice = 0;   //To store the colour chosen by the user from the keypad
  int colour_choice_int = 0;    //To convert the char colour_choice to an int
  int choice = 0;       //To store the random colour that is to be sent
  int win_flag = 1;     //0 for nonsuccessful state, 1 for successful state; will only change if false
  int choice_flag = 1;  //Used to wait for valid input from the user. When 0, a valid choice has been made
  int i = 0;            //Loop counter
  int k = 0;            //Loop counter
  int counter = 0;      //Turn counter for use with segment display as extra feature

  sent[0][0] = 0;       //Initial set up of sent to prevent error
  sent[0][1] = 0;
  sent[0][2] = 0;

  //First LCD screen; holds till A is pressed
  cont = '\0';
  while (cont != 'A'){
    display_LCD("SIMPLE SIMON!   ", 0);
    display_LCD("PRESS A TO CONT.", 16);
    cont = EL_KEYPAD_ReadKey();
  }

  //Second LCD screen; holds till A is pressed
  cont = '\0';
  while (cont != 'A'){
    display_LCD("A sequence will ", 0);
    display_LCD("flash    PRESS A", 16);
    cont = EL_KEYPAD_ReadKey();
  }


  //Third LCD screen; holds till A pressed
  cont = '\0';
  while (cont != 'A'){
    display_LCD("Copy the seq.   ", 0);
    display_LCD("using keypad - A", 16);
    cont = EL_KEYPAD_ReadKey();
  }

  //Fourth LCD screen; holds till A pressed
  display_LCD("1 = Red  2 = Blu", 0);
  display_LCD("3 = Grn  4 = RG ", 16);
  Delay(5*SECOND);

  //END OF SET UP LCD INSTRUCTIONS
  cont = '\0';
  while (cont != 'A'){
    display_LCD("PRESS A TO START", 0);
    display_LCD("                ", 16);
    cont = EL_KEYPAD_ReadKey();
  }

  //GAME LOOP CONTAINED BELOW
  while(win_flag == 1){
    counter++;
    //Display current turn on 8Seg
    SEGMENT_Write(counter, 0);

    //Holds till the sequence has sent
    display_LCD("WATCH THE SEQ...", 0);
    display_LCD("                ", 16);

    //Generates a random number between 1 and 4, inclusive
    choice = (rand() % 4)+1;
    sent_int[counter-1] = choice;

    //Convert choice to DMX function standard
    switch(choice){
      case 1:
      //Red
      sent[counter-1][0] = 255;
      sent[counter-1][1] = 0;
      sent[counter-1][2] = 0;
      break;

      case 2:
      //Blue
      sent[counter-1][0] = 0;
      sent[counter-1][1] = 0;
      sent[counter-1][2] = 255;
      break;

      case 3:
      //Green
      sent[counter-1][0] = 0;
      sent[counter-1][1] = 255;
      sent[counter-1][2] = 0;
      break;

      case 4:
      //Red and Green
      sent[counter-1][0] = 255;
      sent[counter-1][1] = 255;
      sent[counter-1][2] = 0;
      break;
    }

    for(k = 0; k < counter; k++){
      //Loop through the sequence with counter # of colours
      send_colours(sent[k], 1, 0);
      //Play a tone at the same time
      speaker_GPIO(sent_int[k]);
      Delay(3*SECOND);
    }
    //Clear the light before the next sequence
    send_colours(zeroes, 1, 0);
    //Holds until they have finished inputting
    display_LCD("INPUT THE SEQ...", 0);
    display_LCD("                ", 16);

    int x = 0;
    for (x = 0; x < counter; x++){
      //Loops through until they have input the correct # of colours
      choice_flag = 1;

      while(choice_flag == 1){
        colour_choice = EL_KEYPAD_ReadKey();

        switch(colour_choice){
          //Switches keypad char input to int
          //User is prevented from entering illegal input
          case '1':
          //Red
          choice_flag = 0;
          colour_choice_int = 1;
          break;

          case '2':
          //Blue
          choice_flag = 0;
          colour_choice_int = 2;
          break;

          case '3':
          //Green
          choice_flag = 0;
          colour_choice_int = 3;
          break;

          case '4':
          //Red and Green
          choice_flag = 0;
          colour_choice_int = 4;
          break;

          default:
          //Illegal input
          choice_flag = 1;

        }
      }
      //Append the selected colour to the receive array to check later
      receive[x] = colour_choice_int;
    }


    for(i = 0; i < counter; i ++){
      //Loop through, comparing sent colours to received answers
      if(sent_int[i] == receive[i]){
        //If correct, continue
        win_flag = 1;
        continue;
      } else {
        //If wrong, break out of the win loop
        win_flag = 0;
        break;
      }
    }

    if(win_flag == 0){
      break;
    }
    //Win activity here?
  }

  //Holds till A pressed
  cont = '\0';
  while (cont != 'A'){
    display_LCD("You got to      ", 0);
    uint8_t str[16];
    sprintf(str, "%02d turns!     -A", counter);
    display_LCD(str, 16);
    cont = EL_KEYPAD_ReadKey();
  }

  //Holds till A pressed
  cont = '\0';
  while (cont != 'A'){
    display_LCD(" DO YOU WANT TO ", 0);
    display_LCD(" PLAY AGAIN? -A ", 16);
    cont = EL_KEYPAD_ReadKey();
  }

  //Holds till 1 or 2 pressed
  cont = '\0';
  while (cont != '1' && cont != '2'){
    display_LCD("     1 = YES    ", 0);
    display_LCD("     2 = NO     ", 16);
    cont = EL_KEYPAD_ReadKey();
  }

  switch(cont){
    //Choose to play again or "quit"
    case('1'):
    game_loop();
    break;

    case('2'):
    //Reset segment display, and display end screen
    SEGMENT_Write(0, 0);
    display_LCD("   THANKS FOR   ", 0);
    display_LCD("     PLAYING    ", 16);
    break;
  }
}


int main(){
  //Init from generator functions
  Full_Init();
  //Serial init for printing
  EL_SERIAL_Init();
  //Set up speaker pin
  GPIO_SetDir(0, SPEAKER, 1);
  //Enter the game
  game_loop();
  return 0;
}

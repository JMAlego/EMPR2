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

#define ALL_LEDS 0xB40000
#define LED1 0x040000
#define LED2 0x100000
#define LED3 0x200000
#define LED4 0x800000

#define LCD_ADDRESS 0x3B
#define EIGHT_SEG_ADDRESS 0x38
#define KEYPAD_ADDRESS 0x21
#define SPEAKER 0x80 //PIN7

//END OF PREDEFINED ---

const int SECOND = 1000000;
const int MILISECOND = 1000;
const int MICROSECOND = 1;

const int leds[4] = {LED1, LED2, LED3, LED4};
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

void setLedsWithChar(char led_vals){
  GPIO_ClearValue(1, ALL_LEDS);
  led_vals = led_vals & 0xf;
  if(led_vals & 0x1) GPIO_SetValue(1, LED1);
  if(led_vals & 0x2) GPIO_SetValue(1, LED2);
  if(led_vals & 0x4) GPIO_SetValue(1, LED3);
  if(led_vals & 0x8) GPIO_SetValue(1, LED4);
}

void speaker_GPIO(int colour){
  int i;

  switch(colour){
    case 1:
      for(i = 0; i < 600; i++){
        GPIO_SetValue(0, SPEAKER);
        Delay(MILISECOND*1);
        GPIO_ClearValue(0, SPEAKER);
        Delay(MILISECOND*1);
      }
      break;

    case 2:
      for(i = 0; i < 300; i++){
        GPIO_SetValue(0, SPEAKER);
        Delay(MILISECOND*2);
        GPIO_ClearValue(0, SPEAKER);
        Delay(MILISECOND*2);
      }
      break;

    case 3:
      for(i = 0; i < 200; i++){
        GPIO_SetValue(0, SPEAKER);
        Delay(MILISECOND*3);
        GPIO_ClearValue(0, SPEAKER);
        Delay(MILISECOND*3);
      }
      break;

    case 4:
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

  SysTick_Config(SystemCoreClock/SECOND - 6);
  //1000000 == second
  //1000 == milisecond

  GPIO_SetDir(0, SPEAKER, 1);
  GPIO_SetDir(1, ALL_LEDS, 1);

  uint8_t sent[64][3];
  int sent_int[64];
  int receive[64];
  char cont = '\0';
  char colour_choice = 0;
  int choice = 0;
  int rec_counter = 0;
  int win_flag = 1;     //0 for nonsuccessful state, 1 for successful state; will only change if false
  int rec_flag = 1;     //Used for checking when to stop receiving; Initially 1, turned off when the loop has filled all available receieve slots
  int choice_flag = 1;  //Used to wait for valid input from the user. When 0, a valid choice has been made
  int i = 0;
  int k = 0;
  int counter = 0;

  //uint8_t tester[3][3]= {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
  //send_colours(tester, 3, 100000);

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
  /////////////////////////////////////////////////////////////////////////////
  while(win_flag == 1){
    counter++;
    SEGMENT_Write(counter, 0);

    display_LCD("WATCH THE SEQ...", 0);
    display_LCD("                ", 16);

    //TESTED
    choice = (rand() % 4)+1;
    sent_int[counter] = choice;

    //Convert choice to DMX function standard

    switch(choice){
      case 1:
      sent[counter][0] = 255;
      sent[counter][1] = 0;
      sent[counter][2] = 0;
      break;

      case 2:
      sent[counter][0] = 0;
      sent[counter][1] = 0;
      sent[counter][2] = 255;
      break;

      case 3:
      sent[counter][0] = 0;
      sent[counter][1] = 255;
      sent[counter][2] = 0;
      break;

      case 4:
      sent[counter][0] = 255;
      sent[counter][1] = 255;
      sent[counter][2] = 0;
      break;
    }
  }


    for(k = 0; k < counter; k++){
      sprintf(to_print, "colour = %d\r\n", sent_int[k]);
      print(to_print);
      send_colours(sent[k], k, 0);
      speaker_GPIO(sent_int[k]);
      Delay(3*SECOND);
    }


    display_LCD("INPUT THE SEQ...", 0);
    display_LCD("                ", 16);

    while(rec_flag == 1){

      choice_flag = 1;

      //Wait until a valid key is entered; prevents illegal input
      //TESTED
      while(choice_flag == 1){

        colour_choice = EL_KEYPAD_ReadKey();

        switch(colour_choice){
          case '1':
            choice_flag = 0;
            break;

          case '2':
            choice_flag = 0;
            break;

          case '3':
            choice_flag = 0;
            break;

          case '4':
            choice_flag = 0;
            break;

          default:
            choice_flag = 1;
            break;

        }
      }



      //append valid input to the receive array
      receive[rec_counter] = colour_choice;

      //loop until we have the required number of received variables
      if(rec_counter == counter){
        rec_flag = 0;
      }
      rec_counter++;
    }

    for(i = 0; i < counter; i ++){
      if(sent_int[i] == receive[i]){
        win_flag = 1;
        rec_flag = 1;
        continue;
      } else {
        win_flag = 0;
        break;
      }
    }

    if(win_flag == 0){
      break;
    }
    for(i = 0; i < 4; i++){
      GPIO_SetValue(1, leds[4]);
      Delay(SECOND);
      GPIO_ClearValue(1, leds[4]);
    }

  }

  /////////////////////////////////////////////////////////////////////////////

  cont = '\0';
  while (cont != 'A'){
    display_LCD("You got to      ", 0);
    uint8_t str[16];
    sprintf(str, "%02d turns!     -A", counter);
    display_LCD(str, 16);
    cont = EL_KEYPAD_ReadKey();
  }

  cont = '\0';
  while (cont != 'A'){
    display_LCD(" DO YOU WANT TO ", 0);
    display_LCD(" PLAY AGAIN? -A ", 16);
    cont = EL_KEYPAD_ReadKey();
  }

  cont = '\0';
  while (cont != '1' && cont != '2'){
    display_LCD("     1 = YES    ", 0);
    display_LCD("     2 = NO     ", 16);
    cont = EL_KEYPAD_ReadKey();
  }

  switch(cont){
    case('1'):
      game_loop();
      break;
    case('2'):
      SEGMENT_Write(0, 0);
      display_LCD("   THANKS FOR   ", 0);
      display_LCD("     PLAYING    ", 16);

      break;
  }
}

/*void tetris_play(long duration, float freq){
  int i;
  long ontime = (long)(((float) SECOND)/freq);
  for(i = 0; i < (duration/ontime); i++){
    GPIO_SetValue(0, SPEAKER);
    Delay(ontime/2);
    GPIO_ClearValue(0, SPEAKER);
    Delay(ontime/2);
  }
}

void tetris(){
  int theme[111][2] = {{658, 125},{1320, 500},{990, 250},{1056, 250},{1188, 250},{1320, 125},{1188, 125},{1056, 250},{990, 250},{880, 500},{880, 250},{1056, 250},{1320, 500},{1188, 250},{1056, 250},{990, 750},{1056, 250},{1188, 500},{1320, 500},{1056, 500},{880, 500},{880, 500},{1188, 500},{1408, 250},{1760, 500},{1584, 250},{1408, 250},{1320, 750},{1056, 250},{1320, 500},{1188, 250},{1056, 250},{990, 500},{990, 250},{1056, 250},{1188, 500},{1320, 500},{1056, 500},{880, 500},{880, 500},{1320, 500},{990, 250},{1056, 250},{1188, 250},{1320, 125},{1188, 125},{1056, 250},{990, 250},{880, 500},{880, 250},{1056, 250},{1320, 500},{1188, 250},{1056, 250},{990, 750},{1056, 250},{1188, 500},{1320, 500},{1056, 500},{880, 500},{880, 500},{1188, 500},{1408, 250},{1760, 500},{1584, 250},{1408, 250},{1320, 750},{1056, 250},{1320, 500},{1188, 250},{1056, 250},{990, 500},{990, 250},{1056, 250},{1188, 500},{1320, 500},{1056, 500},{880, 500},{880, 500},{660, 1000},{528, 1000},{594, 1000},{495, 1000},{528, 1000},{440, 1000},{419, 1000},{495, 1000},{660, 1000},{528, 1000},{594, 1000},{495, 1000},{528, 500},{660, 500},{880, 1000},{838, 2000},{660, 1000},{528, 1000},{594, 1000},{495, 1000},{528, 1000},{440, 1000},{419, 1000},{495, 1000},{660, 1000},{528, 1000},{594, 1000},{495, 1000},{528, 500},{660, 500},{880, 1000},{838, 2000}};
  int i = 0;
  while(i < 111){
    tetris_play(MILISECOND * theme[i][1], theme[i][0]/3);
    i++;
  }
}

*/

int main(){
  Full_Init();
  EL_SERIAL_Init();
  GPIO_SetDir(0, SPEAKER, 1);
  //game_loop();

  int seed = time(NULL);
  srand(seed);

  uint8_t sent[64][3];
  //uint8_t dmx[1][3];
  int sent_int[64];
  //int receive[64];
  //char cont = '\0';
  //char colour_choice = 0;
  int choice = 0;
  //int rec_counter = 0;
  //int win_flag = 1;
  //int rec_flag = 1;
  //int choice_flag = 1;
  int i = 0;
  int k = 0;
  int counter = 0;
  char to_print[64];

  for(i = 0; i < 5; i++){
    counter++;
    SEGMENT_Write(counter, 0);



    //TESTED
    choice = (rand() % 4)+1;
    sent_int[counter] = choice;

    //Convert choice to DMX function standard

    switch(choice){
      case 1:
      sent[counter][0] = 255;
      sent[counter][1] = 0;
      sent[counter][2] = 0;
      break;

      case 2:
      sent[counter][0] = 0;
      sent[counter][1] = 0;
      sent[counter][2] = 255;
      break;

      case 3:
      sent[counter][0] = 0;
      sent[counter][1] = 255;
      sent[counter][2] = 0;
      break;

      case 4:
      sent[counter][0] = 255;
      sent[counter][1] = 255;
      sent[counter][2] = 0;
      break;
    }
  }


  //TESTED
  //Send the whole current sequence
  //send_colours(sent, counter, 3*SECOND);
  //Speaker timings lined up to coincide



  for(k = 0; k < counter; k++){
    sprintf(to_print, "colour = %d %d %d\r\n", sent[k][0], sent[k][1], sent[k][2]);
    print(to_print);
    send_colours(sent[k], k, 0);

    switch(sent_int[k]){
      case 1:
      sprintf(to_print, "red");
      print(to_print);
      break;

      case 2:
      sprintf(to_print, "blue");
      print(to_print);
      break;

      case 3:
      sprintf(to_print, "green");
      print(to_print);
      break;

      case 4:
      sprintf(to_print, "r&g");
      print(to_print);
      break;
    }
    Delay(3*SECOND);
  }



  //speaker_GPIO(sent_int[k]);


  uint8_t zeroes[1][3] = {{0, 0, 0}};
  send_colours(zeroes, 1, 0);


  sprintf(to_print, "----------------------------------------");
  print(to_print);



  /*
  GPIO_SetDir(0, SPEAKER, 1);
  speaker_GPIO(1);  //red       highest
  speaker_GPIO(2);  //blue
  speaker_GPIO(3);  //green
  speaker_GPIO(4);  //RG        lowest  */

  return 0;
}

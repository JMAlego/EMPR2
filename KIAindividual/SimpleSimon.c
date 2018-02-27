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
int rand_colour = 0;

volatile unsigned int PWMCounter = 0;
volatile int8_t PWMDirection = 1;
volatile uint8_t RIT_Mode = 0;
volatile uint8_t sine_mode = 0;


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

  char lcd_string[32];
  char turn_string[32];
  uint8_t sent[64][3];
  uint8_t dmx[1][3];
  int sent_int[64];
  int receive[64];
  char cont = '\0';
  int colour_choice = 0;
  int choice = 0;
  int rec_counter = 0;
  int win_flag = 1;     //0 for nonsuccessful state, 1 for successful state; will only change if false
  int rec_flag = 1;   //Used for checking when to stop receiving; Initially 1, turned off when the loop has filled all available receieve slots
  int i = 0;
  int k = 0;
  int counter = 0;

  Full_Init();

  //uint8_t tester[3][3]= {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
  //send_colours(tester, 3, 100000);

  //First LCD screen; holds till A is pressed
  cont = '\0';
  while (cont != 'A'){
    display_LCD("SIMPLE SIMON!   PRESS A TO CONT.", 0);


    strcpy(lcd_string, "SIMPLE SIMON!");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 13);
    strcpy(lcd_string, "PRESS A TO CONT.");
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteAddress(0x40);
    EL_LCD_WriteChars(lcd_string, 16);
    cont = EL_KEYPAD_ReadKey();
  }

  //Second LCD screen; holds till A is pressed

  cont = '\0';
  while (cont != 'A'){
    strcpy(lcd_string, "A sequence will");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 15);
    strcpy(lcd_string, "flash    PRESS A");
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteAddress(0x40);
    EL_LCD_WriteChars(lcd_string, 16);
    cont = EL_KEYPAD_ReadKey();
  }


  //Third LCD screen; holds till A pressed

  cont = '\0';
  while (cont != 'A'){
    strcpy(lcd_string, "Copy the seq.");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 13);
    strcpy(lcd_string, "using keypad- A");
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteAddress(0x40);
    EL_LCD_WriteChars(lcd_string, 15);
    cont = EL_KEYPAD_ReadKey();
  }

  //Fourth LCD screen; holds till A pressed
  strcpy(lcd_string, "1 = Red  2 = Blu");
  LCD_clear();
  EL_LCD_EncodeASCIIString(lcd_string);
  EL_LCD_WriteChars(lcd_string, 15);
  strcpy(lcd_string, "3 = Grn  4 = Ylw");
  EL_LCD_EncodeASCIIString(lcd_string);
  EL_LCD_WriteAddress(0x40);
  EL_LCD_WriteChars(lcd_string, 16);
  Delay(8*SECOND);
  //END OF SET UP LCD INSTRUCTIONS

  cont = '\0';
  while (cont != 'A'){
    strcpy(lcd_string, "PRESS A TO START");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 16);
    strcpy(lcd_string, "                ");
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteAddress(0x40);
    EL_LCD_WriteChars(lcd_string, 16);
    cont = EL_KEYPAD_ReadKey();
  }

  //GAME LOOP CONTAINED BELOW
  while(win_flag == 1){

    counter++;
    SEGMENT_Write(counter, 0);

    strcpy(lcd_string, "WATCH THE SEQ...");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 16);


    choice = rand() % 4;
    //choice = (rand() % 4) + 1;
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

    //Reassign a single colour to a DMX array in order to produce a tone at the same time
    for(k = 0; k < counter; k++){
      sent[i][0] = dmx[i][0];
      sent[i][1] = dmx[i][1];
      sent[i][2] = dmx[i][2];
      send_colours(dmx, 1, 2*1000000);   //Had to write out 2 second delay in order to fit GENERATOR functions
      speaker_GPIO(choice);
    }

    strcpy(lcd_string, "INPUT SEQ.......");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 16);

    while(rec_flag == 1){

      colour_choice = 0;
      //Wait until a valid key is entered; prevents illegal input
      while(colour_choice != '1' && colour_choice != '2' && colour_choice != '3' && colour_choice != '4'){
        colour_choice = EL_KEYPAD_ReadKey();
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

  cont = '\0';
  while (cont != 'A'){
    strcpy(lcd_string, "You got to      ");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 16);
    sprintf(turn_string, "%02d turns!     -A", counter);
    strcpy(lcd_string, turn_string);
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteAddress(0x40);
    EL_LCD_WriteChars(lcd_string, 16);
    cont = EL_KEYPAD_ReadKey();
  }

  cont = '\0';
  while (cont != 'A'){
    strcpy(lcd_string, " DO YOU WANT TO ");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 16);
    strcpy(lcd_string, " PLAY AGAIN? -A ");
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteAddress(0x40);
    EL_LCD_WriteChars(lcd_string, 16);
    cont = EL_KEYPAD_ReadKey();
  }

  cont = '\0';
  while (cont != '1' && cont != '2'){
    strcpy(lcd_string, "     1 = YES    ");
    LCD_clear();
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteChars(lcd_string, 16);
    strcpy(lcd_string, "     2 = NO     ");
    EL_LCD_EncodeASCIIString(lcd_string);
    EL_LCD_WriteAddress(0x40);
    EL_LCD_WriteChars(lcd_string, 16);
    cont = EL_KEYPAD_ReadKey();
  }

  switch(cont){
    case('1'):
      game_loop();
      break;
    case('2'):
      SEGMENT_Write(0, 0);
      strcpy(lcd_string, "   THANKS FOR   ");
      LCD_clear();
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 16);
      strcpy(lcd_string, "    PLAYING!    ");
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteAddress(0x40);
      EL_LCD_WriteChars(lcd_string, 16);
      break;
  }
}

void tetris_play(long duration, float freq){
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

int main(){
  //UART_Init2();
  //tetris();
  //print("TESTING\r\n");
  game_loop();

  return 0;
}

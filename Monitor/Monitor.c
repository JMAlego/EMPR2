#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lib/empr_lib_utilities.c"
#include "lib/empr_lib_lcd.c"
#include "lib/empr_lib_keypad.c"
#include <stdio.h>
#include <string.h>
#define READ() ((GPIO_ReadValue(0) & 0x20000) == 0x20000)

#define SENDING 0x40040
#define RECEIVING 0x20000
#define STOP 0x000000

//Special serial for higher throughput than normally in library
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
  //High baud rate for faster PC interface
  UARTCfg.Baud_rate = 230400;
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

//Print alias for easier
void print(char * string)
{
  UART_Send(LPC_UART0, (uint8_t *) string, EL_SERIAL_SizeOfString((uint8_t *) string), BLOCKING);
}

//Handy consts for times that make human sense
const int SECOND = 1000000;
const int MILISECOND = 1000;
const int MICROSECOND = 1;

//States for core state machine
typedef enum MONITOR_STATE{
  IDLE=0,
  BREAK=1,
  INITIAL_MARK_AFTER_BREAK=2,
  INITIAL_SLOT=3,
  SLOT=4,
  END_FRAME=5
} MONITOR_STATE;

//Function to get a frame/packet
int getFrame(uint8_t * out_type, uint8_t out_slots[]){
  //Initialise variables
  int input_state;
  unsigned long start_time;
  uint8_t slots[513];
  MONITOR_STATE state = IDLE;
  uint8_t slot = 0;
  uint8_t break_bit = 0;
  uint8_t end_bits = 0;
  int bit_index = 0;
  unsigned long bit_start_time = 0;
  int slot_count = 0;
  unsigned long slot_start_time = 0;
  int errors = 0;
  while(1){//Start main state machine loop
    //Reset variables
    slot = 0;
    break_bit = 0;
    end_bits = 0;
    bit_index = 0;
    bit_start_time = 0;
    slot_count = 0;
    slot_start_time = 0;
    state = IDLE;
    //IDLE STATE - Wait for DMX data to start
    start_time = SysTickCnt;
    while(state == IDLE){
      input_state = READ();
      if (input_state == 0){
        if(SysTickCnt - start_time > MILISECOND)
          break;
        else
          start_time = SysTickCnt;
      }
    }
    //BREAK STATE - Got idle, now we need to check for the break
    state = BREAK;
    start_time = SysTickCnt;
    while(state == BREAK){
      input_state = READ();
      if (input_state == 1){
        if(SysTickCnt - start_time > MILISECOND)
          break;
        else if(SysTickCnt - start_time > SECOND){
          //Something has gone really wrong PANIC!
          state = IDLE;
        }else{
          start_time = SysTickCnt;
          state = IDLE;
        }
      }
    }
    if(state == IDLE) //Didn't get a break, so go to start
      continue;
    //SLOT STATE
    state = SLOT;
    start_time = SysTickCnt;
    while(state == INITIAL_SLOT || state == SLOT){ //Read slots
      //Whille we've got more slots to read
      while(slot_count < 513){
        //Set slot variables
        slot = 0;
        bit_index = 0;
        end_bits = 0;
        while(READ() == 1);//Wait for a low
        slot_start_time = SysTickCnt;
        while(bit_index < 11){//Read the 11 bits of the slot
          bit_start_time = SysTickCnt;
          if(bit_index == 0){
              break_bit = READ();
          }else if (bit_index == 9 || bit_index == 10){
              end_bits += READ();
          }else{
            slot |= READ() << (bit_index - 1);
          }
          bit_index++;
          if(bit_index < 11){//Wait for end of bit
            while(SysTickCnt - bit_start_time < MICROSECOND*4);
          }
        }
        if(break_bit != 0 || end_bits != 2){//If there's an error, count it
          errors++;
        }
        //Increment slot
        slots[slot_count] = slot;
        slot_count++;
        //wait for end of slot
        while(SysTickCnt - slot_start_time < MICROSECOND*4*11 - 1*MICROSECOND);
      }
      //If we've got all the data, exit reading state
      if(slot_count == 513) state = END_FRAME;
    }
    if(state == END_FRAME)//Exit loop on successful read
      break;
    if(state == IDLE)//Else return to start of state machine
      continue;
  }
  //Copy data to output
  int i = 1;
  while(i < 513){
    out_slots[i-1] = slots[i];
    i++;
  }
  *out_type = slots[0];
  return errors;
}

typedef enum MENU_STATE{
  MENU_TOP,
  CAPTURED_PACKET,
  TRIGGER_INPUT_1,
  TRIGGER_INPUT_2,
  TRIGGER_INPUT_CAPTURE,
  TRIGGER_INPUT_LOOP,
  DISPLAY_MODE
} MENU_STATE;

void type_lookup(char * string, uint8_t type_slot){
  switch (type_slot) {
    case 0x00:
      strcpy(string, "Lighting Packet ");
      break;
    case 0x17:
      strcpy(string, "Text Packet     ");
      break;
    case 0xcf:
      strcpy(string, "Sys Info Packet ");
      break;
    case 0xcc:
      strcpy(string, "RDM Ext. for DMX");
      break;
    default:
      strcpy(string, "Unknown         ");
  }
}

void sweep_display(char * startup_message){
  uint8_t startup_message_length = strlen(startup_message);
  EL_LCD_EncodeASCIIString(startup_message);
  unsigned int i;
  uint8_t offset = 5;
  uint8_t pointer1 = 0;
  uint8_t pointer2 = 0;
  for(i = 0; i < 32 + offset; i++){
    if(i < 16)
      pointer1 = i;
    else
      pointer1 = 48 + i;
    if(i - offset < 16)
      pointer2 = i - offset;
    else
      pointer2 = 48 + i - offset;
    if (i >= offset){
      EL_LCD_WriteAddress(pointer2);
      EL_LCD_WriteChar(i - offset < startup_message_length ? startup_message[i-offset] : 0b10100000);
    }
    if (i < 32){
      EL_LCD_WriteAddress(pointer1);
      EL_LCD_WriteChar(0b00010011);
    }
    Delay(MILISECOND*50);
  }
}

/*
  Function to display startup message
*/
void startup(){
  char startup_message[33] = "EMPR Monitor    Group 4";
  sweep_display(startup_message);
  strcpy(startup_message, "By Jacob, Kia,  Max, Calum");
  sweep_display(startup_message);
  strcpy(startup_message, "");
  sweep_display(startup_message);
  Delay(MILISECOND*10);
}

int main(){
  //Configure timer
  SysTick_Config(SystemCoreClock/SECOND - 6);
  //1000000 == second
  //1000 == milisecond
  //Initialise I2C, Serial, and LCD
  EL_I2C_Init();
  EL_SERIAL_Init();
  EL_LCD_Init();
  EL_LCD_ClearDisplay();
  //Setup GPIO
  GPIO_SetDir(0, SENDING, 1);
  GPIO_SetDir(0, RECEIVING, 0);
  //Display startup message
  startup();
  //Send start on UART
  print("START\r\n");
  //Setup variables
  uint8_t slots[512];
  uint8_t trigger_compare[512];
  uint8_t type_slot;
  uint8_t trigger_changed = 0;
  int slot_offset = 0;
  char lcd_string[32];
  int errors = 0;
  MENU_STATE current_menu_state = MENU_TOP;
  char menu_key;
  int channel_size = 0;
  int channel_address = 0;
  //Main menu state machine loop
  while(1){
    while(current_menu_state == MENU_TOP){ //Top level menu
      //Display menu
      strcpy(lcd_string, "MENU:  2=MANUAL ");
      EL_LCD_ClearDisplay();
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 16);
      strcpy(lcd_string, "5=DISP 8=TRIGGER");
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteAddress(0x40);
      EL_LCD_WriteChars(lcd_string, 16);
      //Wait for input
      menu_key = EL_KEYPAD_ReadKey();
      switch (menu_key){//Do menu actions
        case '2': //Single Capture
          strcpy(lcd_string, "CAPTURING");
          EL_LCD_ClearDisplay();
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 9);
          errors = getFrame(&type_slot, slots);
          current_menu_state = CAPTURED_PACKET;
          slot_offset = 0;
          print("READ\r\n");
          break;
        case '8': //Trigger Mode
          current_menu_state = TRIGGER_INPUT_1;
          break;
        case '5': //PC Display Mode
          current_menu_state = DISPLAY_MODE;
          strcpy(lcd_string, "DISPLAY MODE    ");
          EL_LCD_WriteAddress(0x00);
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);
          strcpy(lcd_string, "HOLD D TO EXIT  ");
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteAddress(0x40);
          EL_LCD_WriteChars(lcd_string, 16);
          break;
      }
    }
    //Packet captured state
    while(current_menu_state == CAPTURED_PACKET){
      print("STATE = CAPTURED_PACKET\r\n");
      if (slot_offset == -1){// Display data type
        EL_LCD_WriteAddress(0x00);
        sprintf(lcd_string, "Type Slot: 0x%02x ", type_slot);
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 16);
        EL_LCD_WriteAddress(0x40);
        type_lookup(lcd_string, type_slot);
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 16);
      }else{// Display normal slots
        EL_LCD_WriteAddress(0x00);
        sprintf(lcd_string, "0x%02x:0x%02x    %03d", slots[slot_offset*4], slots[slot_offset*4+1], slot_offset*4 + 1);
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 16);
        EL_LCD_WriteAddress(0x40);
        sprintf(lcd_string, "0x%02x:0x%02x HELP=A ", slots[slot_offset*4+2], slots[slot_offset*4+3]);
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 16);
      }
      //Get input
      menu_key = EL_KEYPAD_ReadKey();
      switch (menu_key) {//Do menu action
        case '#': //Next
          slot_offset++;
          if(slot_offset == 128){
            slot_offset = -1;
          }
          break;
        case '*': //Previous
          slot_offset--;
          if(slot_offset == -2){
            slot_offset = 127;
          }
          break;
        case '0':
          errors = getFrame(&type_slot, slots);
          break;
        case 'D': //Exit
          current_menu_state = MENU_TOP;
          break;
        case 'A': //Help
          EL_LCD_WriteAddress(0x00);
          strcpy(lcd_string, "NEXT=# PREV=*   ");
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);
          EL_LCD_WriteAddress(0x40);
          strcpy(lcd_string, "CAPTURE=0 D=BACK");
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);

          EL_KEYPAD_ReadKey();
          break;
      }
    }
    //Get the user's input channel size
    while(current_menu_state == TRIGGER_INPUT_1){
      //Prompt the user for input
      EL_LCD_WriteAddress(0x00);
      strcpy(lcd_string, "Ch. Size A=ENTER");
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 16);
      strcpy(lcd_string, "000 Default: 1  ");
      EL_LCD_WriteAddress(0x40);
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 16);
      EL_LCD_WriteAddress(0x40);
      char input_key = '\0';
      char input_chars[3] = {0, 0, 0};
      unsigned char key_index = 0;
      //While we're still getting input
      while(current_menu_state == TRIGGER_INPUT_1){
        input_key = EL_KEYPAD_ReadKey();
        //If the user pressed "enter"
        if(input_key == 'A'){
          channel_size = input_chars[2] + input_chars[1]*10 + input_chars[0]*100;
          //If it's not valid, set it to 0
          if(channel_size>512){
            channel_size = 0;
          }else{ //Else use it
            if(channel_size == 0)
              channel_size = 1;
            current_menu_state = TRIGGER_INPUT_2;
          }
        }else{
          //Decode as a number and move to next input char
          input_chars[key_index] = EL_UTIL_ASCIINumberCharacterToNumber(input_key);
          if(input_chars[key_index]!=10){
            EL_LCD_WriteAddress(0x40+key_index);
            EL_LCD_WriteChar(EL_LCD_EncodeASCII(input_key));
            key_index=(key_index + 1) % 3;
          }
        }
      }
    }
    //Get the user's input address
    while(current_menu_state == TRIGGER_INPUT_2){
      //Prompt the user for input
      EL_LCD_WriteAddress(0x00);
      strcpy(lcd_string, "Address A=ENTER ");
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 16);
      strcpy(lcd_string, "000 Default: 1  ");
      EL_LCD_WriteAddress(0x40);
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 16);
      EL_LCD_WriteAddress(0x40);
      char input_key = '\0';
      char input_chars[3] = {0, 0, 0};
      unsigned char key_index = 0;
      //While we're still getting input
      while(current_menu_state == TRIGGER_INPUT_2){
        input_key = EL_KEYPAD_ReadKey();
        //If the user pressed "enter"
        if(input_key == 'A'){
          channel_address = input_chars[2] + input_chars[1]*10 + input_chars[0]*100;
          //If it's not valid, set it to 0
          if(channel_address>512){
            channel_address = 0;
          }else{ //Else use it
            if(channel_address == 0)
              channel_address = 1;
            current_menu_state = TRIGGER_INPUT_CAPTURE;
          }
        }else{// If it's another char
          //Decode as a number and move to next input char
          input_chars[key_index] = EL_UTIL_ASCIINumberCharacterToNumber(input_key);
          if(input_chars[key_index]!=10){
            EL_LCD_WriteAddress(0x40+key_index);
            EL_LCD_WriteChar(EL_LCD_EncodeASCII(input_key));
            key_index=(key_index + 1) % 3;
          }
        }
      }
    }
    //If we're about to go into the Trigger Capture state
    if(current_menu_state == TRIGGER_INPUT_CAPTURE){
      //Prompt user that we're waiting for input
      EL_LCD_WriteAddress(0x00);
      strcpy(lcd_string, "WAITING FOR     ");
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 16);
      EL_LCD_WriteAddress(0x40);
      strcpy(lcd_string, "TRIGGER COND.   ");
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 16);
      //Read initial state
      errors = getFrame(&type_slot, slots);
      current_menu_state = CAPTURED_PACKET;
      //Set displayed slot to the start of specified channel
      slot_offset = ((channel_address-1)/4);
      //Clamp input
      if(slot_offset>127){
        slot_offset = 127;
      }else if(slot_offset < 0){
        slot_offset = 0;
      }
      current_menu_state = TRIGGER_INPUT_LOOP;
      trigger_changed = 1;
    }
    //Trigger Capture state
    while(current_menu_state == TRIGGER_INPUT_LOOP){
      //If we've detected a change
      if(trigger_changed){
        //Update the display
        if (slot_offset == -1){
          //Display type slot
          EL_LCD_WriteAddress(0x00);
          sprintf(lcd_string, "Type Slot: 0x%02x ", type_slot);
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);
          EL_LCD_WriteAddress(0x40);
          type_lookup(lcd_string, type_slot);
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);
        }else{
          //Display normal slots
          EL_LCD_WriteAddress(0x00);
          sprintf(lcd_string, "0x%02x:0x%02x    %03d", slots[slot_offset*4], slots[slot_offset*4+1], slot_offset*4 + 1);
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);
          EL_LCD_WriteAddress(0x40);
          sprintf(lcd_string, "0x%02x:0x%02x MENU=A ", slots[slot_offset*4+2], slots[slot_offset*4+3]);
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);
        }
      }
      //Check for user input
      menu_key = EL_KEYPAD_CheckKey();
      //Decode what input char means
      switch (menu_key) {
        case '#': //Move right
          slot_offset++;
          if(slot_offset == 128){
            slot_offset = -1;
          }
          trigger_changed = 1;
          break;
        case '*': //Move left
          slot_offset--;
          if(slot_offset == -2){
            slot_offset = 127;
          }
          trigger_changed = 1;
          break;
        case '0': //Force read
          errors = getFrame(&type_slot, slots);
          trigger_changed = 1;
          break;
        case 'D': //Exit
          current_menu_state = MENU_TOP;
          break;
        case 'A': //Help
          EL_LCD_WriteAddress(0x00);
          strcpy(lcd_string, "NEXT=# PREV=*   ");
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);
          EL_LCD_WriteAddress(0x40);
          strcpy(lcd_string, "CAPTURE=0 D=BACK");
          EL_LCD_EncodeASCIIString(lcd_string);
          EL_LCD_WriteChars(lcd_string, 16);

          EL_KEYPAD_ReadKey();
          break;
      }
      //Read DMX
      errors = getFrame(&type_slot, trigger_compare);
      //Compare all slots
      int compare_index = (channel_address-1) % 512;
      while(compare_index < channel_address+channel_size-1){
        //If there's a change
        if(trigger_compare[compare_index] != slots[compare_index]){
          //Copy data
          int copy_index = 0;
          while(copy_index < 512){
            slots[copy_index] = trigger_compare[copy_index];
            copy_index++;
          }
          //Mark changed
          trigger_changed = 1;
          break;
        }
        compare_index = (compare_index + 1) % 512;
      }
    }
    //Display Mode State
    //Uses run length encoding: https://en.wikipedia.org/wiki/Run-length_encoding
    while(current_menu_state == DISPLAY_MODE){
      //Captures a frame
      errors = getFrame(&type_slot, slots);
      //Creates a buffer for output to the PC
      uint8_t output_buffer[1028];
      int output_index = 0;
      //Indicate start of data
      output_buffer[0] = '!';
      uint8_t prev = 0;
      uint8_t prev_count = 0;
      unsigned long long output_buffer_index = 1;
      //For each output slot
      while(output_index < 512){
        //If the slot == the last slot and we haven't had >= 255 of the last slot
        if(prev == slots[output_index] && prev_count < 255){
          //Run is longer
          prev_count++;
        }else{
          //If we had a run, add that to the buffer
          if(prev_count > 0){
            output_buffer[output_buffer_index] = 'X';
            output_buffer_index++;
            output_buffer[output_buffer_index] = EL_UTIL_IntToHex(prev_count/16);
            output_buffer_index++;
            output_buffer[output_buffer_index] = EL_UTIL_IntToHex(prev_count%16);
            output_buffer_index++;
            prev_count = 0;
          }
          //Write encoded slot to buffer
          output_buffer[output_buffer_index] = EL_UTIL_IntToHex(slots[output_index]/16);
          output_buffer_index++;
          output_buffer[output_buffer_index] = EL_UTIL_IntToHex(slots[output_index]%16);
          output_buffer_index++;
          prev = slots[output_index];
        }
        output_index++;
      }
      //Catch runs that haven't been written yet
      if(prev_count > 0){
        output_buffer[output_buffer_index] = 'X';
        output_buffer_index++;
        output_buffer[output_buffer_index] = EL_UTIL_IntToHex(prev_count/16);
        output_buffer_index++;
        output_buffer[output_buffer_index] = EL_UTIL_IntToHex(prev_count%16);
        output_buffer_index++;
        prev_count = 0;
      }
      //Indicate end of data
      output_buffer[output_buffer_index] = '\r';
      output_buffer_index++;
      output_buffer[output_buffer_index] = '\n';
      output_buffer_index++;
      output_buffer[output_buffer_index] = '\0';
      //Send buffer to PC
      print((char *) output_buffer);
      menu_key = EL_KEYPAD_CheckKey();
      if(menu_key == 'D'){
        current_menu_state = MENU_TOP;
      }
    }
  }
  print("END\r\n");
  return 0;
}

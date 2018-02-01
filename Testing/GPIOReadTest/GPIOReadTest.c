#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lib/empr_lib_utilities.c"
#include "lib/empr_lib_lcd.c"
#include "lib/empr_lib_keypad.c"
#include <stdio.h>
#define READ() ((GPIO_ReadValue(0) & 0x20000) == 0x20000)

#define SENDING 0x40040
#define RECEIVING 0x20000
#define STOP 0x000000

/*
volatile unsigned long SysTickCnt;

void SysTick_Handler (void);
void Delay (unsigned long tick);


void SysTick_Handler (void) {
  SysTickCnt++;
}

void Delay (unsigned long tick) {
  uint64_t systickcnt;
  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}*/

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
  UARTCfg.Baud_rate = 256000;
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

void print(char string[])
{
  UART_Send(LPC_UART0, (uint8_t *) string, EL_SERIAL_SizeOfString((uint8_t *) string), BLOCKING);
}

const int SECOND = 1000000;
const int MILISECOND = 1000;
const int MICROSECOND = 1;

typedef enum MONITOR_STATE{
  IDLE=0,
  BREAK=1,
  INITIAL_MARK_AFTER_BREAK=2,
  INITIAL_SLOT=3,
  SLOT=4,
  END_FRAME=5
} MONITOR_STATE;

int getFrame(uint8_t * out_type, uint8_t out_slots[]){
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
  while(1){
    slot = 0;
    break_bit = 0;
    end_bits = 0;
    bit_index = 0;
    bit_start_time = 0;
    slot_count = 0;
    slot_start_time = 0;
    state = IDLE;
    //IDLE STATE
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
    //BREAK STATE
    state = BREAK;
    start_time = SysTickCnt;
    while(state == BREAK){
      input_state = READ();
      if (input_state == 1){
        if(SysTickCnt - start_time > MILISECOND)
          break;
        else if(SysTickCnt - start_time > SECOND){
          print("AAHAHAHAHAHAHH PANIC!\r\n");
          state = IDLE;
        }else{
          start_time = SysTickCnt;
          state = IDLE;
        }
      }
    }
    if(state == IDLE)
      continue;
    //SLOT STATE
    state = SLOT;
    start_time = SysTickCnt;
    while(state == INITIAL_SLOT || state == SLOT){
      while(slot_count < 513){
        slot = 0;
        bit_index = 0;
        end_bits = 0;
        while(READ() == 1);
        slot_start_time = SysTickCnt;
        /*if(slot_count == 0){
          while(SysTickCnt - slot_start_time < MICROSECOND*1);
        }*/
        while(bit_index < 11){
          bit_start_time = SysTickCnt;
          if(bit_index == 0){
              break_bit = READ();
          }else if (bit_index == 9 || bit_index == 10){
              end_bits += READ();
          }else{
            slot |= READ() << (bit_index - 1);
          }
          bit_index++;
          if(bit_index < 11){
            while(SysTickCnt - bit_start_time < MICROSECOND*4);
          }
        }
        if(break_bit != 0 || end_bits != 2){
          errors++;
        }
        /*if(state == INITIAL_SLOT){
          state = SLOT;
          type_slot = slot;
        }else{
          slots[slot_count - 1] = slot;
        }*/
        slots[slot_count] = slot;
        slot_count++;
        while(SysTickCnt - slot_start_time < MICROSECOND*4*11 - 1*MICROSECOND);
      }
      if(slot_count == 513) state = END_FRAME;
    }
    if(state == END_FRAME)
      break;
    if(state == IDLE)
      continue;
  }
  int i = 1;
  while(i < 513){
    out_slots[i-1] = slots[i];
    i++;
  }
  *out_type = slots[0];
  return errors;
}

typedef enum MENU_STATE{
  MENU_TOP=0,
  CAPTURED_PACKET=1
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

int main(){
  SysTick_Config(SystemCoreClock/SECOND - 6);
  //1000000 == second
  //1000 == milisecond
  EL_I2C_Init();
  EL_SERIAL_Init();
  EL_LCD_Init();
  EL_LCD_ClearDisplay();
  GPIO_SetDir(0, SENDING, 1);
  GPIO_SetDir(0, RECEIVING, 0);
  print("START\r\n");
  uint8_t slots[512];
  uint8_t type_slot;
  int slot_offset = 0;
  char lcd_string[32];
  int errors = 0;
  MENU_STATE current_menu_state = MENU_TOP;
  char menu_key;
  while(1){
    while(current_menu_state == MENU_TOP){
      strcpy(lcd_string, "MENU");
      EL_LCD_ClearDisplay();
      EL_LCD_EncodeASCIIString(lcd_string);
      EL_LCD_WriteChars(lcd_string, 4);
      menu_key = EL_KEYPAD_ReadKey();
      if(menu_key == '0'){
        strcpy(lcd_string, "CAPTURING");
        EL_LCD_ClearDisplay();
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 9);
        errors = getFrame(&type_slot, &slots);
        current_menu_state = CAPTURED_PACKET;
        slot_offset = 0;
        print("READ\r\n");
      }
    }
    while(current_menu_state == CAPTURED_PACKET){
      print("STATE = CAPTURED_PACKET\r\n");
      if (slot_offset == -1){
        EL_LCD_WriteAddress(0x00);
        sprintf(lcd_string, "Type Slot: 0x%02x ", type_slot);
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 16);
        EL_LCD_WriteAddress(0x40);
        type_lookup(&lcd_string, type_slot);
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 16);
      }else{
        EL_LCD_WriteAddress(0x00);
        sprintf(lcd_string, "0x%02x:0x%02x    %03d", slots[slot_offset*4], slots[slot_offset*4+1], slot_offset*4 + 1);
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 16);
        EL_LCD_WriteAddress(0x40);
        sprintf(lcd_string, "0x%02x:0x%02x       ", slots[slot_offset*4+2], slots[slot_offset*4+3]);
        EL_LCD_EncodeASCIIString(lcd_string);
        EL_LCD_WriteChars(lcd_string, 16);
      }
      menu_key = EL_KEYPAD_ReadKey();
      switch (menu_key) {
        case '#':
          slot_offset++;
          if(slot_offset == 128){
            slot_offset = -1;
          }
          break;
        case '*':
          slot_offset--;
          if(slot_offset == -2){
            slot_offset = 127;
          }
          break;
        case '0':
          errors = getFrame(&type_slot, &slots);
          break;
        case 'D':
          current_menu_state = MENU_TOP;
          break;
      }
    }
  }
  print("END\r\n");
  return 0;
}
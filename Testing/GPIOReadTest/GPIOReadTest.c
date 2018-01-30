#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>

#define READ() ((GPIO_ReadValue(0) & 0x20000) == 0x20000)

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
  uint64_t systickcnt;
  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}

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

void print(uint8_t string[])
{
  UART_Send(LPC_UART0, string, EL_SERIAL_SizeOfString(string), BLOCKING);
}

typedef struct Packet{
  uint8_t type_slot;
  uint8_t slots[512];
} Packet;

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

int getFrame(uint8_t * out_type, uint8_t * out_slots){
  int input_state;
  unsigned long start_time;
  int packet_count = 0;
  uint8_t slots[512];
  uint8_t type_slot = 0;
  MONITOR_STATE state = IDLE;
  uint8_t slot = 0;
  uint8_t break_bit = 0;
  uint8_t end_bits = 0;
  unsigned long times[11];
  int bit_index = 0;
  unsigned long bit_start_time = 0;
  int slot_count = 0;
  unsigned long slot_start_time = 0;
  unsigned long mab_end = 0;
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
        if(SysTickCnt - start_time > MICROSECOND*1000)
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
        if(SysTickCnt - start_time > MILISECOND*9)
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
    state = INITIAL_SLOT;
    start_time = SysTickCnt;
    unsigned long inter_slot_time = 0;
    while(state == INITIAL_SLOT || state == SLOT){
      while(slot_count < 513){
        slot = 0;
        bit_index = 0;
        end_bits = 0;
        while(READ() == 1);
        slot_start_time = SysTickCnt;
        //while(SysTickCnt - slot_start_time < MICROSECOND*2);
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
          if(bit_index < 10){
            while(SysTickCnt - bit_start_time < MICROSECOND*4);
          }
        }
        if(break_bit != 0 || end_bits != 2){
          errors++;
        }
        if(state == INITIAL_SLOT){
          state = SLOT;
          type_slot = slot;
        }else{
          slots[slot_count - 1] = slot;
        }
        while(SysTickCnt - slot_start_time < MICROSECOND*4*11 - 3*MICROSECOND);
        slot_count++;
      }
      if(slot_count == 513) state = END_FRAME;
    }
    if(state == END_FRAME)
      break;
    if(state == IDLE)
      continue;
  }
  int i = 0;
  while(i < 512){
    out_slots[i] = slots[i];
    i++;
  }
  out_type = type_slot;
  return errors;
}

int main(){
  SysTick_Config(SystemCoreClock/SECOND - 1);
  //1000000 == second
  //1000 == milisecond
  EL_SERIAL_Init();
  GPIO_SetDir(0, SENDING, 1);
  GPIO_SetDir(0, RECEIVING, 0);
  Packet data;
  print("START\r\n");
  uint8_t slots[512];
  uint8_t type_slot;
  int errors = getFrame(&type_slot, &slots);
  int i = 0;
  char out_str[256];
  sprintf(out_str, "Errors: %d\r\n", errors);
  print(out_str);
  sprintf(out_str, "Type Slot: %d\r\n", type_slot);
  print(out_str);
  while(i <32){
    sprintf(out_str, "slot[%d] = 0x%x\r\n", i, slots[i]);
    print(out_str);
    i++;
  }
  i = 0;
  int errors_80 = 0;
  while(i <512){
    if(slots[i] == 0x80)
      errors_80++;
    i++;
  }
  sprintf(out_str, "Errors 0x80: %d\r\n", errors_80);
  print(out_str);
  i = 1;
  int errors_not = 0;
  while(i <512){
    if(slots[i] != 0x00)
      errors_not++;
    i++;
  }
  sprintf(out_str, "Errors Not: %d\r\n", errors_not);
  print(out_str);
  print("END\r\n");
}

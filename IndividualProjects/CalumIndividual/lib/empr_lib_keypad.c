#ifndef EMPR_LIB_KEYPAD_C
#define EMPR_LIB_KEYPAD_C

#include "empr_lib_keypad.h"

const char EL_KEYPAD_MAP[4][4] = {
  {'D','#','0','*'},
  {'C','9','8','7'},
  {'B','6','5','4'},
  {'A','3','2','1'}
};

unsigned char EL_KEYPAD_ReadReset = 4;

unsigned char EL_KEYPAD_SeparateCols(uint8_t data){
  return (0xf0 & data) >> 4;
}

unsigned char EL_KEYPAD_SeparateRows(uint8_t data){
  return 0x0f & data;
}

unsigned char EL_KEYPAD_1HotToChar(uint8_t data){
  if(data == 0xe) return 0;
  else if(data == 0xd) return 1;
  else if(data == 0xb) return 2;
  else if(data == 0x7) return 3;
  else return 4;
}

void EL_KEYPAD_WriteRow(char row){
  unsigned char data[1];
  row = row & 0x0f;
  if(row == 0) data[0] = 0xfe;
  else if(row == 1) data[0] = 0xfd;
  else if(row == 2) data[0] = 0xfb;
  else if(row == 3) data[0] = 0xf7;
  else data[0] = 0xff;
  EL_I2C_SendBytes(EMPR_LIB_I2C_ADDRESS_KEYPAD, data, 1);
}

char EL_KEYPAD_ReadCol(){
  unsigned char data[1];
  EL_I2C_ReceiveBytes(EMPR_LIB_I2C_ADDRESS_KEYPAD, data, 1);
  return EL_KEYPAD_1HotToChar(EL_KEYPAD_SeparateCols(data[0]));
}

char EL_KEYPAD_ReadKey(){
  char current_row = 0;
  char return_char = '\0';
  while(return_char == '\0'){
    EL_KEYPAD_WriteRow(current_row);
    char col_result = EL_KEYPAD_ReadCol();
    if(col_result == 4 && EL_KEYPAD_ReadReset == current_row){
      EL_KEYPAD_ReadReset = 4;
    }else if (EL_KEYPAD_ReadReset == 4 && col_result != 4){
      return_char = EL_KEYPAD_MAP[(int) current_row][(int) col_result];
      EL_KEYPAD_ReadReset = current_row;
    }
    current_row = (current_row + 1) % 4;
  }
  return return_char;
}

char EL_KEYPAD_CheckKey(){
  char current_row = 0;
  char return_char = '\0';
  while(return_char == '\0' && current_row < 4){
    EL_KEYPAD_WriteRow(current_row);
    char col_result = EL_KEYPAD_ReadCol();
    if(col_result == 4 && EL_KEYPAD_ReadReset == current_row){
      EL_KEYPAD_ReadReset = 4;
    }else if (EL_KEYPAD_ReadReset == 4 && col_result != 4){
      return_char = EL_KEYPAD_MAP[(int) current_row][(int) col_result];
      EL_KEYPAD_ReadReset = current_row;
    }
    current_row = current_row + 1;
  }
  return return_char;
}

#endif

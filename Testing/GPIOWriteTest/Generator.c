#include <GenericLibraries.c>

/*#define setdata(int1,int2,int3) (data[0] = int1; data[2] = int2; data[3] = int3)

uint8_t read_buff[0];
uint8_t colour[9][3][255];


// non-state functions
uint8_t navigate(void);
void colour(uint8_t number);
void sequence(uint8_t number);

// "States"
void main_menu(void);
void def_menu(void);
void opt_menu(void);


uint8_t navigate(void){
  get_keypad_press(read_buff);
  //the decoded keypad "rrcc" format makes a sequence from 0-15. Counting column then row.
  return decode_keypad(read_buff);
}

void colour(uint8_t number){
  setdata(colour[number][0],colour[number][1],colour[number][2]);
}

void main_menu(void){
  uint8_t key = navigate();

  //Display Text

  switch (key){
    case 0: colour(0);
      break;
    case 4: colour(1);
      break;
    case 8: colour(2);
      break;
    case 12: sequence(0);
      break;
    case 1: colour(3);
      break;
    case 5: colour(4);
      break;
    case 9: colour(5);
      break;
    case 13: sequence(1);
      break;
    case 2: colour(6);
      break;
    case 6: colour(7);
      break;
    case 10: colour(8);
      break;
    case 14: sequence(2);
      break;
    case 3: def_menu();
      break;
    case 7: //TODO ZERO
      break;
    case 11: opt_menu();
      break;
    case 15: sequence(3);
  }
}
*/

int main(void){
  Full_Init();
  SEGMENT_Write(1,0);
  //SEGMENT_WriteHidden(1, 0, 1);
  //set_basic_data();
  /* TEST CODE
  while(1){
    get_keypad_press(read_buff);

    int temp = decode_keypad(read_buff[0]);

    char address[2];
    address[0] = 0x00;
    char buff_char[2];
    buff_char[0] = 0x40;
    address[1] = 0x80;
    buff_char[1] = LOOKUP[(temp >> 2) & 3][temp & 3];

    write_i2c(address, 2, LCD_ADDRESS);
    write_i2c(buff_char, 2, LCD_ADDRESS);

    data[0] = 255 * temp&1;
    data[1] = 255 * temp&2;
    data[2] = 255 * temp&4;

    send_data_UART(WAIT);
  }
  */


  //main_menu();
}

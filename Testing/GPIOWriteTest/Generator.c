#include <GenericLibraries.c>

#define setdata(int1,int2,int3) data[0] = int1; data[1] = int2; data[2] = int3
/*#define _cb(no,com) case no: com; break;
#define menu(com1, com2, com3, com4, com5, \
  com6, com7, com8, com9, com10, com11,\
  com12, com13, com14, com15, com16) \
  do { uint8_t key = navigate(); \
  switch (key){\
    _cb(0, com1)  _cb(1, com2)  _cb(2, com3)  _cb(3, com4)\
    _cb(4, com5)  _cb(5, com6)  _cb(6, com7)  _cb(7, com8)\
    _cb(8, com9)  _cb(9, com10) _cb(10,com11) _cb(11,com12)\
    _cb(12,com13) _cb(13,com14) _cb(14,com15) _cb(15,com16)}\
  } while(0)*/
#define DELAY 500000

uint8_t read_buff[0];
uint8_t colour[9][3];
uint8_t* sequence[4][16];


// non-state functions
uint8_t navigate(void);
void display_colour(uint8_t number);
void display_sequence(uint8_t number);

// "States"
void define_colour(uint8_t number);
void define_sequence(uint8_t number);
void main_menu(void);
void def_menu(void);
void opt_menu(void);


uint8_t navigate(void){
  get_keypad_press(read_buff);
  //the decoded keypad "rrcc" format makes a sequence from 0-15. Counting column then row.
  return decode_keypad(read_buff[0]);
}
void display_colour(uint8_t number){
  //display LCD
  //set data
  setdata(colour[number][0],colour[number][1],colour[number][2]);
  //light up lamp
  send_data_UART(NONE_BLOCKING);
}
void display_sequence(uint8_t number){
  //display LCD
  //display sequence
  int i = 0;
  while(i<16){
    if (sequence[number]){
      setdata(sequence[number][i][0],sequence[number][i][1],sequence[number][i][2]);
    }
    send_data_UART(BLOCKING);
    Delay(DELAY);
    i++;
  }
}

void define_colour(uint8_t number){
  display_colour(number);
}
void define_sequence(uint8_t number){
  display_sequence(number);
}


//
void def_menu(void){
  //Display LCD
/*
  while(1) menu(
    define_colour(1),
    define_colour(2),
    define_colour(3),
    define_sequence(0),
    define_colour(4),
    define_colour(5),
    define_colour(6),
    define_sequence(1),
    define_colour(7),
    define_colour(8),
    define_colour(9),
    define_sequence(2),
    ,//TODO asterix,
    ,//TODO ZERO
    return,
    define_sequence(3)
  )*/
}

//
void opt_menu(void){

}

void main_menu(void){
  //Display LCD
/*
  while (1) menu(
    display_colour(0),
    display_colour(1),
    display_colour(2),
    display_sequence(9),
    display_colour(3),
    display_colour(4),
    display_colour(5),
    display_sequence(1),
    display_colour(6),
    display_colour(7),
    display_colour(8),
    display_sequence(2),
    def_menu(),
    ,//TODO ZERO,
    opt_menu(),
    display_sequence(3)
  )*/
}

int main(void){
  Full_Init();
  set_basic_data();
  //send_data_UART(BLOCKING);

  colour[0][0] = 255;
  colour[0][1] = 0;
  colour[0][2] = 0;

  colour[1][0] = 0;
  colour[1][1] = 255;
  colour[1][2] = 0;

  colour[2][0] = 0;
  colour[2][1] = 0;
  colour[2][2] = 255;

  colour[4][0] = 255;
  colour[4][1] = 255;
  colour[4][2] = 0;

  int i;
  for (i = 0; i < 5; i++){
    sequence[0][i] = colour[i];
  }

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
  while(1) main_menu();
}

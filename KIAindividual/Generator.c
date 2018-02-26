#include "GenericLibraries.c"

#define setdata(int1,int2,int3) data[0] = int1; data[1] = int2; data[2] = int3
#define _cb(no,com) case no: {com;break;}
#define menu(com1, com2, com3, com4, com5, \
  com6, com7, com8, com9, com10, com11,\
  com12, com13, com14, com15, com16) \
  do { uint8_t key = navigate(); \
  switch (key){\
    _cb(0, com1)  _cb(1, com2)  _cb(2, com3)  _cb(3, com4)\
    _cb(4, com5)  _cb(5, com6)  _cb(6, com7)  _cb(7, com8)\
    _cb(8, com9)  _cb(9, com10) _cb(10,com11) _cb(11,com12)\
    _cb(12,com13) _cb(13,com14) _cb(14,com15) _cb(15,com16)}\
  } while(0)
#define DELAY 500000

uint8_t char_buff[1];
uint8_t read_buff[0];
uint8_t out_buff[32];
uint8_t colour[9][3];
uint8_t* sequence[4][16];
static uint8_t LCDcount = 0;

////////////////////////////////////////////////////////////////////////////////

// Non-state functions
uint8_t navigate(void);
void display_colour(uint8_t number);
void display_sequence(uint8_t number); // printKeyToLCD(number,LCDcount);

// States
void define_colour(uint8_t number);
void define_sequence(uint8_t number);
void main_menu(void);
void def_menu(void);
void opt_menu(void);

/////////////////////////////////////////////////////////////////////////////////

//
void write_input_number(uint number_input[3]){
  
}

// Non-state functions

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
  send_data_UART(BLOCKING);
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

// States
void define_colour(uint8_t number){
  //LCD Display
  uint8_t string[15];
  sprintf(string,"Col %d: VAL: --- SAVE rgb ->A,B,C", number);
  display_LCD(string,0);

  uint8_t number_input[3];
  uint8_t in = 2;

  input:
  while(in > 0){
    menu(
      display_LCD("1",12+in); number_input[in] = 1; in--,
      display_LCD("2",12+in); number_input[in] = 2; in--,
      display_LCD("3",12+in); number_input[in] = 3; in--,
      ,
      display_LCD("4",12+in); number_input[in] = 4; in--,
      display_LCD("5",12+in); number_input[in] = 5; in--,
      display_LCD("6",12+in); number_input[in] = 6; in--,
      ,
      display_LCD("7",12+in); number_input[in] = 7; in--,
      display_LCD("8",12+in); number_input[in] = 8; in--,
      display_LCD("9",12+in); number_input[in] = 9; in--,
      display_LCD("---",12);
        number_input[0]=0;
        number_input[1]=0;
        number_input[2]=0;
        in = 2,
      display_LCD("0",12+in); number_input[in] = 0; in--,
      ,
      ,
    );
  }
  menu(
      ,
      ,
      ,
      ,
      ,
      ,
      ,
      ,
      ,
      ,
      ,
      ,
      ,
      ,
      ,
  );
  goto input;
}
void define_sequence(uint8_t number){

}

void def_menu(void){
  display_LCD("DEF: 1-9 or A-D *:DISP ALL #:CXL",0);

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
  );
}
void opt_menu(void){

}
void main_menu(void){
  //Display LCD
  display_LCD("1-9:Col, A-D:Seq", 0);
  display_LCD("*: Def, #:Repeat", 16);


  while (1) menu(
    display_colour(0),
    display_colour(1),
    display_colour(2),
    display_sequence(0),
    display_colour(3),
    display_colour(4),
    display_colour(5),
    display_sequence(1),
    display_colour(6),
    display_colour(7),
    display_colour(8),
    display_sequence(2),
    def_menu(); return,
    ,//TODO ZERO,
    opt_menu(); return,
    display_sequence(3)
  );
}

int main(void){
  Full_Init();
  LCD_clear();

  /* //DEBUG light
  set_basic_data();
  display_colour(1);
  send_data_UART(BLOCKING);
  //*/

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


  /* //DEBUG LCD and keypad
  LCD_clear();
  while(1){
    get_keypad_press(read_buff);
    display_colour(0);
    printKeyToLCD(decode_keypad(read_buff[0]),LCDcount);
  }
  //*/



  //LCD_clear();


  //write_i2c(char_buff,2,LCD_ADDRESS);

  while(1) main_menu();
}

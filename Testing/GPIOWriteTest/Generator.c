#include "GenericLibraries.c"

#define setdata(int1,int2,int3) data[0] = int1; data[1] = int2; data[2] = int3
#define _cb(no,com) case no: {com;break;}
#define menu(com1, com2, com3, com4, com5, \
  com6, com7, com8, com9, com10, com11,\
  com12, com13, com14, com15, com16) \
  do { uint8_t key = read_keypress(); \
  switch (key){\
    _cb(0, com1)  _cb(1, com2)  _cb(2, com3)  _cb(3, com4)\
    _cb(4, com5)  _cb(5, com6)  _cb(6, com7)  _cb(7, com8)\
    _cb(8, com9)  _cb(9, com10) _cb(10,com11) _cb(11,com12)\
    _cb(12,com13) _cb(13,com14) _cb(14,com15) _cb(15,com16)}\
  } while(0)
#define clear_number_input() number_input[0] = 0;number_input[1] = 0;number_input[2] = 0; in_count = 0
#define DELAY 500000

int8_t char_buff[1];
int8_t out_buff[32];
uint8_t colour[10][3];
uint8_t sequence[4][16];
uint8_t sequence_lengths[4] = {4,0,0,0};
uint8_t sequence_repeat[4] = {0,0,0,0};
static uint8_t LCDcount = 0;

////////////////////////////////////////////////////////////////////////////////
// Extra Functions
uint8_t input_translate(uint8_t number_input[3], uint8_t in_count);

// Non-state functions
void display_colour(uint8_t number);
void display_sequence(uint8_t number); // printKeyToLCD(number,LCDcount);
void copy_sequence(uint8_t def_seq_number, uint8_t seq_tobecopied_number, uint8_t* in_count);
void add_seq_to_display(uint8_t number, uint8_t in_count);
void clear_sequence(uint8_t number);

// States
void define_colour(uint8_t number);
void define_sequence(uint8_t number);
void main_menu(void);
void def_menu(void);
void opt_menu(void);

/////////////////////////////////////////////////////////////////////////////////

//maths and extra stuff
uint8_t input_translate(uint8_t number_input[3], uint8_t in_count){
  uint16_t output = 0;
  if(in_count == 0){
    output = 0;
  } else if (in_count == 1){
    output = number_input[0];
  } else if (in_count == 2){
    output = (number_input[0] * 10) + number_input[1];
  } else if (in_count == 3){
    output = (number_input[0] * 100) + (number_input[1] * 10) + number_input[2];
  }
  if (output > 255) output = 255;

  return (uint8_t) output;
}

// Non-state functions


void display_colour(uint8_t number){
  //set data
  setdata(colour[number][0],colour[number][1],colour[number][2]);
  //light up lamp
  send_data_UART(BLOCKING);
}
void display_sequence(uint8_t number){
  //display sequence
  int i = 0;
  int rep;
  for (rep = 0; rep <= sequence_repeat[number]; rep++){
    for(i = 0; i < sequence_lengths[number]; i++){
      if (sequence[number]){
        setdata(colour[sequence[number][i]][0],colour[sequence[number][i]][1],colour[sequence[number][i]][2]);
      }
      send_data_UART(BLOCKING);
      Delay(DELAY);
    }
  }
}
void display_all(){
  display_LCD("Displaying all: -              -",0);
  int8_t str[1];
  //Colours
  int i;
  for (i = 0; i < 10; i++){
    sprintf(str, "%d",i);
    display_LCD(str,17+i);
    display_colour(i);
    Delay(500000);
  }
  display_LCD("A",17+(i++));
  display_sequence(0);
  Delay(500000);
  display_LCD("B",17+(i++));
  display_sequence(1);
  Delay(500000);
  display_LCD("C",17+(i++));
  display_sequence(2);
  Delay(500000);
  display_LCD("D",17+(i++));
  display_sequence(3);
  Delay(500000);
}
void copy_sequence(uint8_t def_seq_number, uint8_t seq_tobecopied_number, uint8_t* in_count){
  uint8_t copy_seq[16];
  int i;
  //make a copy of copy_seq because it may be pointing to the same as def_seq.
  for (i = 0; i < sequence_lengths[seq_tobecopied_number]; i++){copy_seq[i] = sequence[seq_tobecopied_number][i];}

  for (i = 0; i < sequence_lengths[seq_tobecopied_number]; i++){
    sequence[def_seq_number][*in_count] = copy_seq[i];
    ++*in_count;
  }
}
void add_seq_to_display(uint8_t number, uint8_t in_count){
  int8_t str[1];
  int i;

  for (i = 0; i < sequence_lengths[number]; i++){
    sprintf(str, "%d", sequence[number][i]+1);
    display_LCD(str, 16+in_count+i);
  }
}
void clear_sequence(uint8_t number){
  sequence_lengths[number] = 0;
}

// States
void define_colour(uint8_t number){
  int8_t string[32];
  display_LCD("Set rgb ->A,B,C,Save to mem: ->D",0);
  Delay(1000000);
  sprintf(string,"Col %d: VAL: --- SAVE rgb ->A,B,C", (number+1));
  display_LCD(string,0);

  uint8_t number_input[3];
  number_input[0] = 0;
  number_input[1] = 0;
  number_input[2] = 0;
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t r_str[3];
  uint8_t g_str[3];
  uint8_t b_str[3];
  uint8_t in_count = 0;
  uint8_t reset = 0;

  while(1){
    if (reset==1){
      sprintf(string,"Col %d: VAL: --- SAVE rgb ->A,B,C", number);
      display_LCD(string,0);
      clear_number_input();
      reset = 0;
    }

    menu(
      if(in_count < 3) display_LCD("1",12+in_count); number_input[in_count] = 1; in_count++,
      if(in_count < 3) display_LCD("2",12+in_count); number_input[in_count] = 2; in_count++,
      if(in_count < 3) display_LCD("3",12+in_count); number_input[in_count] = 3; in_count++,
      r = input_translate(number_input, in_count);
        display_LCD("Saved RED val.  Save to mem: ->D",0);
        Delay(1000000);
        reset = 1,
      if(in_count < 3) display_LCD("4",12+in_count); number_input[in_count] = 4; in_count++,
      if(in_count < 3) display_LCD("5",12+in_count); number_input[in_count] = 5; in_count++,
      if(in_count < 3) display_LCD("6",12+in_count); number_input[in_count] = 6; in_count++,
      g = input_translate(number_input, in_count);
        display_LCD("Saved GREEN val.Save to mem: ->D",0);
        Delay(1000000);
        reset = 1,
      if(in_count < 3) display_LCD("7",12+in_count); number_input[in_count] = 7; in_count++,
      if(in_count < 3) display_LCD("8",12+in_count); number_input[in_count] = 8; in_count++,
      if(in_count < 3) display_LCD("9",12+in_count); number_input[in_count] = 9; in_count++,
      b = input_translate(number_input, in_count);
        display_LCD("Saved BLUE val. Save to mem: ->D",0);
        Delay(1000000);
        reset = 1,
      display_LCD("---",12);
        clear_number_input(),
      if(in_count < 3) display_LCD("0",12+in_count); number_input[2-in_count] = 0; in_count++,
      return,
      colour[number][0] = r;
        colour[number][1] = g;
        colour[number][2] = b;
        display_LCD("Saved to memory.R:   G:   B:    ",0);
        sprintf(r_str, "%d",r);
        sprintf(g_str, "%d",g);
        sprintf(b_str, "%d",b);
        display_LCD(r_str, 18);
        display_LCD(g_str, 23);
        display_LCD(b_str, 28);
        display_colour(number);
        Delay(1000000);
        reset = 1;
        return
    );
  }
}
void define_sequence(uint8_t number){
  display_LCD("Type Sequence.   SAVE: * CLR: #",0);
  Delay(1000000);
  //uint8_t str[32];
  int8_t letter[1];
  switch (number) {
    case 0: letter[0]='A'; break;
    case 1: letter[0]='B'; break;
    case 2: letter[0]='C'; break;
    case 3: letter[0]='D'; break;
  }

  uint8_t reset = 1;
  uint8_t cancel = 0;
  uint8_t in_count = 0;
  while(1){
    if (reset == 1){
      display_LCD("Seq:  SAV*/CLR#",0);
      display_LCD(letter, 4);
      display_LCD("                ",16);
      in_count = 0;
      reset = 0;
    }
    if (cancel==1 && in_count > 0){
      cancel = 0;
      display_LCD("CLR#",11);
    }
    menu(
      if (in_count < 16) sequence[number][in_count] = 1; display_LCD("1",16+in_count); in_count++,
      if (in_count < 16) sequence[number][in_count] = 2; display_LCD("2",16+in_count); in_count++,
      if (in_count < 16) sequence[number][in_count] = 3; display_LCD("3",16+in_count); in_count++,
      add_seq_to_display(0,in_count); copy_sequence(number,0,&in_count),
      if (in_count < 16) sequence[number][in_count] = 4; display_LCD("4",16+in_count); in_count++,
      if (in_count < 16) sequence[number][in_count] = 5; display_LCD("5",16+in_count); in_count++,
      if (in_count < 16) sequence[number][in_count] = 6; display_LCD("6",16+in_count); in_count++,
      add_seq_to_display(1,in_count); copy_sequence(number,1,&in_count),//B
      if (in_count < 16) sequence[number][in_count] = 7; display_LCD("7",16+in_count); in_count++,
      if (in_count < 16) sequence[number][in_count] = 8; display_LCD("8",16+in_count); in_count++,
      if (in_count < 16) sequence[number][in_count] = 9; display_LCD("9",16+in_count); in_count++,
      add_seq_to_display(2,in_count); copy_sequence(number,2,&in_count),//C
      display_LCD("Saved Seq  .   ",0);display_LCD(letter, 10);Delay(1000000);return,//*
      if (in_count < 16) sequence[number][in_count] = 0; display_LCD("0",16+in_count); in_count++,
      if (cancel==0){
          display_LCD("CXL#    = = CLR = =  ",11);
          clear_sequence(number);
          cancel=1;
          in_count = 0;
          Delay(1000000);
          display_LCD("                ",16);
        } else {
          display_LCD(" = = CANCEL = = ", 16);
          Delay(1000000);
          return;
        },//#
      add_seq_to_display(3,in_count); copy_sequence(number,3,&in_count)//D
    );
    sequence_lengths[number] = in_count;
  }
}

void def_menu(void){
  while(1) {
    display_LCD("DEF: 0-9 or A-D *:DISP ALL #:CXL",0);
    menu(
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
      display_all(),
      define_colour(0),
      return,
      define_sequence(3)
    );
  }
}
void opt_menu(void){
  display_LCD("Select Seq and  times to repeat.",0);
  Delay(1000000);
  display_LCD("SEQ: A RPT: 0   SAVE: * CXL: #  ",0);

  uint8_t repeat = 0;
  uint8_t sequence = 0;
  while(1){
    menu(
      repeat = 1; display_LCD("1", 12),
      repeat = 2; display_LCD("2", 12),
      repeat = 3; display_LCD("3", 12),
      sequence = 0; display_LCD("A", 5),
      repeat = 4; display_LCD("4", 12),
      repeat = 5; display_LCD("5", 12),
      repeat = 6; display_LCD("6", 12),
      sequence = 1; display_LCD("B", 5),
      repeat = 7; display_LCD("7", 12),
      repeat = 8; display_LCD("8", 12),
      repeat = 9; display_LCD("9", 12),
      sequence = 2; display_LCD("C", 5),
      sequence_repeat[sequence] = repeat; display_LCD("  Saved repeat.                 ",0); Delay(1000000); return,
      repeat = 0; display_LCD("0", 12),
      return,
      sequence = 3; display_LCD("D", 5)
    );
  }
}
void main_menu(void){
  while (1) {
    display_LCD("0-9:Col, A-D:Seq*: Def, #:Repeat", 0);
    menu(
      display_colour(1),
      display_colour(2),
      display_colour(3),
      display_sequence(0),
      display_colour(4),
      display_colour(5),
      display_colour(6),
      display_sequence(1),
      display_colour(7),
      display_colour(8),
      display_colour(9),
      display_sequence(2),
      def_menu(),
      display_colour(0),
      opt_menu(),
      display_sequence(3)
    );
  }
}

int main(void){
  Full_Init();
  display_LCD("                                ",0);

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



  sequence[0][0] = 0;
  sequence[0][1] = 1;
  sequence[0][2] = 2;
  sequence[0][3] = 3;
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
  //add_seq_to_display(sequence[0],0);


  while(1) main_menu();
}

#include <GenericLibraries.c>

int main(void){
  Full_Init();
  set_basic_data();

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
}

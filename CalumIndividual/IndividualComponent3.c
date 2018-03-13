#include "GenericLibraries.c"
#include "lpc17xx_adc.h"

uint8_t intensity_at_interval[100];
uint8_t sequence_of_colours[20][3];

void IC3_menu(void);

void LCD_clear_lower(void){
	uint8_t i;
  uint8_t address[2];
  address[0] = 0x00;
  uint8_t buff_char[2];
  buff_char[0] = 0x40;
	for(i = 0; i < 16; i++){
    address[1] = 0xC0 + i;
    buff_char[1] = 0xA0;
    write_i2c(address, 2, LCD_ADDRESS);
    write_i2c(buff_char, 2, LCD_ADDRESS);
  }
}

void LED_Clear(void){
	uint8_t i;
	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	for (i; i < 20; i++){
		sequence_of_colours[i][0] = 0;
		sequence_of_colours[i][1] = 0;
		sequence_of_colours[i][2] = 0;
	}
	send_data_UART(1);
}

int print_usb(char *buf,uint8_t length)
{
	return(UART_Send((LPC_UART_TypeDef *)LPC_UART0,(uint8_t *)buf,length, BLOCKING));
}

void serial_init(void)
{
	UART_CFG_Type UARTConfigStruct;			// UART Configuration structure variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;	// UART FIFO configuration Struct variable
	PINSEL_CFG_Type PinCfg;				// Pin configuration for UART
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	// USB serial first
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);
	// Initialize UART Configuration parameter structure to default state:
	UART_ConfigStructInit(&UARTConfigStruct);
	// Initialize FIFOConfigStruct to default state:
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_Init((LPC_UART_TypeDef *)LPC_UART0, &UARTConfigStruct);		// Initialize UART0 peripheral with given to corresponding parameter
	UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART0, &UARTFIFOConfigStruct);	// Initialize FIFO for UART0 peripheral
	UART_TxCmd((LPC_UART_TypeDef *)LPC_UART0, ENABLE);			// Enable UART Transmit
}

void ADC_Config_Piezo(void){
  PINSEL_CFG_Type pincfg;
  pincfg.Funcnum = 1;
  pincfg.OpenDrain = 0;
  pincfg.Pinmode = 0;
  pincfg.Pinnum = 23;
  pincfg.Portnum = 0;
  PINSEL_ConfigPin(&pincfg);
}

void ADC_Config_IR(void){
  PINSEL_CFG_Type pincfg;
  pincfg.Funcnum = 1;
  pincfg.OpenDrain = 0;
  pincfg.Pinmode = 0;
  pincfg.Pinnum = 24;
  pincfg.Portnum = 0;
  PINSEL_ConfigPin(&pincfg);
}

void ADC_Config_Break_Piezo(void){
	PINSEL_CFG_Type pincfg;
  pincfg.Funcnum = 1;
  pincfg.OpenDrain = 0;
  pincfg.Pinmode = 0;
  pincfg.Pinnum = 25;
  pincfg.Portnum = 0;
  PINSEL_ConfigPin(&pincfg);
}

uint8_t Get_Intensity_From_Piezo(void){
	uint8_t seg_val = 0;
	char out[16];
	display_LCD("Select intensity",0);
	display_LCD("Range: 0 to 255 ",16);
	Delay(2500000);
	display_LCD("Right piezo to  ",0);
	display_LCD("inc intensity   ",16);
	Delay(2500000);
	display_LCD("Press left piezo",0);
	display_LCD("to select value ",16);
	while(1){
		if (ADC_ChannelGetData(LPC_ADC, 0) < 1000){
			if (seg_val == 255) {
				seg_val = 0;
			} else {
				seg_val++;
			}
			SEGMENT_WriteHidden(seg_val,5,1);
			Delay(100000);
		}
		// sprintf(out, "L:%d R:%d\n\r", ADC_ChannelGetData(LPC_ADC, 0), ADC_ChannelGetData(LPC_ADC, 2));
		// print_usb(out, 15);
		if (ADC_ChannelGetData(LPC_ADC, 2) == 0){
			break;
		}
	}
	return seg_val;
}

uint8_t Get_Hue_From_Piezo(void){
	char out[16];
	uint8_t seg_val = 0;
	char* col;
	display_LCD("Select colour:  ",0);
	display_LCD("R=1, G=2, B=3   ",16);
	Delay(2500000);
	display_LCD("Right piezo     ",0);
	display_LCD("to change colour",16);
	Delay(2500000);
	display_LCD("Press left piezo",0);
	display_LCD("to select colour",16);
	while(1){
		if (ADC_ChannelGetData(LPC_ADC, 0) < 1000){
			if (seg_val == 3) {
				seg_val = 1;
			} else {
				seg_val++;
			}
			Delay(100000);
		}
		// sprintf(out, "L:%d R:%d\n\r", ADC_ChannelGetData(LPC_ADC, 0), ADC_ChannelGetData(LPC_ADC, 2));
		// print_usb(out, 15);
		SEGMENT_WriteHidden(seg_val,5,1);
		if (ADC_ChannelGetData(LPC_ADC, 2) == 0){
			break;
		}
	}
	if (seg_val == 0){
		col = "Red";
	} else if (seg_val == 1){
		col = "Green";
	} else {
		col = "Blue";
	}
	display_LCD("Hue =                           ",0);
	display_LCD(col, 6);
	return (seg_val-1);
}

uint8_t Get_Intensity_From_IR(void){
	uint8_t seg_val = 0;
	char out[12];
	display_LCD("Select intensity",0);
	display_LCD("Range: 0 to 255 ",16);
	Delay(2500000);
	display_LCD("Press left piezo",0);
	display_LCD("to select value ",16);
	while(1){
		// sprintf(out, "IR : %.3f\n\r", ADC_ChannelGetData(LPC_ADC, 1));
		// print_usb(out, 12);
		if ((ADC_ChannelGetData(LPC_ADC, 1) <= 3750) && (ADC_ChannelGetData(LPC_ADC, 1) > 1200)){
			seg_val = (ADC_ChannelGetData(LPC_ADC, 1) - 1200) * 0.1;
		} else if (ADC_ChannelGetData(LPC_ADC, 1) > 3750){
			seg_val = 255;
		} else if (ADC_ChannelGetData(LPC_ADC, 1) <= 1200){
			seg_val = 0;
		}
		SEGMENT_WriteHidden(seg_val,5,1);
		if (ADC_ChannelGetData(LPC_ADC, 2) == 0){
			break;
		}
		Delay(100000);
	}
	display_LCD("Intensity =                     ",0);
	Delay(10000);
	return seg_val;
}

uint8_t Get_Hue_From_IR(void){
	//0 - 8cm = Blue, 8 - 12cm = Green, 12 - 30cm = read
	uint8_t hue;
	char* col;
	display_LCD("Press left piezo",0);
	display_LCD("to select colour",16);
	Delay(2500000);
	display_LCD("Colour/Hue:     ",0);
	display_LCD("                ",16);
	while(1){
		if (ADC_ChannelGetData(LPC_ADC, 1) > 2950){
			display_LCD("Blue            ",16);
			hue = 2;
		} else if ((ADC_ChannelGetData(LPC_ADC, 1) <= 2950) && (ADC_ChannelGetData(LPC_ADC, 1) > 1850)){
			display_LCD("Green           ",16);
			hue = 1;
		} else if ((ADC_ChannelGetData(LPC_ADC, 1) <= 1850) && (ADC_ChannelGetData(LPC_ADC, 1) > 1150)){
			display_LCD("Red             ",16);
			hue = 0;
		}
		if (ADC_ChannelGetData(LPC_ADC, 2) == 0){
			break;
		}
		Delay(1000000);
	}
	display_LCD("Hue =                           ",0);
	if (hue == 0){
		col = "Red";
	} else if (hue == 1){
		col = "Green";
	} else {
		col = "Blue";
	}
	display_LCD(col, 6);
	return hue;
}

void Set_Colour_Via_IR(void){
	uint8_t hue = Get_Hue_From_IR();
	uint8_t intensity = Get_Intensity_From_IR();
	data[hue] = intensity;
	send_data_UART(1);
}

void Set_Colour_Via_Piezo(void){
	uint8_t hue = Get_Hue_From_Piezo();
	uint8_t intensity = Get_Intensity_From_Piezo();
	data[hue] = intensity;
	send_data_UART(1);
}

int State_menu(void){
	uint8_t in_count = 0;
	uint8_t number_input[3];
	number_input[0] = 0;
	number_input[1] = 0;
	number_input[2] = 0;
	while(1){
		menu(
			if (in_count < 3) display_LCD("1",7+in_count); number_input[in_count] = 1; in_count++,
			if (in_count < 3) display_LCD("2",7+in_count); number_input[in_count] = 2; in_count++,
			if (in_count < 3) display_LCD("3",7+in_count); number_input[in_count] = 3; in_count++,
			return input_translate(number_input, in_count),
			if (in_count < 3) display_LCD("4",7+in_count); number_input[in_count] = 4; in_count++,
			if (in_count < 3) display_LCD("5",7+in_count); number_input[in_count] = 5; in_count++,
			if (in_count < 3) display_LCD("6",7+in_count); number_input[in_count] = 6; in_count++,
			return 513,
			if (in_count < 3) display_LCD("7",7+in_count); number_input[in_count] = 7; in_count++,
			if (in_count < 3) display_LCD("8",7+in_count); number_input[in_count] = 8; in_count++,
			if (in_count < 3) display_LCD("9",7+in_count); number_input[in_count] = 9; in_count++,
			break,
			break,
			if (in_count < 3) display_LCD("0",7+in_count); number_input[in_count] = 0; in_count++,
			break,
			break
		);
	}
}

void Define_Sequence(uint8_t Piezo_OR_IR){ // 0 or 1 resp.
	uint8_t key;
	int pos_in_seq = 0;
	uint8_t hue;
	uint8_t intensity;
	char out[32];
	while(1){
		display_LCD("Please type in  state           ",0);
		Delay(1500000);
		display_LCD("Press A to set  state           ",0);
		Delay(1500000);
		display_LCD("Press B to      return to main  ",0);
		Delay(1500000);
		display_LCD("State:                          ",0);
		Delay(100000);
		pos_in_seq = State_menu();
		if (pos_in_seq > 512){
			return;
		}
		if (Piezo_OR_IR == 0){
			hue = Get_Hue_From_Piezo();
			SEGMENT_WriteHidden(0,5,1);
			intensity = Get_Intensity_From_Piezo();
			SEGMENT_WriteHidden(0,5,1);
		} else {
			hue = Get_Hue_From_IR();
			SEGMENT_WriteHidden(0,5,1);
			intensity = Get_Intensity_From_IR();
			SEGMENT_WriteHidden(0,5,1);
		}
		sequence_of_colours[pos_in_seq][hue] = intensity;
		sprintf(out,"State %.3d R:%.3d,G:%.3d, B:%.3d    ", pos_in_seq, sequence_of_colours[pos_in_seq][0],	sequence_of_colours[pos_in_seq][1], sequence_of_colours[pos_in_seq][2]);
		display_LCD(out,0);
		Delay(2500000);
	}
}

void IC3_Display_Sequence(void){
	uint8_t i;
	char out[32];
	for (i = 0; i < 64; i++){
		sprintf(out,"State %.3d R:%.3d,G:%.3d, B:%.3d    ", i, sequence_of_colours[i][0],	sequence_of_colours[i][1], sequence_of_colours[i][2]);
		display_LCD(out,0);
		data[0] = sequence_of_colours[i][0];
		data[1] = sequence_of_colours[i][1];
		data[2] = sequence_of_colours[i][2];
		send_data_UART(1);
		Delay(750000);
	}
}



void IC3_menu(void){
	uint8_t reset = 1;
  while (1) {
    if (reset == 1){
			SEGMENT_WriteHidden(0, 5, 1);
			display_LCD("0-9 : Return                    ", 0);
			Delay(1000000);
			display_LCD("# : Clear       sequence        ", 0);
			Delay(1000000);
			display_LCD("* : Display     sequence        ", 0);
			Delay(1000000);
			display_LCD("A : Set colour  using Infra-Red ", 0);
			Delay(1000000);
			display_LCD("B : Set colour  using Piezo     ", 0);
			Delay(1000000);
			display_LCD("C : Define seq  using IR        ", 0);
			Delay(1000000);
			display_LCD("D : Define seq  using Piezo     ", 0);
			Delay(1000000);
			reset = 0;
		}
		menu(
			return,
			return,
			return,
			Set_Colour_Via_IR(); reset = 1,
			return,
			return,
			return,
			Set_Colour_Via_Piezo(); reset = 1,
			return,
			return,
			return,
			Define_Sequence(1); reset = 1,
			IC3_Display_Sequence(); reset = 1,
			return,
			LED_Clear(); reset = 1,
			Define_Sequence(0); reset = 1
		);
  }
}


void IC3(void){
	serial_init();
  Full_Init();
  LCD_clear();
  uint8_t seg_val = 0;
  char test[10] = "test\n\r";
  print_usb(test,10);
  //Configure the pin connect (multiplexer) block
  ADC_Config_Piezo();
  ADC_Config_IR();
	ADC_Config_Break_Piezo();
  //Call ADC-Init for chosen channel, specifying the clock rate (conversion rate)
  ADC_Init(LPC_ADC, 100);
  ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);
  //Enable the channel number to be used
  ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
  ADC_ChannelCmd(LPC_ADC, 1, ENABLE);
	ADC_ChannelCmd(LPC_ADC, 2, ENABLE);
	char read_buff[1];
  char buff[1] = {0x0F};
  write_i2c(buff,1,0x21);
	send_data_UART(1);
	IC3_menu();
}


int main(void){
	IC3();
	return 0;
}

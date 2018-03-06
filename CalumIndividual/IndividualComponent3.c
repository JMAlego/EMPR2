#include "GenericLibraries.c"
#include "lpc17xx_adc.h"

uint8_t intensity_at_interval[100];
uint8_t sequence_of_colours[20][3];

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

void Save_Pattern_Using_IR(void){
	uint8_t interval;
	uint8_t key;
	display_LCD("Create fade out,",0);
	display_LCD("Max length of 10",16);
	Delay(1000000);
	display_LCD("Max length of 10",0);
	display_LCD("seconds,        ",16);
	Delay(1000000);
	display_LCD("seconds,        ",0);
	display_LCD("                ",16);
	Delay(1000000);
	display_LCD("The closer to   ",0);
	display_LCD("the sensor, the ",16);
	Delay(1000000);
	display_LCD("the sensor, the ",0);
	display_LCD("brighter the    ",16);
	Delay(1000000);
	display_LCD("brighter the    ",0);
	display_LCD("light.          ",0);
	Delay(1500000);
	display_LCD("Press A to begin",0);
	display_LCD("Press # to end  ",16);
	Delay(1500000);
	while(1){
		if (read_keypress() == 3) {
			break;
		}
	}
	for (interval = 0; interval < 100; interval++){
		if ((ADC_ChannelGetData(LPC_ADC, 1) <= 3750) && (ADC_ChannelGetData(LPC_ADC, 1) > 1200)){
			intensity_at_interval[interval] = (ADC_ChannelGetData(LPC_ADC, 1) - 1200) * 0.1;
		} else if (ADC_ChannelGetData(LPC_ADC, 1) > 3750){
			intensity_at_interval[interval] = 255;
		} else if (ADC_ChannelGetData(LPC_ADC, 1) <= 1200){
			intensity_at_interval[interval] = 0;
		}
		SEGMENT_WriteHidden(intensity_at_interval[interval],5,1);
		if (read_keypress() == 14){
			break;
		}
		Delay(100000);
	}
	display_LCD("Pattern saved      ",0);display_LCD("Press # to      ",0);
	display_LCD("select intensity",16);
	display_LCD("                   ",16);
	//=====================IMPLEMENT AFTER FINAL STATE============================
	for (interval = 0; interval < 100; interval++){
		data[0] = (data[0] / 255) * intensity_at_interval[interval];
		data[1] = (data[1] / 255) * intensity_at_interval[interval];
		data[2] = (data[2] / 255) * intensity_at_interval[interval];
		send_data_UART(1);
		Delay(100000);
	}
}

uint8_t Get_Intensity_From_Piezo(void){
	uint8_t seg_val = 0;
	char out[8];
	display_LCD("Select intensity",0);
	display_LCD("Range: 0 to 255 ",16);
	Delay(100000)
	display_LCD("Press # to      ",0);
	display_LCD("select intensity",16);
	if (ADC_ChannelGetData(LPC_ADC, 0) < 2000){
		sprintf(out, "TRUE\n\r");
		print_usb(out, 6);
		if (seg_val == 255) {
			seg_val = 0;
		} else {
			seg_val++;
		}
		SEGMENT_WriteHidden(seg_val,5,1);
	}
	return seg_val;
}

uint8_t Get_Hue_From_Piezo(void){
	uint8_t seg_val = 0;
	display_LCD("Select colour:  ",0);
	display_LCD("R=1, G=2, B=3   ",16);
	Delay(100000)
	display_LCD("Press # to      ",0);
	display_LCD("select colour   ",16);
	while(1){
		if (ADC_ChannelGetData(LPC_ADC, 0) < 2000){
			if (seg_val == 3) {
				seg_val = 1;
			} else {
				seg_val++;
			}
		}
		SEGMENT_WriteHidden(seg_val,5,1);
		if (read_keypress() == 14){
			break;
		}
	}
	return seg_val;
}

uint8_t Get_Intensity_From_IR(void){
	uint8_t seg_val = 0;
	display_LCD("Select intensity",0);
	display_LCD("Range: 0 to 255 ",16);
	Delay(100000)
	display_LCD("Press # to      ",0);
	display_LCD("select intensity",16);
	while(1){
		if ((ADC_ChannelGetData(LPC_ADC, 1) <= 3750) && (ADC_ChannelGetData(LPC_ADC, 1) > 1200)){
			seg_val = (ADC_ChannelGetData(LPC_ADC, 1) - 1200) * 0.1;
		} else if (ADC_ChannelGetData(LPC_ADC, 1) > 3750){
			seg_val = 255;
		} else if (ADC_ChannelGetData(LPC_ADC, 1) <= 1200){
			seg_val = 0;
		}
		SEGMENT_WriteHidden(seg_val,5,1);
		if (read_keypress() == 14){
			break;
		}
	}
	return seg_val;
}

uint8_t Get_Hue_From_IR(void){
	//0 - 8cm = Blue, 8 - 12cm = Green, 12 - 30cm = read
	uint8_t hue;
	display_LCD("Press # to      ",0);
	display_LCD("select colour   ",16);
	Delay(100000)
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
		if (read_keypress() == 14){
			break;
		}
		Delay(1000000);
	}
	return hue;
}

void Set_Colour_Via_IR(void){
	uint8_t hue = Get_Hue_From_IR();
	uint8_t intensity = Get_Intensity_From_IR();
	data[hue] = intensity;
	send_data_UART(1);
}

void Set_Colour_Via_IR(void){
	uint8_t hue = Get_Hue_From_Piezo();
	uint8_t intensity = Get_Intensity_From_Piezo();
	data[hue] = intensity;
	send_data_UART(1);
}

void Define_Sequence(uint8_t Piezo_OR_IR){ // 0 or 1 resp.
	uint8_t key;
	uint8_t pos_in_seq = 0;
	uint8_t hue;
	uint8_t intensity;
	display_LCD("After defining  a hue, press A  ",0);
	Delay(1000000);
	display_LCD("To move to next state, press *  ",0);
	Delay(1000000);
	display_LCD("When finished,  press #         ",0);
	Delay(1000000);
	while(1){
		if (Piezo_OR_IR == 0){
			hue = Get_Hue_From_Piezo();
			intensity = Get_Intensity_From_Piezo();
		} else {
			hue = Get_Hue_From_IR();
			intensity = Get_Intensity_From_IR();
		}
		sequence_of_colours[pos_in_seq][hue] = intensity;
		display_LCD("State XX. R:XXX,G:XXX, B:XXX.   ",0);
		display_LCD(pos_in_seq,6);
		display_LCD(sequence_of_colours[pos_in_seq][0],12);
		display_LCD(sequence_of_colours[pos_in_seq][1],18);
		display_LCD(sequence_of_colours[pos_in_seq][2],25);
		while(1){
			key = read_keypress();
			if (key == 3 || key == 12 || key == 14){
				break;
			}
		}
		if(key == 12){
			pos_in_seq++;
		} else if (key == 14){
			break;
		}
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
  //Call ADC-Init for chosen channel, specifying the clock rate (conversion rate)
  ADC_Init(LPC_ADC, 100);
  ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);
  //Enable the channel number to be used
  ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
  ADC_ChannelCmd(LPC_ADC, 1, ENABLE);
	char read_buff[1];
  char buff[1] = {0x0F};
  write_i2c(buff,1,0x21);
  SEGMENT_WriteHidden(seg_val, 5, 1); // Write 0
}

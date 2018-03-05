#include "GenericLibraries.c"
#include "lpc17xx_adc.h"

//double samples[5] = {1.12, 1.36, 1.68, 2.40, 3.36};
int intensity_at_interval[100];

void LCD_clear_lower(void){
	int i;
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

int print_usb(char *buf,int length)
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

/* Initialize UART Configuration parameter structure to default state:
	 * - Baudrate = 9600bps
	 * - 8 data bit
	 * - 1 Stop bit
	 * - None parity
*/
	UART_ConfigStructInit(&UARTConfigStruct);

/* Initialize FIFOConfigStruct to default state:
	 * - FIFO_DMAMode = DISABLE
	 * - FIFO_Level = UART_FIFO_TRGLEV0
	 * - FIFO_ResetRxBuf = ENABLE
	 * - FIFO_ResetTxBuf = ENABLE
	 * - FIFO_State = ENABLE
*/
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Built the basic structures, lets start the devices/
	// USB serial
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

// void IR_Calibration(){                                        //Voltage proportional to distance throughout
//   display_LCD("CALIBRATION MODE", 0);
//
//   uint8_t key;																																//Key pressed (if = 0, end loop)
//   double raw_adc, voltage;
//   char output[16];
//
//   display_LCD("Object at 80cm", 16);
//   while (key != 14){
//     key = read_keypress();
//   }                                                        										//When a key (Piezo?) pressed, then test data ready
//   raw_adc = ADC_ChannelGetData(LPC_ADC, 1);                                   //Analogue signal from pin 16 converted to digital
//   voltage = (raw_adc / 4096) * 3.3;                                           //Voltage level of pin 16 ((/ 4096) * 3.3)
//   samples[0] = voltage;
//   sprintf(output, "READ:%fV", voltage);
//   display_LCD(output,16);
// 	Delay(20000);
// 	LCD_clear_lower();
//
//   display_LCD("Object at 40cm",16);
//   while (key != 14){
//     key = read_keypress();
//   }
//   raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
//   voltage = (raw_adc / 4096) * 3.3;
//   samples[1] = voltage;
//   sprintf(output, "READ: %fV", voltage);
//   display_LCD(output,16);
// 	Delay(20000);
// 	LCD_clear_lower();
//
//   display_LCD("Object at 10cm", 16);
//   while (key != 14){
//     key = read_keypress();
//   }
//   raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
//   voltage = (raw_adc / 4096) * 3.3;
//   samples[2] = voltage;
// 	sprintf(output, "READ:%fV", voltage);
//   display_LCD(output,16);
// 	Delay(20000);
// 	LCD_clear_lower();
//
//   display_LCD("Object at 7cm ", 16);
//   while (key != 14){
//     key = read_keypress();
//   }
//   raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
//   voltage = (raw_adc / 4096) * 3.3;
//   samples[3] = voltage;
//   sprintf(output, "READ:%fV", voltage);
//   display_LCD(output,16);
// 	Delay(20000);
// 	LCD_clear_lower();
//
//   display_LCD("Object at 10cm ", 16);
//   while (key != 14){
//     key = read_keypress();
//   }
//   raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
//   voltage = (raw_adc / 4096) * 3.3;
//   samples[4] = voltage;
//   sprintf(output, "READ:%fV", voltage);
//   display_LCD(output,16);
// 	Delay(20000);
//   LCD_clear();
//   display_LCD("DONE!", 6);
// 	Delay(20000);
// 	LCD_clear();
// }
//
// double SENSOR_VoltageToDistance2(double voltage){
//   double m = 0;
//   double inv_distance;
//   double c = 0;
//   double distance;
//   double distances[5] = {1/30.0, 1/20.0, 1/15.0, 1/10.0, 1/5.0};               //X-axis = 1/distance                                                                      //
//                                                                               //Y-axis = V-out = sample voltage
//   if(voltage <= samples[0]){//80
//     distance = 30;
//   } else if (voltage > samples[4]){//5
//     distance = 5;
//   } else if (voltage >= samples[3] && voltage < samples[4]) {                 //Using Y = mX + c
//     m = (samples[4]-samples[3])/(distances[4]-distances[3]);                  //m = dY/dX
//     c = samples[4] - (distances[4] * m);                                      //c = Y - mX
//   }else if (voltage >= samples[2] && voltage < samples[3]) {
//     m = (samples[3]-samples[2])/(distances[3]-distances[2]);
//     c = samples[3] - (distances[3] * m);
//   }else if (voltage >= samples[1] && voltage < samples[2]) {
//     m = (samples[2]-samples[1])/(distances[2]-distances[1]);
//     c = samples[2] - (distances[2] * m);
//   }else if (voltage >= samples[0] && voltage < samples[1]) {
//     m = (samples[1]-samples[0])/(distances[1]-distances[0]);
//     c = samples[1] - (distances[1] * m);
//   }
//   if(m!=0){
//     inv_distance = (voltage-c)/m;                                             //(Y - c) / m = X
//     distance = 1/inv_distance;
//   }
//   return distance;
// }
//
// void IR_Func(void){
// 	int intensity_val = 0;
// 	SEGMENT_WriteHidden(0, 5, 1);
// 	while(1){
// 		char out[8];
//     sprintf(out, "IR: %dv\n\r", ADC_ChannelGetData(LPC_ADC, 1));
//     print_usb(out, 11);
// 		if ((int) ADC_ChannelGetData(LPC_ADC, 1) > 3750){
// 			intensity_val = 255;
// 		} else {
// 			intensity_val = ADC_ChannelGetData(LPC_ADC, 1) - 1200;
// 			if (intensity_val >= 0){
// 				intensity_val = intensity_val * 0.1;
// 			}
// 		}
// 		SEGMENT_WriteHidden(intensity_val, 5, 1);
// 	}
// }

void Save_Pattern_Using_IR(void){
	int interval;
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

uint8_t Get_Hue_From_IR(void){ //Loop required
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

void IC3(void){
	serial_init();
  Full_Init();
  LCD_clear();
  int seg_val = 0;
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

  // do {
  //   /*char out[8];
  //   sprintf(out, "Piezo: %dv | IR: %dv\n\r", ADC_ChannelGetData(LPC_ADC, 0), ADC_ChannelGetData(LPC_ADC, 1));
  //   print_usb(out, 27);*/
	//
  //   SEGMENT_WriteHidden(seg_val,5,1);
  //   read_i2c(read_buff,1,0x21);
  //   Delay(100000);
  // } while(1);//read_buff[0] != 0xF);
}

int main(void){
	IC3();
}

// uint16_t waveform_generator(double amplitude, double frequency){
//   double peaktopeak = ((double) 2) * amplitude;
//   int period = (int)(1000 * ((double) 1 / frequency));                        //Frequency in milliseconds,
//   double count = ((double) (SysTickCnt % period)) / ((double) period);
//   count = count * (double) 2 * PI;
//   double x = sin(count);
//   x = x + (double) 1;
//   x = x / (double) 2;
//   x = peaktopeak * x;
//   return (uint16_t) (x * (1023 / 3.33));
// }

double SENSOR_VoltageToDistance(double voltage){
  return (1.72955 * pow(voltage, 4)) - (19.0498 * pow(voltage, 3)) + (76.172 * pow(voltage, 2)) - (136.629 * voltage) + 104.683;
}


void SENSOR_Calibrate(double samples[]){                                      //Voltage proportional to distance throughout
  print("CALIBRATION MODE SELECTED\r\n");                                     //Will need to change to LCD instead of just print
  print("When prompted, place an object at the distance specified\r\n");      //""
  print("Then push any key on the keypad to continue\r\n");                   //""

  double raw_adc;
  double voltage;
  //uint8_t i;
  char output[20];

  print("80cm\r\n");
  KEYPAD_ReadKey();                                                           //When a key (Piezo?) pressed, then test data ready
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);                                   //Analogue signal from pin 16 converted to digital
  voltage = (raw_adc / 4096) * 3.3;                                           //Voltage level of pin 16 ((/ 4096) * 3.3)
  samples[0] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("40cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = (raw_adc / 4096) * 3.3;
  samples[1] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("10cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = (raw_adc / 4096) * 3.3;
  samples[2] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("7cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = (raw_adc / 4096) * 3.3;
  samples[3] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("5cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = (raw_adc / 4096) * 3.3;
  samples[4] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("DONE!\r\n");
}

double SENSOR_VoltageToDistance2(double samples[], double voltage){
  double m = 0;
  double inv_distance;
  double c = 0;
  double distance;
  double distances[5] = {1/80.0, 1/40.0, 1/10.0, 1/7.0, 1/5.0};               //X-axis = 1/distance                                                                      //
                                                                              //Y-axis = V-out = sample voltage
  if(voltage <= samples[0]){//80
    distance = 80;
  } else if (voltage > samples[4]){//5
    distance = 5;
  } else if (voltage >= samples[3] && voltage < samples[4]) {                 //Using Y = mX + c
    m = (samples[4]-samples[3])/(distances[4]-distances[3]);                  //m = dY/dX
    c = samples[4] - (distances[4] * m);                                      //c = Y - mX
  }else if (voltage >= samples[2] && voltage < samples[3]) {
    m = (samples[3]-samples[2])/(distances[3]-distances[2]);
    c = samples[3] - (distances[3] * m);
  }else if (voltage >= samples[1] && voltage < samples[2]) {
    m = (samples[2]-samples[1])/(distances[2]-distances[1]);
    c = samples[2] - (distances[2] * m);
  }else if (voltage >= samples[0] && voltage < samples[1]) {
    m = (samples[1]-samples[0])/(distances[1]-distances[0]);
    c = samples[1] - (distances[1] * m);
  }
  if(m!=0){
    inv_distance = (voltage-c)/m;                                             //(Y - c) / m = X
    distance = 1/inv_distance;
  }
  return distance;
}

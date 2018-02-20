//Address Defines for SAA1064
#define SAA1064_SA0 0x70
#define SAA1064_SA1 0x72
#define SAA1064_SA2 0x74
#define SAA1064_SA3 0x76

//Register Defines for SAA1064
#define SAA1064_CTRL 0x00
#define SAA1064_DIG1 0x01
#define SAA1064_DIG2 0x02
#define SAA1064_DIG3 0x03
#define SAA1064_DIG4 0x04

//Control Register Defines for SAA1064
//Static display (2 digits) or Multiplexed (4 digits)
#define SAA1064_MPX  0x01
//Digits 1 and 2 On
#define SAA1064_B0   0x02
//Digits 3 and 4 On
#define SAA1064_B1   0x04
//Intensity of display
#define SAA1064_INT0 0x00
#define SAA1064_INT1 0x10
#define SAA1064_INT2 0x20
#define SAA1064_INT3 0x30
#define SAA1064_INT4 0x40
#define SAA1064_INT5 0x50
#define SAA1064_INT6 0x60
#define SAA1064_INT7 0x70

//Default Mode: Multiplex On, All Digits On
#define SAA1064_CTRL_DEF (SAA1064_MPX | SAA1064_B0 | SAA1064_B1)

//Pin Defines for SAA1064
#define D_L0                 0x01
#define D_L1                 0x02
#define D_L2                 0x04
#define D_L3                 0x08
#define D_L4                 0x10
#define D_L5                 0x20
#define D_L6                 0x40
#define D_L7                 0x80

const uint8_t SAA1064_SEGM[] = {0x3F,0x06, 0x5B,0x4F,
                                0x66,0x6D,0x7D,0x07,
                                0x7F,0x6F,0x77,0x7C,
                                0x39,0x5E,0x79,0x71};

#define SAA1064_DP              0x80   //Decimal Point
#define SAA1064_MINUS           0x40   //Minus Sign
#define SAA1064_BLNK            0x00   //Blank Digit
#define SAA1064_ALL             0xFF   //All Segments On

#define EIGHT_SEG_ADDRESS 0x38

void init_SEGMENTS(){
  uint8_t data[6];

  data[0] = SAA1064_CTRL;                     // Select Control Reg
  data[1] = SAA1064_CTRL_DEF | SAA1064_INT3;  // Init Control Reg
  data[2] = SAA1064_BLNK;                     // Digit 1: All Segments Off
  data[3] = SAA1064_BLNK;                     // Digit 2: All Segments Off
  data[4] = SAA1064_BLNK;                     // Digit 3: All Segments Off
  data[5] = SAA1064_BLNK;                     // Digit 4: All Segments Off

  //data[2] = SAA1064_ALL;                      // Digit 1: All Segments On
  //data[3] = SAA1064_ALL;                      // Digit 2: All Segments On
  //data[4] = SAA1064_ALL;                      // Digit 3: All Segments On
  //data[5] = SAA1064_ALL;                      // Digit 4: All Segments On

  EL_I2C_SendBytes(EIGHT_SEG_ADDRESS, data, 6);
}

void SEGMENT_WriteHidden(int value, uint8_t dp_digit, int leading){
  uint8_t digit_value;
  uint8_t data[6];
  data[0] = SAA1064_DIG1;                     // Select Digit1 Reg

  // limit to valid range
  if (value >= 9999) value = 9999;
  if (value <= -999) value = -999;

  if (value >= 0) {
    // value 0...9999
    digit_value = value/1000; // compute thousands
    value = value % 1000;     // compute remainder
    if ((digit_value==0) && !(dp_digit==1) && leading )
      data[1] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[1] = SAA1064_SEGM[digit_value];
      leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==1) {data[1] = data[1] | SAA1064_DP;} // Set decimal point


    digit_value = value/100;  // compute hundreds
    value = value % 100;      // compute remainder
    if ((digit_value==0) && !(dp_digit==2) && leading)
      data[2] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[2] = SAA1064_SEGM[digit_value];
      leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==2) {data[2] = data[2] | SAA1064_DP;} // Set decimal point

    digit_value = value/10;   // compute tens
    value = value % 10;       // compute remainder
    if ((digit_value==0) && !(dp_digit==3) && leading)
      data[3] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[3] = SAA1064_SEGM[digit_value];
      //leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==3) {data[3] = data[3] | SAA1064_DP;} // Set decimal point

    //digit_value = value;      // compute units
    data[4] = SAA1064_SEGM[value];          // never suppress units zero
    if (dp_digit==4) {data[4] = data[4] | SAA1064_DP;} // Set decimal point

  }
  else {
    // value -999...-1
    value = -value;
    data[1] = SAA1064_MINUS;               // Sign
    if (dp_digit==1) {data[1] = data[1] | SAA1064_DP;} // Set decimal point

    digit_value = value/100;  // compute hundreds
    value = value % 100;      // compute remainder
    if ((digit_value==0) && !(dp_digit==2) && leading)
      data[2] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[2] = SAA1064_SEGM[digit_value];
      leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==2) {data[2] = data[2] | SAA1064_DP;} // Set decimal point

    digit_value = value/10;   // compute tens
    value = value % 10;       // compute remainder
    if ((digit_value==0) && !(dp_digit==3) && leading)
      data[3] = SAA1064_BLNK;               // suppress leading zero
    else {
      data[3] = SAA1064_SEGM[digit_value];
      //leading = 0;                      // dont suppress zero's
    }
    if (dp_digit==3) {data[3] = data[3] | SAA1064_DP;} // Set decimal point

    //digit_value = value;      // compute units
    data[4] = SAA1064_SEGM[value];          // never suppress units zero
    if (dp_digit==4) {data[4] = data[4] | SAA1064_DP;} // Set decimal point
  }

 EL_I2C_SendBytes(EIGHT_SEG_ADDRESS, data, 5);
}

void SEGMENT_Write(int int_value, int zeros){
  uint8_t dp_digit = 0;
  int leading = 1;

  if (zeros == 1){
    //leading is opposite to zeros
    leading = 0;
  }
  SEGMENT_WriteHidden(int_value, dp_digit, leading);
}

void SEGMENT_WriteFloat(double double_value, int zeros){
  double value;
  uint8_t no_of_digit = ceil(log10(double_value));
  uint8_t dp_digit = 0;
  int leading = 1;
  double decimals = double_value - (int)floor(double_value);

  if (zeros == 1){
    //leading is opposite to zeros
    leading = 0;
  }

  uint8_t dec_digit = 4 - no_of_digit;
  decimals = decimals * pow(10, dec_digit);
  double_value = double_value * pow(10, dec_digit);

  if(no_of_digit > 4 || no_of_digit < 0){
    print("ERROR. MAX VALUE EXCEEDED");
    value = 9999;
    dp_digit = 4;
    leading = 0;
  }

  value = double_value + decimals;

  SEGMENT_WriteHidden(value, dp_digit, leading);
}
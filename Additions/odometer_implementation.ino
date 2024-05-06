// Function for displaying distance traveled by the smart car on a 4x7-segment display
/*
  Displays potentiometer value on 4x7-segment display
  Written by Colin Cain
  AS OF 5-6-24 THIS CIRCUIT WORKS
  05-06-24
  I/O Pins
  A0: Potentiometer Input
  A1:
  A2:
  A3:
  A4: BCD D
  A5: BCD C
  D0: BCD B
  D1: BCD A
  D2:
  D3:
  D4:
  D5:
  D6:
  D7:
  D8:
  D9:
  D10:
  D11:
  D12: Mux B
  D13: Mux A
*/

// This is for proof of concept with potentiometer
// volatile unsigned int distTraveled;
  
void setup() {

  // Outputs
  DDRB = 0x18;
  DDRD = 0x03;
  DDRC = 0x30;

/*
  For proof of concept with potentiometer
  // ADC setup
  ADCSRA = 0xEF;
  ADCSRB = 0x00;
  ADMUX = 0x40;
*/ 
}



void loop() {
  // Declare array to hold each numeral place
  static unsigned char mux = 0;
  const unsigned char n = 3;
  unsigned char digit[n] = {((distTraveled / 100) % 10), ((distTraveled / 10) % 10), (distTraveled % 10)};

  odometer(digit[mux], mux); // INSERT INTO LOOP

  switch (mux)
  {
    case 0:
      PORTB &= 0xE7;
      break;

    case 1:
      PORTB &= 0xE7;
      PORTB |= 0x08;
      break;

    case 2:
      PORTB &= 0xE7;
      PORTB |= 0x10;
      break;
  }

  if (mux == 2)
  {
    mux = 0;
  }
  else
  {
    mux++;
  }
  
  _delay_ms(5);    // Small delay for legibility, uncomment if needed
}

void odometer(unsigned char numeral, unsigned char muxVal) {

  // Unable to clear display prior to write, hoping direct assignment works
  // Clear and set PORTD pins
  PORTD &= 0xFC;
  PORTD |= (numeral >> 2);

  // Clear and set PORTC pins
  PORTC &= 0xCF;
  PORTC |= ((numeral & 0x03) << 4);
  
}

/*
// For proof of concept with potentiometer
ISR(ADC_vect) {
  distTraveled = ADC;
}
*/

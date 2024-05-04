// Code to get potentiometer value onto 7-segment x4
/*
  Displays potentiometer value on 4x7-segment display (ELEGOO Unit)
  Written by Colin Cain
  05-02-24
  I/O Pins
  A0:
  A1:
  A2:
  A3:
  A4: Potentiometer input
  A5:
  D0:
  D1:
  D2:
  D3:
  D4:
  D5: Cathode 3 (100s)
  D6: Cathode 2 (10s)
  D7: Cathode 1 (1s)
  D8:
  D9:
  D10: SS out (74595 Pin 12)
  D11: Serial out (74595 Pin 14)
  D12:
  D13: CLK out (74595 Pin 11)
*/

volatile unsigned int potVal;

void setup() {

  // ADC setup
  ADCSRA = 0xEF;
  ADCSRB = 0x00;
  ADMUX = 0x44;

  // Assert outputs
  DDRB = 0x2C;
  DDRD = 0xE0;
  PORTB |= 0x04;

  cli();
  // SPCR config
  SPCR = 0x70;
  sei();
}

void loop() {

  // Declare an array to hold 7-seg vlaues 0-9
  unsigned char hexToNum[10] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6};

  // Declare array to hold each numeral place
  const unsigned char n = 3;
  unsigned char digit[n] = {((potVal / 100) % 10), ((potVal / 10) % 10), (potVal % 10)};

  for (unsigned char i = 0; i < 3; i++) {

    // Call the spiTx function with the digit and i value
    SPITx(hexToNum[digit[i]], i);
    // Small delay for legibility
    _delay_ms(5);
  }
}

ISR(ADC_vect) {
  potVal = ADC;
}

void SPITx(unsigned char numeral, unsigned char j) {


  PORTB &= 0xFB; // enable SPI write
  PORTD |= 0xE0; // Set all cathodes high
  // switch case to determine numeral place
  switch (j) {
    case 0:
      // Enable hundreds place
      PORTD &= 0x7F;
      break;
    case 1:
      // Enable tens place
      PORTD &= 0xBF;
      break;
    case 2:
      // Enable ones place
      PORTD &= 0xDF;
      break;
  }

  SPDR = numeral;
  while (!(SPSR & (1 << SPIF)));
  PORTB |= 0x04; // disable SPI write
}

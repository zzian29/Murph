// Function for displaying distance traveled by the smart car on a 4x7-segment display
/*
  Displays potentiometer value on 4x7-segment display
  Written by Colin Cain
  05-04-24
  I/O Pins
  A0:
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
  D12:
  D13:
*/

void setup() {
}



void loop() {
  // Declare array to hold each numeral place
  const unsigned char n = 3;
  unsigned char digit[n] = {((potVal / 100) % 10), ((potVal / 10) % 10), (potVal % 10)};

  odometer(digit[mux]); // INSERT INTO MUX LOOP
  // _delay_ms(5);    // Small delay for legibility, uncomment if needed
}

void odometer(unsigned char numeral) {

  // Unable to clear display prior to write, hoping direct assignment works
  // Clear and set PORTD pins 
  PORTD &= 0xFC;
  PORTD |= (numeral & 0x03);

  // Clear and set PORTC pins
  PORTC &= 0xCF;
  PORTC |= ((numeral & 0x0C) << 4);
}

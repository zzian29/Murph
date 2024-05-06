/*
  Line follower demo FAST
  By: Ian Zentner, Colin Cain, Cole Shoemaker
  Written: April 18th, 2024
  Edited: April 26th, 2024
  I/O Pins
  A0: Left wheel encoder
  A1: Left sensor
  A2: Center sensor
  A3: Right sensor
  A4: BCD D
  A5: BCD C
  D0: BCD B
  D1: BCD A
  D2:
  D3: Motor L OC2B
  D4:
  D5: Motor R OCOB
  D6: Motor R OCOA
  D7: Right wheel encoder
  D8: LED
  D9:
  D10:
  D11: Motor L OC2A
  D12: Mux B
  D13: Mux A
*/

// ADC working, ADC values typically range from 950-1000 on black,
// and 650-800 for white
volatile unsigned int lineLeft = 0;
volatile unsigned int lineRight = 0;
volatile unsigned int lineCenter = 0;

volatile unsigned int sumR = 0;
volatile unsigned int motorR = 0;
volatile unsigned int sumL = 0;
volatile unsigned int motorL = 0;

void setup() {

  cli();
  // Used to test the line sensor values
  // The tests we used consist of holding the each sensor over the line
  // and seeing what value the line sensor reports when over the black line vs
  // over the white background
  // Serial.begin(9600);
  //setting OC2A as output, and mux for odo
  DDRB = 0x39;
  // setting OCOA, OCOB, OC2B as outputs (remaining PWM pins, and out for odo
  DDRD = 0x6B;
  // Set PORTC outputs for odo
  DDRC = 0x30;
  // Sets LEDS on
  PORTB |= 0x01;
  // Set internal pull up on A0
  PORTC |= 0x01;
  // Set internal pull up on D7
  PORTD |= 0x80;
  // OCOB CLEAR AT TOP
  TCCR0A = 0xA1;
  // Prescaler of 1
  TCCR0B = 0x01;
  // OC2B CLEAR AT TOP
  TCCR2A = 0xA1;
  // Prescaler of 1
  TCCR2B = 0x01;
  // ADC in 10 bit mode, 128 prescaler, non-freerunning mode
  // and initially looking at pin A1
  ADCSRA = 0xCF;
  ADCSRB = 0x00;
  ADMUX = 0x41;
  // Pin change interrupts for pin A0 and D7
  PCICR = 0x06;   // Enable PC interrupts on the C and D ports
  PCMSK2 = 0x80;  // Enable PC interrupt on pin D7
  PCMSK1 = 0x01;  // Enable PC interrupt on pin A0

  sei();
}

void loop() {
  // We determined that the ADC value corresponding to a sensor being over the black line
  // is around 900, so by using 850 as a safety net incase of inconsistent lighting, we check
  // to see if the center sensor is over the black line, and if not, it will compare the side
  // sensor values and determine which side sensor is over the black line
  // Serial.println(lineRight);
  unsigned int average = (motorL + motorR) / 2;
  unsigned char distTraveled = ((average * 2042L) / 19200);

  // Mux for cycling displays
  static unsigned char mux = 0;

  // Array for distTraveled numerals
  const unsigned char n = 3;
  unsigned char digit[n] = {((distTraveled / 100) % 10), ((distTraveled / 10) % 10), (distTraveled % 10)};

  // odometer function call
  odometer(digit[mux], mux);
  // Delay for legibility on display

  // Switch case to assert ports and if statement to cycle the mux values 0-2
  switch (mux)
  {
    case 0:
      PORTB &= 0xCF;
      break;

    case 1:
      PORTB &= 0xCF;
      PORTB |= 0x10;
      break;

    case 2:
      PORTB &= 0xCF;
      PORTB |= 0x20;
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

  // Delay for legibility on display
  _delay_ms(5);
  
  if ((lineCenter > 850))
  {
    OCR0A = 0;
    OCR2A = 0;
    OCR0B = 120;
    OCR2B = 120;
  }
  else if (lineRight > 850)
  {
    OCR0A = 30;
    OCR2A = 0;
    OCR0B = 0;
    OCR2B = 135;
  }
  else if (lineLeft > 850)
  {
    OCR0A = 0;
    OCR2A = 30;
    OCR0B = 135;
    OCR2B = 0;
  }
  // Personal note to remember which OCxx register corresponds to which motor
  // L is OCR2B
  // R is OCR0B
  else if (lineCenter < 900 && lineRight < 900 && lineLeft < 900 && distTraveled >= 190)
  {
    // so that the wheels get torqued in the opposite direction to stop the car faster
    OCR0A = 70;
    OCR2A = 70;
    OCR0B = 0;
    OCR2B = 0;
    _delay_ms(10);
    OCR0A = 0;
    OCR2A = 0;
  }
  else {}
}

ISR(ADC_vect)
{
  // The ADC ISR starts with looking at the previous ADMUX value set,
  // then during the ISR the next ADMUX value gets set
  if ((ADMUX & 0x0F) == 0x01)
  {
    lineLeft = ADC;
    ADMUX = 0x42;
  }
  else if ((ADMUX & 0x0F) == 0x02)
  {
    lineCenter = ADC;
    ADMUX = 0x43;
  }
  else
  {
    lineRight = ADC;
    ADMUX = 0x41;
  }
  ADCSRA |= 0x40;
}
// Both of ISR's are used for the wheel encoders
// They function the same, only changing the variables corresponding to the wheel encoder
// being tracked. The conditional logic is used to determine if the wheel encoder has ticked
// an odd or even amount of times. If the wheel encoder has ticked an odd amount of times, then we just
// divide the number of ticks by 2 and add 1 to the total because of how division on the arduino works.
// (3/2) = 1 on arduino
// If the number is even, we just divide by 2, as there are equal amounts of rising and falling edges.
//
// This one specifically is for the right wheel encoder
ISR(PCINT2_vect)
{
  sumR++;
  if (sumR % 2)
  {
    motorR = (sumR / 2) + 1;
  }
  else
  {
    motorR = sumR / 2;
  }
}
// Left wheel encoder ISR
ISR(PCINT1_vect)
{
  sumL++;
  if (sumL % 2)
  {
    motorL = (sumL / 2) + 1;
  }
  else
  {
    motorL = sumL / 2;
  }
}

// odometer function to write to display
void odometer(unsigned char numeral, unsigned char muxVal) {

  // Unable to clear display prior to write, hoping direct assignment works
  // Clear and set PORTD pins
  PORTD &= 0xFC;
  PORTD |= (numeral >> 2);

  // Clear and set PORTC pins
  PORTC &= 0xCF;
  PORTC |= ((numeral & 0x03) << 4);

}

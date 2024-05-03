/*
  Barrier Demo
  By: Ian Zentner, Colin Cain, and Cole Shoemaker
  Written: March 19th, 2024
  Edited: April 3rd, 2024, by Cole Shoemaker
  Revised: April 11th, by Ian Zentner
  I/O Pins
  A0:
  A1:
  A2:
  A3:
  A4:
  A5:
  D0:
  D1:
  D2:
  D3: Motor L OC2B
  D4:
  D5: Motor R OCOB
  D6: Motor R OCOA
  D7:
  D8: ECHO from ultrasonic sensor MUX
  D9: Trigger to ultrasonic sensors (they all share the same trigger)
  D10:
  D11: Motor L OC2A
  D12: A mux
  D13: B mux
*/

/* 4/11/24
  I removed the ADC configurations, the ADC ISR, all
  LCD screen configurations/functions, and updated header.
*/
//LED's are left as that as I don't remember which one corresponds to a left
//turn vs a right turn. A lot of this code is done on the fly and could use a ton
//of improvements so please feel free to critique so we can make stronger code.
//That being said, the code does work. Work in a sense that we get accurate values,
//pretty consistently, but we still have random fluxuations in values from time to
//time.


//This code starts the same way lab 12 circuit 3 does, as I
//used that circuit as the foundation for making this one
//global variables used by the ISR to calculate the time elapsed between echo
//

volatile unsigned char sregValue;
volatile unsigned long count = 0;
volatile unsigned long countStop = 0;
volatile unsigned long capt[2];
volatile unsigned int sumR = 0;
volatile unsigned int motorR = 0;
volatile unsigned int sumL = 0;
volatile unsigned int motorL = 0;

void setup() {

  cli();

  //configured the input capture unit on TCNT1
  TCCR1A = 0x00;
  //Input caputre noise canceler & rising edge select for ICU
  //Prescaler of 256, this value should be changed and tested to see
  //if results vary
  TCCR1B = 0xC4;
  //enabled input capture unit and TCNT1 overflow interrupt enable
  TIMSK1 = 0x21;

  //setting outputs mux, motor control, and trigger pins all have locations in B
  DDRB = 0x3A;

  // OCOA & OCOA CLEAR AT TOP
  TCCR0A = 0xA1;
  // Prescaler of 1
  TCCR0B = 0x01;

  // OCOA & OCOA CLEAR AT TOP
  TCCR2A = 0xA1;
  // Prescaler of 1
  TCCR2B = 0x01;
  Serial.begin(9600);
  // setting OCOA, OCOB, OC2B as outputs (remaining PWM pins)
  DDRD = 0x68;

  PCICR = 0x06;   // Enable PC interrupts on the C and D ports
  PCMSK2 = 0x80;  // Enable PC interrupt on pin D7
  PCMSK1 = 0x01;  // Enable PC interrupt on pin A0

  sei();
}

void loop() {

  //3 individual distance values, each corresponding to their own
  //ultrasonic sensor, they're static because during a single iteration
  //of the loop, only one distance is being updated, as the mux variable
  //iterates with each instance of the loop
  static unsigned int distanceR = 0;
  static unsigned int distanceC = 0;
  static unsigned int distanceL = 0;
  static unsigned int oldDistanceR = 0;
  static unsigned int oldDistanceC = 0;
  static unsigned int oldDistanceL = 0;
  unsigned int average = (motorL + motorR) / 2;
  unsigned char distTraveled = ((average * 2042L) / 19200);
  static unsigned char oldDist = 0;
  
  if (distTraveled > oldDist)
  {
    countStop = 0;
    oldDist = distTraveled;
  }
  if (countStop > 3276750)
  {
    OCR0A = 80;
    OCR2A = 80;
    OCR0B = 0;
    OCR2B = 0;
    _delay_ms(1000);
  }

  cli();

  //10 us pulse to trigger on all of the ultrasonic sensors


  sei();

  //mux variable is used to keep track of which ultrasonic sensor
  //should be communicating to the arduino during the loop
  static unsigned char mux = 0;
  trigger();
  //used to update the distance variable depending
  //on which ultrasonic sensor is echoing to the arduino
  switch (mux)
  {
    case 0:
      PORTB &= 0xCF;
      distanceR = distanceCalc(oldDistanceR);
      oldDistanceR = distanceR;
      break;

    case 1:
      PORTB &= 0xCF;
      PORTB |= 0x10;
      distanceC = distanceCalc(oldDistanceC);
      oldDistanceC = distanceC;
      break;

    case 2:
      PORTB &= 0xCF;
      PORTB |= 0x20;
      distanceL = distanceCalc(oldDistanceL);
      oldDistanceL = distanceL;
      break;
  }

  if (distanceC < 290)
  {
    if (distanceR > distanceL)
    {
      OCR0B = 90;
      OCR2B = 0;
      OCR0A = 0;
      OCR2A = 90;
    }
    else
    {
      OCR0B = 0;
      OCR2B = 90;
      OCR0A = 30;
      OCR2A = 0;
    }
  }
  else {
    if (distanceR > distanceL)
    {
      OCR0B = 110;
      OCR2B = 40;
      OCR0A = 0;
      OCR2A = 0;
    }
    else if (distanceL < distanceR)
    {
      OCR0B = 40;
      OCR2B = 110;
      OCR0A = 0;
      OCR2A = 0;
    }
    else {
      OCR0B = 100;
      OCR2B = 100;
    }
  }

  //our mux is the 74153 4-1 mux, in this circuit we only use 3 inputs on the mux
  //so our mux value should never be greater than 2, so when it is 2, before the end of the loop
  //instead of going to 3, it gets set to 0
  if (mux == 2)
  {
    mux = 0;
  }
  else
  {
    mux++;
  }
  _delay_ms(60);


  //Serial.println(distanceC);
  // Serial.println(distanceR);
  //Serial.println(distanceL);
}

/*
   Interrupt Service Routines
  ------------------------------------------------------------------------------------------------
*/

//this flips the edge that the ICU ISR triggers on so
//that we can calculate the time between peaks
ISR(TIMER1_CAPT_vect)
{
  sregValue = SREG;
  if (TCCR1B & 0x40)
  {
    capt[0] = count + ICR1;
    TCCR1B &= 0xBF;
  }
  else
  {
    capt[1] = count + ICR1;
    TCCR1B |= 0x40;
  }
  SREG = sregValue;
}


//if timer overflows, add the total time counted to the count
ISR(TIMER1_OVF_vect)
{
  sregValue = SREG;
  count += 65536;
  countStop += 65336;
  SREG = sregValue;
}

/*
   External Functions
   -----------------------------------------------------------------------------------
*/

//this is used to calculate the current distance for the sensor being updated
unsigned int distanceCalc(unsigned int oldDistance)
{

  //variables to store ticks elapsed and the high duration of the duty cycle
  unsigned long tksElapsed = 0;
  unsigned long timeHigh = 0;

  if (capt[1] > capt[0])
  {
    tksElapsed = capt[1] - capt[0];
    //calculating time high
    timeHigh = tksElapsed * 16;
  }
  //equation provided in lab 12, circuit 3
  unsigned int distance = (timeHigh * 17182L) / 100000;

  if (!distance || distance == 8)
  {
    distance = oldDistance;
  }

  return distance;
}

void trigger()
{
  PORTB |= 0x02;
  _delay_us(10);
  PORTB &= 0xFD;
}
// Right wheel encoder ISR
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

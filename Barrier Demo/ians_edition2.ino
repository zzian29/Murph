/*
  Barrier Demo
  By: Ian Zentner, Colin Cain, and Cole Shoemaker
  Written: March 19th, 2024
  Edited: April 3rd, 2024, by Cole Shoemaker
  Revised: May 3rd, by Ian Zentner
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
// Speed constants (for OCR0B and 2B
unsigned char regSpeed = 100;

// Variables for ultrasonic capture
volatile unsigned char sregValue;
volatile unsigned long count = 0;
volatile unsigned long capt[2];


// Variables for wheel encoders
volatile unsigned int sumR = 0;
volatile unsigned int motorR = 0;
volatile unsigned int sumL = 0;
volatile unsigned int motorL = 0;


// PI Control
// Error History array size
const char tau = 50;
// Setpoint value
const int sP = 100;
// Constants for correction algorithm
const char Kp = 1;
const char Ki = 4;
// Variables for error calculations
volatile unsigned int ratioLR;    //avg of left and right sensors, unsigned since distances are unsigned
volatile int err;   // avg error
volatile int errHist[tau];  //history of average error
volatile int errSum;


void setup() {

  cli();
  //configured the input capture unit on TCNT1
  //Input caputre noise canceler & rising edge select for ICU
  //Prescaler of 256, this value should be changed and tested to see if results vary
  //enabled input capture unit and TCNT1 overflow interrupt enable
  TCCR1A = 0x00;
  TCCR1B = 0xC4;
  TIMSK1 = 0x21;

  DDRB = 0x3A;  //setting outputs mux, motor control, and trigger pins all have locations in B
  DDRD = 0x68;  // setting OCOA, OCOB, OC2B as outputs (remaining PWM pins)

  // OCOA & OCOA CLEAR AT TOP, Prescaler of 1
  TCCR0A = 0xA1;
  TCCR0B = 0x01;

  // OCOA & OCOA CLEAR AT TOP, Prescaler of 1
  TCCR2A = 0xA1;
  TCCR2B = 0x01;

  PCICR = 0x06;   // Enable PC interrupts on the C and D ports
  PCMSK2 = 0x80;  // Enable PC interrupt on pin D7
  PCMSK1 = 0x01;  // Enable PC interrupt on pin A0

  PORTC |= 0x01;
  PORTD |= 0x80;

  sei();

  Serial.begin(9600); // For testing, comment out as needed
}


void loop() {

  //3 individual distance values, each corresponding to their own
  //ultrasonic sensor, they're static because during a single iteration
  //of the loop, only one distance is being updated, as the mux variable
  //iterates with each instance of the loop
  static unsigned int distR = 0;
  static unsigned int distF = 0;
  static unsigned int distL = 0;
  static unsigned int oldDistR = 0;
  static unsigned int oldDistF = 0;
  static unsigned int oldDistL = 0;

  unsigned int average = (motorL + motorR) / 2;               // Calculating distance traveled based on avg calculation of wheel encoders
  unsigned char distTraveled = ((average * 2042L) / 19200);

  //mux variable is used to keep track of which ultrasonic sensor
  //should be communicating to the arduino during the loop
  static unsigned char mux = 0;

  trigger();

  //used to update the distance variable depending
  //on which ultrasonic sensor is echoing to the arduino
  //only one of three conditions run during each loop
  if (mux == 0)
  {
    PORTB &= 0xCF;    // Clear mux select pins
    distL = distanceCalc(oldDistL);
    oldDistL = distL;
    mux++;
  }
  else if (mux == 1)
  {
    PORTB &= 0xCF;    // Clear mux select pins
    PORTB |= 0x10;    // D12 HIGH
    distF = distanceCalc(oldDistF);
    oldDistF = distF;
    mux++;
  }
  else if (mux == 2)
  {
    PORTB &= 0xCF;    // Clear mux select pins
    PORTB |= 0x20;    // D13 HIGH
    distR = distanceCalc(oldDistR);
    oldDistR = distR;
    mux = 0;
  }

  ratioLR = (distR * 100) / distL;


  if (distR < distL)
  {
    OCR0B += ((Kp * err) / 10);
    OCR0B += ((Ki * errSum) / 100);
    //    OCR2A += (((Kp * err) / 20));
    //    OCR2A += ((Ki * errSum) / 200);

  }

  else if (distL < distR)
  {
    OCR2B -= (((Kp * err) / 10));
    OCR2B -= ((Ki * errSum) / 100);
    //    OCR0A += (((Kp * err) / 20));
    //    OCR0A += ((Ki * errSum) / 200);
  }
  else
  {
    OCR0B = 90;
    OCR2B = 90;
    OCR0A = 0;
    OCR2A = 0;
  }

//  if (distR < distL)
//  {
//    OCR0B = 0;
//    OCR2B = 120;
//    //    OCR2A += (((Kp * err) / 20));
//    //    OCR2A += ((Ki * errSum) / 200);
//
//  }
//
//  else if (distL < distR)
//  {
//    OCR2B = 0;
//    OCR0B = 120;
//    //    OCR0A += (((Kp * err) / 20));
//    //    OCR0A += ((Ki * errSum) / 200);
//  }
//  else
//  {
//    OCR0B = 90;
//    OCR2B = 90;
//    OCR0A = 0;
//    OCR2A = 0;
//  }



  //_delay_ms(10);



  // For testing purposes, comment in and out as needed
  //------------------------------------------------------
  Serial.print("distF: ");
  Serial.print(distF);
  Serial.println(" cm");
  Serial.print("distR: ");
  Serial.print(distR);
  Serial.println(" cm");
  Serial.print("distL: ");
  Serial.print(distL);
  Serial.println(" cm");
  Serial.print("error: ");
  Serial.println(err);




  // Calibrate setpoint (Comment out as needed once complete)
  // -----------------------------------------------------------
  // This block prints a table of distances L and R as well as the average to give a numerical analysis of the maze parameters

  //  Serial.print("distL: ");
  //  Serial.print(distL);
  //  Serial.print("cm  |");
  //  Serial.print("distR: ");
  //  Serial.print(distR);
  //  Serial.print("cm  |");
  //  Serial.print("Avg: ");
  //  Serial.print(ratioLR);
  //  Serial.println("cm");


  // This block prints a graph on the serial plotter displaying a qualitative analysis of the system's oscillations/feedback to gauge sP accuracy
  //  Serial.print(ratioLR);
  //  Serial.print('\t');
  //  Serial.println(sP);

}


// ---------------------------------------------------------------------------
// Ultrasonic sensor subsystem
// ---------------------------------------------------------------------------

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


  errorCalc(ratioLR);                 // PI control error calculations occur each time a new sensor updates
  errSum = histSum(errHist, tau);   // Take error history from both side sensors
}


//if timer overflows, add the total time counted to the count
ISR(TIMER1_OVF_vect)
{
  sregValue = SREG;
  count += 65536;
  SREG = sregValue;
}


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


void trigger()      // Sends 10us pulse to all HC-SR04 trigger pins (daisy chain from D9)
{
  PORTB |= 0x02;
  _delay_us(10);
  PORTB &= 0xFD;
}



// ---------------------------------------------------------------------------
// Wheel encoder subsystem
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// PI Control subsystem
// ---------------------------------------------------------------------------

// Function to take current error of the avgLR
void errorCalc(unsigned int ratioLR)
{
  static unsigned char x = 0;
  err = sP - ratioLR;
  errHist[x] = err;
  if (x == tau)
    x = 0;
  else
    x++;
}

// Function that takes sum of error history from the arrays
int histSum(int errArr[], int arrSize)
{
  int tempVar = 0;
  for (unsigned char j = 0; j < arrSize; j++)
  {
    tempVar += errArr[j];
  }
  return tempVar;
}

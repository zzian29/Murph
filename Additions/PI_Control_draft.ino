// Cole Garrett Shoemaker
// PI Control for the smart car with the input and setpoints based on HC-SR04 sensors on left and right

// 04/25/24

// Error History array size
const char tau = 50;
// Setpoint value 
const int sP = ;
// Variables for LEFT sensor & motor
volatile int pVL;
volatile int errL;
volatile int errHistL[tau];

// Variables for RIGHT sensor & motor
volatile int pVR;
volatile int errR;
volatile int errHistR[tau];



void setup() {
  cli();

  ADCSRA = 0xEF;
  ADCSRB = 0x00;
  ADMUX = 0x64;

  TCCR0A = 0x83;
  TCCR0B = 0x03;

  // PC interrupt on Port B to update pV values when D8 is updated
  PCICR = 0x01;  // Port B
  PCMSK = 0x01;  // Pin D8

  sei();
}

void loop() {

  // Constants for correction algorithm
  char Kp = 0.1;
  char Ki = 0.001;

  int histSum = sum(errHistR, tau);
  int histSum = sum(errHistL, tau);

  while(distR != sP)
  {
    OCR0B += ((Kp * errL) / 10);
    OCR0B += (Ki * histSum);
  }

  while(distL != sP)
  {
    OCR2B += ((Kp * errL) / 10);
    OCR2B += (Ki * histSum);
  }
}


// Function to take history of error array
int histSum(int errArr[], int arrSize)
{
  int tempVar = 0;
  for (unsigned char j = 0; j < arrSize; j++)
  {
    tempVar += arr[j];
  }
  return tempVar;
}


// ISR to read the current value of a distance
ISR(PCINT3_vect)
{
  static unsigned char sensors = 0;
  switch (sensors)
  {
    case 0:
      static unsigned char x = 0;
      pVL = distanceL;
      errL = sP - pVL;
      errHistL[x] = errL;
      if (x == tau)
        x = 0;
      else
        x++;
      break;
    case 1:
      static unsigned char y = 0;
      pVR = distanceR;
      errR = sP - pVR;
      errHistR[y] = errR;
      if (y == tau)
        y = 0;
      else
        y++;
      break;

      sensors++;
      if(sensors == 2)
      sensors = 0;
  }
}

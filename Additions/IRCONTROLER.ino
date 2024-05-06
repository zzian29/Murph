volatile unsigned char sregValue = 0;
volatile unsigned long count = 0;
volatile unsigned long ticks[2];
volatile unsigned char REMOTE = 0;

void setup() {
  cli();
  //TCNT1's configuration with both the barrier demo and the line follower
  TCCR1A = 0x00;
  TCCR1B = 0xC4;
  TIMSK1 = 0x21;
  // external interrupt triggers on falling edges
  EICRA = 0x01;
  EIMSK = 0x01;
  DDRD = 0x80;
  Serial.begin(9600);
  sei();
}

void loop()
{
  Serial.println(REMOTE);
}

ISR(TIMER1_OVF_vect)
{
  sregValue = SREG;
  count += 65536;
  SREG = sregValue;
}

ISR(INT0_vect)
{
  sregValue = SREG;
  unsigned int ticksElapsed = 0;
  static unsigned char x = 0;
  if (!x)
  {
    ticks[0] = TCNT1 + count;
  }
  else
  {
    ticks[1] = TCNT1 + count;
  }
  if (ticks[0] > ticks[1])
  {
    ticksElapsed = ticks[0] - ticks[1];
  }
  else
  {
    ticksElapsed = ticks[1] - ticks[0];
  }
  if (ticksElapsed > 260 && ticksElapsed < 285)
  {
    PORTD ^= 0x80;
    REMOTE ^= 0x01;
  }
  x ^= 1;
  SREG = sregValue;
}

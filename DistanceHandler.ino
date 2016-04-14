#define Plugin_013_TRIG_Pin1 TRIGGER_PIN
#define Plugin_013_IRQ_Pin1  ECHO_PIN

#define Plugin_013_TRIG_Pin2 TRIGGER_PIN2
#define Plugin_013_IRQ_Pin2  ECHO_PIN2

const uint8_t TRIG[] = {Plugin_013_TRIG_Pin1, Plugin_013_TRIG_Pin2};
const uint8_t IRQ[] = {Plugin_013_IRQ_Pin1, Plugin_013_IRQ_Pin2};

#define NO_ECHO -1
#define PING_MEDIAN_DELAY 29000

volatile unsigned long Plugin_013_timer = 0;
volatile unsigned long Plugin_013_state = 0;

/*********************************************************************/
float Plugin_013_read(byte id)
/*********************************************************************/
{
  float value = NO_ECHO;
  Plugin_013_timer = 0;
  Plugin_013_state = 0;
  noInterrupts();
  digitalWrite(TRIG[id], LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG[id], HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG[id], LOW);
  interrupts();

  //for (int i=0; i<27 && Plugin_013_state != 2; i++)
  //  delay(1);
    
  delay(29);  // wait for measurement to finish (max 500 cm * 58 uSec = 29uSec)
  if (Plugin_013_state == 2)
  {
    value = (float) Plugin_013_timer / 58;
  } 
      
  return value;
}

/*********************************************************************/
void ICACHE_RAM_ATTR Plugin_013_interrupt1()
/*********************************************************************/
{
  if (digitalRead(Plugin_013_IRQ_Pin1) == 1) // Start of pulse
  {
    Plugin_013_timer = micros();
    Plugin_013_state = 1;
  }
  else // End of pulse, calculate timelapse between start & end
  {
    Plugin_013_timer = micros() - Plugin_013_timer;
    Plugin_013_state = 2;
  }
}

/*********************************************************************/
void ICACHE_RAM_ATTR Plugin_013_interrupt2()
/*********************************************************************/
{
  if (digitalRead(Plugin_013_IRQ_Pin2) == 1) // Start of pulse
  {
    Plugin_013_timer = micros();
    Plugin_013_state = 1;
  }
  else // End of pulse, calculate timelapse between start & end
  {
    Plugin_013_timer = micros() - Plugin_013_timer;
    Plugin_013_state = 2;
  }
}

/*********************************************************************/
void Plugin_013_Init()
/*********************************************************************/
{
  pinMode(Plugin_013_TRIG_Pin1, OUTPUT);
  pinMode(Plugin_013_IRQ_Pin1, INPUT_PULLUP);
  attachInterrupt(Plugin_013_IRQ_Pin1, Plugin_013_interrupt1, CHANGE);

  pinMode(Plugin_013_TRIG_Pin2, OUTPUT);
  pinMode(Plugin_013_IRQ_Pin2, INPUT_PULLUP);
  attachInterrupt(Plugin_013_IRQ_Pin2, Plugin_013_interrupt2, CHANGE);  
}

float Plugin_013_Median(byte id, uint32_t it) {
  float uS[it], last;
  uint32_t j, i = 0;
  unsigned long t;
  uS[0] = NO_ECHO;

  while (i < it) {
    t = micros();  // Start ping timestamp.
    last = Plugin_013_read(id); // Send ping.

    if (last != NO_ECHO) {       // Ping in range, include as part of median.
      if (i > 0) {               // Don't start sort till second ping.
        for (j = i; j > 0 && uS[j - 1] < last; j--) // Insertion sort loop.
          uS[j] = uS[j - 1];     // Shift ping array to correct position for sort insertion.
      } else j = 0;              // First ping is sort starting point.
      uS[j] = last;              // Add last ping to array in sorted position.
      i++;                       // Move to next ping.
    } else it--;                 // Ping out of range, skip and don't include as part of median.

    if (i < it && micros() - t < PING_MEDIAN_DELAY)
      delay((PING_MEDIAN_DELAY + t - micros()) / 1000); // Millisecond delay between pings.

  }
  return (uS[it >> 1]); // Return the ping distance median.
}

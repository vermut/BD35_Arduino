#define Plugin_013_TRIG_Pin TRIGGER_PIN
#define Plugin_013_IRQ_Pin  ECHO_PIN

#define NO_ECHO -1
#define PING_MEDIAN_DELAY 29000

volatile unsigned long Plugin_013_timer = 0;
volatile unsigned long Plugin_013_state = 0;

/*********************************************************************/
int Plugin_013_read()
/*********************************************************************/
{
  int value = NO_ECHO;
  Plugin_013_timer = 0;
  Plugin_013_state = 0;
  noInterrupts();
  digitalWrite(Plugin_013_TRIG_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(Plugin_013_TRIG_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(Plugin_013_TRIG_Pin, LOW);
  interrupts();

  for (int i=0; i<7 && Plugin_013_state != 2; i++)
    delay(5);
    
  // delay(29);  // wait for measurement to finish (max 500 cm * 58 uSec = 29uSec)
  if (Plugin_013_state == 2)
  {
    value = Plugin_013_timer / 58;
  } else
    Serial.println("Failed to wait");
    
  return value;
}

/*********************************************************************/
void Plugin_013_interrupt()
/*********************************************************************/
{
  byte pinState = digitalRead(Plugin_013_IRQ_Pin);
  if (pinState == 1) // Start of pulse
  {
    Plugin_013_state = 1;
    Plugin_013_timer = micros();
  }
  else // End of pulse, calculate timelapse between start & end
  {
    Plugin_013_state = 2;
    Plugin_013_timer = micros() - Plugin_013_timer;
  }
}

/*********************************************************************/
void Plugin_013_Init()
/*********************************************************************/
{
  pinMode(Plugin_013_TRIG_Pin, OUTPUT);
  pinMode(Plugin_013_IRQ_Pin, INPUT_PULLUP);
  attachInterrupt(Plugin_013_IRQ_Pin, Plugin_013_interrupt, CHANGE);
}

int Plugin_013_Median(uint32_t it) {
  int uS[it], last;
  uint32_t j, i = 0;
  unsigned long t;
  uS[0] = NO_ECHO;

  while (i < it) {
    t = micros();  // Start ping timestamp.
    last = Plugin_013_read(); // Send ping.

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

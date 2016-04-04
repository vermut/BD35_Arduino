void net_register(float medianDistance);
void net_setup();
void net_report(int lastVal);

#define VCC          D4
#define TRIGGER_PIN  D3  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     D2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define GND          D1

ADC_MODE(ADC_VCC);
int startUpDistance = -1;
long nextReport;

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  Serial.setDebugOutput(true);
  net_setup();

  // Power setup
  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, HIGH);
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);

  Plugin_013_Init();
  startUpDistance = Plugin_013_Median(20);

  Serial.print("Startup: ");
  Serial.print(startUpDistance);
  Serial.println("cm");

  net_register(startUpDistance);
  net_report(startUpDistance);
  resetNextReport();
}

void loop() {
  delay(15);
  int curVal = Plugin_013_read();
  if (nonStartUpDistance(curVal)) {
    int nextVal[10];
    nextVal[0] = curVal;
    int i = 1;
    for (; i < 10; i++)    {
      delay(15);
      nextVal[i] = Plugin_013_read();
      if (/* !correlates(startUpDistance, nextVal[i]) && */ correlates(nextVal[i], nextVal[i - 1]))      {
        /* curVal = nextVal; */
      }
      else
        break;
    }

    if (i > 2) {
      for (int j = 0; j < i; j++)    {
        Serial.print(nextVal[j]);
        Serial.print(",");
        Serial.println((i - 1) * 10);
      }
      Serial.println("500,0");
    }

  } else {
    // Serial.print(curVal);
    // Serial.println(",0");
  }

  /* if (reportable_reading(curVal)/* && multiCheck(10)* /) {
    vermut++;
    // net_report(curVal);
    // resetNextReport();
    // Serial.println("---");
    } else if (millis() > nextReport) {
    // Report safe values
    // net_report(curVal);
    resetNextReport();
    Serial.println("-MARK-");
    Serial.println(vermut);
    Serial.println(total);
    Serial.println(1.0 * vermut/total);
    vermut = 0;
    total = 0;
    } */
}

boolean correlates(int base, int value) {
  return (abs(base - value) <= 2);
}

boolean nonStartUpDistance(float reading) {
  return (reading > 0 && abs(reading - startUpDistance) > 5);
}

#define THRESHOLD 0.3
boolean multiCheck(int amount_to_check) {
  float reportable = 0;
  for (int i = 0; i < amount_to_check; i++) {
    if (reportable_reading(Plugin_013_read()))
      reportable++;
  }

  Serial.print("Percentage: ");
  Serial.println(reportable / amount_to_check);
  return ((reportable / amount_to_check) > THRESHOLD);
}

void resetNextReport() {
  nextReport = millis() + 10 * 1000;
}



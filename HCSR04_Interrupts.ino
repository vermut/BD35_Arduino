void net_register(float medianDistance);
void net_setup();
void net_report(int lastVal);


#define VCC          D4
#define TRIGGER_PIN  D3  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     D2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define GND          D1

ADC_MODE(ADC_VCC);

// #include <NewPing.h>
// #define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
// NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

float startUpDistance = -1;
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
  float curVal = Plugin_013_read();

  if (reportable_reading(curVal)) {
    // Hm? Get the median reading
    float median = Plugin_013_Median(10);

    if (reportable_reading(median)) {
      net_report(curVal);
      net_report(median);
      resetNextReport();
      Serial.println("---");
    }
  } else if (millis() > nextReport) {
    // Report safe values
    net_report(curVal);
    resetNextReport();
    Serial.println("-MARK-");
  }
}

void loop2() {
  delay(15);
  float curVal = Plugin_013_read();

  // Check two readings in a row
  if (reportable_reading(curVal)) {
      delay(30);float doubleCheck = Plugin_013_read();
      delay(30);float tripleCheck = Plugin_013_read();

    if (reportable_reading(doubleCheck)) {
      net_report(curVal);
      net_report(doubleCheck);
      net_report(tripleCheck);
      resetNextReport();
      Serial.println("---");
    }
  } else if (millis() > nextReport) {
    // Report safe values
    net_report(curVal);
    resetNextReport();
    Serial.println("-MARK-");
  }
}

boolean reportable_reading(float reading) {
  return (reading > 0 && abs(reading - startUpDistance) > 5);
}

void resetNextReport() {
  nextReport = millis() + 30 * 1000;
}



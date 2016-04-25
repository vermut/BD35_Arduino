void net_register(float medianDistance);
void net_enable();
void net_disable();
void net_connect();
void net_disconnect();
void net_wait_connect();
bool net_is_connected();
void net_report(int lastVal1, int lastVal2);

void Plugin_013_Init();
float Plugin_013_Median(byte id, uint32_t it);
float Plugin_013_read(byte id);


#define VCC          D4
#define TRIGGER_PIN  D3  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     D2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define GND          D1

// Using native VCC (3.3v)
#define VCC2          D7
#define TRIGGER_PIN2  D6  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN2     D5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define GND2          D0

ADC_MODE(ADC_VCC);
float startUpDistance[] = { -1, -1};

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  Serial.setDebugOutput(true);
  net_enable();
  net_wait_connect();

  // Power setup
  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, HIGH);
  pinMode(VCC2, OUTPUT);
  digitalWrite(VCC2, HIGH);
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);
  pinMode(GND2, OUTPUT);
  digitalWrite(GND2, LOW);


  Plugin_013_Init();
  startUpDistance[0] = Plugin_013_Median(0, 20);
  startUpDistance[1] = Plugin_013_Median(1, 20);

  Serial.print("Startup: ");
  Serial.print(startUpDistance[0]);
  Serial.print(", ");
  Serial.print(startUpDistance[1]);
  Serial.println("cm");

  chill();
}

void loop() {
//  delay(50);
  pingSonar();
  reportIfNeccessary();
  handleWifi();
}

int alertPings[] = {0, 0};
enum WifiState {
  OFF,
  DO_CONNECT,
  CONNECTING,
  DO_REPORT,
  CHILL
};
WifiState wifiState = OFF;
void pingSonar() {
  if (wifiState == CONNECTING || wifiState == DO_REPORT)
    return;

  for (int i = 0; i <= 1; i++)
  {
    float curVal = Plugin_013_read(i);
    if (nonStartUpDistance(i, curVal)) {
      Serial.println(curVal);
      alertPings[i]++;
    } else {
      alertPings[i] = 0;
    }
  }
}

#define ALERT_THRESHOLD 1
boolean timeForReport = false;
long nextReport = 0;
void reportIfNeccessary() {
  if (!timeForReport && (
        alertPings[0] >= ALERT_THRESHOLD || alertPings[1] >= ALERT_THRESHOLD
        || millis() > nextReport)) {
    timeForReport = true;
    wifiState = DO_CONNECT;
  }
}

long chillTime = 0;
void handleWifi() {
  // Serial.println(wifiState);
  switch (wifiState) {
    case OFF: return;

    case DO_CONNECT:
      net_connect();
      wifiState = CONNECTING;
      Serial.println("-> CONNECTING");
      break;

    case CONNECTING:
      if (net_is_connected())
        wifiState = DO_REPORT;
      break;

    case DO_REPORT:
      Serial.println("-> DO_REPORT");
      net_report(alertPings[0], alertPings[1]);
      Serial.println("-> DONE_REPORT");
      chill();
      Serial.println("-> CHILL");
      break;

    case CHILL:
      if (millis() > chillTime) {
        reportSucceeded();
        Serial.println("-> READY");
      }
      break;
  }
}

void chill() {
  timeForReport = true;   // keep it true here
  net_disconnect();
  chillTime = millis() + 500;
  wifiState = CHILL;
}

void reportSucceeded() {
  nextReport = millis() + 60 * 1000;
  wifiState = OFF;
  timeForReport = false;
  for (int i = 0; i < 1; i++)
    alertPings[i] = 0;
}

/*
  boolean correlates(float base, float value) {
  return (abs(base - value) <= 5);
  }*/

boolean nonStartUpDistance(byte id, float reading) {
  return (reading > 0 && abs(reading - startUpDistance[id]) > 15);
}

/*
  void resetNextReport() {
  nextReport = millis() + 10 * 1000;
  }*/

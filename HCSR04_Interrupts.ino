void net_register(float medianDistance);
void net_enable();
void net_disable();
void net_wait_connect();
bool net_is_connected();
void net_report(int lastVal);

void Plugin_013_Init();
float Plugin_013_Median(byte id, uint32_t it);
float Plugin_013_read(byte id);


#define VCC          D4
#define TRIGGER_PIN  D3  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     D2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define GND          D1

// Using native VCC (3.3v)
#define TRIGGER_PIN2  D8  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN2     D7  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define GND2          D6

ADC_MODE(ADC_VCC);
float startUpDistance[] = {-1, -1};

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  Serial.setDebugOutput(true);
  net_enable();
  net_wait_connect();

  // Power setup
  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, HIGH);
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

  // net_register(startUpDistance);
  // net_report(startUpDistance);
  // resetNextReport();

  net_disable();
  reportSucceeded();
}

void loop() {
  pingSonar();
  reportIfNeccessary();
  handleWifi();
}

int alertPings = 0;
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
  
  float curVal = Plugin_013_read(0);
  if (nonStartUpDistance(0, curVal)) {
    Serial.println(curVal);
    alertPings++;
  } else {
    alertPings = 0;
  }
}

#define ALERT_THRESHOLD 1
boolean timeForReport = false;
long nextReport = 0;
void reportIfNeccessary() {
  if (!timeForReport && (alertPings >= ALERT_THRESHOLD || millis() > nextReport)) {
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
      net_enable();
      wifiState = CONNECTING;
      break;

    case CONNECTING:
      if (net_is_connected())
        wifiState = DO_REPORT;
      break;

    case DO_REPORT:
      net_report(alertPings);
      net_disable();
      chillTime = millis() + 5 * 1000;
      wifiState = CHILL;
      break;

    case CHILL:
      if (millis() > chillTime) {
        reportSucceeded();
      }
      break;
  }
}

void reportSucceeded() {
  nextReport = millis() + 60 * 1000;
  wifiState = OFF;
  timeForReport = false;
  alertPings = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  void loopy() {
  // delay(15);
  float curVal =Plugin_013_read();
  if (curVal < 0 || curVal > 220)
    curVal = 220;

  if (false && nonStartUpDistance(curVal)) {
    float nextVal[10];
    nextVal[0] = curVal;
    int i = 1;
    for (; i < 10; i++)    {
      delay(15);
      nextVal[i] = Plugin_013_read();
      if (/* !correlates(startUpDistance, nextVal[i]) && !correlates(nextVal[i], nextVal[i - 1]))
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
    Serial.print(curVal);
    Serial.println(",0");
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
    } 
}*/

boolean correlates(float base, float value) {
  return (abs(base - value) <= 5);
}

boolean nonStartUpDistance(byte id, float reading) {
  return (reading > 0 && abs(reading - startUpDistance[id]) > 5);
}

void resetNextReport() {
  nextReport = millis() + 10 * 1000;
}

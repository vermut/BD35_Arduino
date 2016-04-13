void net_register(float medianDistance);
void net_setup();
void net_report(int lastVal);

#define VCC          D4
#define TRIGGER_PIN  D3  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     D2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define GND          D1

ADC_MODE(ADC_VCC);
float startUpDistance = -1;

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  // Serial.setDebugOutput(true);
  net_enable();
  net_wait_connect();

  // Power setup
  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, HIGH);
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);

  Plugin_013_Init();
  startUpDistance = Plugin_013_Median(20);

  /* Serial.print("Startup: ");
  Serial.print(startUpDistance);
  Serial.println("cm"); */

  // net_register(startUpDistance);
  // net_report(startUpDistance);
  // resetNextReport();

  net_disable();
}

void loop() {
  pingSonar();
  reportIfNeccessary();
  handleWifi();  
}

int alertPings = 0;
void pingSonar() {
  float curVal =Plugin_013_read();
  if (nonStartUpDistance(curVal)) {
    alertPings++;
  }
}

boolean timeForReport = false;
long nextReport = 0;
void reportIfNeccessary() {
  if (alertPings > 0 || millis() > nextReport) {
    timeForReport = true;
    wifiState = WifiState.DO_CONNECT;
  }   
}

enum WifiState {
  OFF,
  DO_CONNECT,
  CONNECTING,
  DO_REPORT
};
WifiState wifiState = WifiState.OFF;
void handleWifi() {
  switch (wifiState):
    case WifiState.OFF: return;

    case WifiState.DO_CONNECT:
      net_enable();
      wifiState = WifiState.CONNECTING;
      break;

    case WifiState.CONNECTING:
      if (net_is_connected())
         wifiState = WifiState.DO_REPORT;
      break;
      
    case WifiState.DO_REPORT:
      net_report(alertPings);
      net_disable();
      reportSucceeded();
      break;
  }    
}

void reportSucceeded() {
  nextReport = millis() + 60 * 1000;
  wifiState = WifiState.OFF;
  timeForReport = false;
  alertPings = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

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
      if (/* !correlates(startUpDistance, nextVal[i]) && */ !correlates(nextVal[i], nextVal[i - 1]))
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
    } */
}

boolean correlates(float base, float value) {
  return (abs(base - value) <= 5);
}

boolean nonStartUpDistance(float reading) {
  return (reading > 0 && abs(reading - startUpDistance) > 5);
}

void resetNextReport() {
  nextReport = millis() + 10 * 1000;
}



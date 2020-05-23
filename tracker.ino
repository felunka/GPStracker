#include <DFRobot_sim808.h>
#include <sim808.h>
#include <NMEAGPS.h>
#include <SoftwareSerial.h>

#define MESSAGE_LENGTH 160
#define PIN_TX    10
#define PIN_RX    11

#define MESSAGE_LENGTH 160

SoftwareSerial SIMCOM(PIN_TX, PIN_RX);
DFRobot_SIM808 sim808(&SIMCOM);

NMEAGPS gps;
gps_fix fix;

enum state_t {
  WAITING,
  GPS_PWR_ON, GPS_PWR_ON_DELAY,
  GPS_PWR_OFF, GPS_PWR_OFF_DELAY,
  GPS_PKT_ON, GPS_PKT_ON_DELAY,
  GPS_PKT_OFF, GPS_PKT_OFF_DELAY,
  GET_GPS, GET_GPS_DELAY
};

void setup() {
  SIMCOM.begin(9600);
  Serial.begin(9600);

  while (!sim808.init()) {
    Serial.print("Sim808 init error\n");
    delay(1000);
  }
  delay(3000);

  Serial.println("Init success!");
}

void loop() {
  int messageIndex = sim808.isSMSunread();

  if (messageIndex > 0) {
    Serial.println("Message got!");
    
    char trash[MESSAGE_LENGTH];
    char phoneNumber[24];
    char datetime[24];
    sim808.readSMS(messageIndex, trash, MESSAGE_LENGTH, phoneNumber, datetime);
    
    Serial.print("From: ");
    Serial.println(phoneNumber);
    Serial.println();
    
    delay(200);
    
    sim808.deleteSMS(messageIndex);

    Serial.println("Start get GPS pos");
    updatePos();
    
    Serial.println();
    Serial.println("FIX!");

    float lat = fix.latitude();
    float lon = fix.longitude();

    Serial.print("lat: ");
    Serial.println(lat, 6);
    Serial.print("lon: ");
    Serial.println(lon, 6);

    char lat_str[10];
    char lon_str[10];

    dtostrf(lat, 8, 6, lat_str);
    dtostrf(lon, 8, 6, lon_str);

    char message[200];
    sprintf(message, "http://maps.google.com/maps?q=%s,%s", lat_str, lon_str);
    Serial.println(message);

    sim808.sendSMS(phoneNumber, message);
  } else {
    Serial.println("nomessage");
  }
  delay(500);
}

void updatePos() {
  sim808.attachGPS();

  state_t  state = WAITING;
  uint32_t stateTime;

  serialClear();
  state = GPS_PWR_ON;

  while (state != WAITING) {
    switch (state) {
      case WAITING:
        serialClear();
        if (millis() - stateTime >= 10000) // once every 10 seconds
          state = GPS_PWR_ON;
        break;

      case GPS_PWR_ON:
        SIMCOM.println( F("AT+CGPSPWR=1") );
        state     = GPS_PWR_ON_DELAY;
        stateTime = millis();
        break;

      case GPS_PWR_ON_DELAY:
        serialClear();
        if (millis() - stateTime >= 5000)
          state = GPS_PKT_ON;
        break;

      case GPS_PKT_ON:
        SIMCOM.println( F("AT+CGPSOUT=1") );
        state     = GPS_PKT_ON_DELAY;
        stateTime = millis();
        break;

      case GPS_PKT_ON_DELAY:
        serialClear();
        if (millis() - stateTime >= 200)
          state = GET_GPS;
        break;

      case GET_GPS:
        if (gps.available( SIMCOM )) {
          fix = gps.read(); // received a new fix structure!

          state     = GET_GPS_DELAY;
          stateTime = millis();
        }
        break;

      case GET_GPS_DELAY:
        serialClear();
        if (millis() - stateTime >= 200)
          state = GPS_PKT_OFF;
        break;

      case GPS_PKT_OFF:
        SIMCOM.println( F("AT+CGPSOUT=0") );
        state     = GPS_PKT_OFF_DELAY;
        stateTime = millis();
        break;

      case GPS_PKT_OFF_DELAY:
        serialClear();
        if (millis() - stateTime >= 200)
          state = GPS_PWR_OFF;
        break;

      case GPS_PWR_OFF:
        SIMCOM.println( F("AT+CGPSPWR=0") );
        state     = GPS_PWR_OFF_DELAY;
        stateTime = millis();
        break;

      case GPS_PWR_OFF_DELAY:
        serialClear();
        if (millis() - stateTime >= 500) {
          stateTime = millis();
          state = WAITING;
        }
        break;
    }
  }

  sim808.detachGPS();
}

void serialClear() {
  // Empty the input buffer.
  while (SIMCOM.available()) {
    char c = SIMCOM.read();
    Serial.write(c); // to see the response, if you want
  }
}

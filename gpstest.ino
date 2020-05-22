#include <SoftwareSerial.h>
#include <DFRobot_sim808.h>
#include <sim808.h>

#include <NMEAGPS.h>
NMEAGPS gps;
gps_fix fix; // the parsed pieces from the GPRMC

#define DEBUG(x)      Serial.println(x)

SoftwareSerial SIMCOM(10, 11);
DFRobot_SIM808 sim808(&SIMCOM);

enum state_t
{
  WAITING,
  GPS_PWR_ON, GPS_PWR_ON_DELAY,
  GPS_PWR_OFF, GPS_PWR_OFF_DELAY,
  GPS_PKT_ON, GPS_PKT_ON_DELAY,
  GPS_PKT_OFF, GPS_PKT_OFF_DELAY,
  GET_GPS, GET_GPS_DELAY
};

state_t  state = WAITING;
uint32_t stateTime;

void setup() {
  Serial.begin(9600);
  SIMCOM.begin(9600);

  while (!sim808.init()) {
    Serial.print("Sim808 init error\r\n");
    delay(1000);
  }
  delay(3000);

  sim808.attachGPS();
}

void loop() {
  delay(500);

  switch (state) {

    case WAITING:
      serialClear();
      if (millis() - stateTime >= 10000) // once every 10 seconds
        state = GPS_PWR_ON;
      break;

    case GPS_PWR_ON:
      DEBUG("GPS PWR ON");
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
      DEBUG("GPS PKT ON");
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
        Serial.println("");
        Serial.println("FIX!");
        Serial.print("lat: ");
        Serial.println(fix.latitude(), 6);
        Serial.print("lon: ");
        Serial.println(fix.longitude(), 6);

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
      DEBUG("GPS PKT OFF");
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
      DEBUG("GPS PWR OFF");
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
} // loop


void serialClear() {
  // Empty the input buffer.
  while (SIMCOM.available()) {
    char c = SIMCOM.read();
    Serial.write( c ); // to see the response, if you want
  }
}

#include <Arduino.h>
#include <DebugUtils.h>

#define DEBUG

const int BUTTON = 3;
const int MOSFET = LED_BUILTIN; //5
const int POTI = A6;

int interval = 0;
int minInterval = 1023;
int maxInterval = 0;

int buttonState = 0;
int wasRealesed = 0;
unsigned long previousMillis = 0;

void setup()
{
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(MOSFET, OUTPUT);
  debugln("Calibration started...");
  calibrate();
  debugln("Calibration done.");
}

void loop()
{
  readInterval();

  if (1 == digitalRead(BUTTON))
    wasRealesed = 1;
  if (!digitalRead(BUTTON) && wasRealesed)
    shift();
}

void shift()
{
  previousMillis = millis();
  digitalWrite(MOSFET, HIGH);
  while (millis() - previousMillis < interval);
  digitalWrite(MOSFET, LOW);

  // prevent continuous execution
  wasRealesed = 0;
}

void calibrate()
{
  while (millis() < 5000)
  {
    interval = analogRead(POTI);
    debugln(interval);
    // record the maximum sensor value
    if (interval > maxInterval)
      maxInterval = interval;

    // record the minimum sensor value
    if (interval < minInterval)
      minInterval = interval;
  }
}

void readInterval()
{
  interval = analogRead(POTI);
  interval = map(interval, minInterval, maxInterval, 50, 200);
  interval = constrain(interval, 50, 200);
}

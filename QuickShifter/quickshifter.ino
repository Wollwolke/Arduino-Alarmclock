#include <Arduino.h>

#define BUTTON 2
#define LED LED_BUILTIN

extern HardwareSerial Serial;

const int interval = 60;

int buttonState = 0;
int wasRealesed = 0;
unsigned long previousMillis = 0;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  // may not recognice to short releases...
  if(1 == digitalRead(BUTTON))
    wasRealesed = 1;
  if (!digitalRead(BUTTON) && wasRealesed)
    shift();
}

//TODO: Dauerkontakt fixen...
void shift(){
  previousMillis = millis();
  digitalWrite(LED, HIGH);
  while(millis() - previousMillis < interval){}
  digitalWrite(LED, LOW);

  // prevent continuous execution
  wasRealesed = 0;

  // "debouncing"
  delay(500);
}

#include <Arduino.h>

#define BUTTON 2
#define LED LED_BUILTIN

extern HardwareSerial Serial;

int buttonState = 0;
unsigned long previousMillis = 0;
const int interval = 60;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  buttonState = digitalRead(BUTTON);
  if (!buttonState)
    shift();
}

//TODO: Dauerkontakt fixen...
void shift(){
  previousMillis = millis();
  digitalWrite(LED, HIGH);
  while(millis() - previousMillis < interval){}
  digitalWrite(LED, LOW);
}

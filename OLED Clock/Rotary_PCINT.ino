/* Example sketch for Rotary library. 
 * Original library written by Ben Buxton (available at http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html)
 * Extended to support encoders with a push button
 *
 * Simple demonstration that will output to the serial monitor when the enocder is turned
 * or the button is pressed.
 * 
 */
#include <Arduino.h>
#include <avr/interrupt.h>
#include "lib/lib.h"
#include "lib/rotary/rotary.h"
#include "lib/U8g2/src/U8g2lib.h"

extern HardwareSerial Serial;

// Initialize the Rotary object
// Rotary(Encoder Pin 1, Encoder Pin 2, Button Pin)
Rotary r = Rotary(8, 9, 10);

//Initialize Display object
U8G2_SSD1306_128X64_NONAME_1_HW_I2C Display(U8G2_R0, /*reset=*/U8X8_PIN_NONE);

volatile int num = 0;

void setup()
{
  Serial.begin(9600);
  Display.begin();

  // Pin Change Interrput Initialisation
  cli();
  PCICR |= 0b00000001;  // turn on port b
  PCMSK0 |= 0b00000111; // turn on pins PCINT0, PCINT1, PCINT2
  sei();

  Serial.println("Initialized");
}

void loop()
{
  Draw();
}

void Draw()
{
  Display.firstPage();
  do
  {
    Display.setFont(u8g2_font_logisoso62_tn);
    Display.drawStr(num, 63, "9");
  } while (Display.nextPage());
  //delay(1000);
}

ISR(PCINT0_vect)
{
  volatile unsigned char val = r.process();
  // if the encoder has been turned, check the direction
  if (val == r.clockwise())
  {
    num++;
    Serial.println("Clockwise");
  }
  else if (val == r.counterClockwise())
  {
    num--;
    Serial.println("Counter-Clockwise");
  }

  // Check to see if the button has been pressed.
  // Passes in a debounce delay of 20 milliseconds
  if (r.buttonPressedReleased(20))
  {
    num = 0;
    Serial.println("Button pressed");
  }
}

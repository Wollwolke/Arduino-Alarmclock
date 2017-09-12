#include <Arduino.h>
#include <avr/interrupt.h>
#include <rotary.h>
#include <U8g2lib.h>
#include <Rtc_Pcf8563.h>

extern HardwareSerial Serial;

// Initialize the Rotary object
// Rotary(Encoder Pin 1, Encoder Pin 2, Button Pin)
Rotary r = Rotary(8, 9, 10);

//Initialize Display object
U8G2_SSD1306_128X64_NONAME_2_HW_I2C Display(U8G2_R0, /*reset=*/U8X8_PIN_NONE);

//init the real time clock
Rtc_Pcf8563 rtc;

enum menuLevels
{
  TIME = 0,
  SETTIME,
  ALARM,
  SETTINGS,
  BACK
};

enum input
{
  NA,
  CW,
  CC,
  PR,
  HL
};

volatile enum input input;
enum menuLevels state = TIME;

void CheckState()
{
  switch (state)
  {
  case TIME:
    switch (input)
    {
    case PR:
      state = SETTIME;
      break;
    }
    break;

  case SETTIME:
    switch (input)
    {
    case PR:
      SetTime();
      break;
    case CW:
      state = ALARM;
      break;
    case CC:
      state = BACK;
      break;
    }
    break;

  case ALARM:
    switch (input)
    {
    case PR:
      break;
    case CW:
      state = SETTINGS;
      break;
    case CC:
      state = SETTIME;
      break;
    }
    break;

  case SETTINGS:
    switch (input)
    {
    case PR:
      break;
    case CW:
      state = BACK;
      break;
    case CC:
      state = ALARM;
      break;
    }
    break;

  case BACK:
    switch (input)
    {
    case PR:
      state = TIME;
      break;
    case CW:
      state = SETTIME;
      break;
    case CC:
      state = SETTINGS;
      break;
    }
  }
  input = NA;
}

void SetTime()
{
  byte time[4] = {rtc.getHour(), rtc.getMinute(), rtc.getSecond(), 12}; // hour, minute, second, position
  bool finished = false;
  int state = 0;
  input = NA;
  do
  {
    switch (state)
    {
    case 0:
      switch (input)
      {
      case PR:
        time[3] = 57;
        state = 1;
        break;
      case CW:
        if (time[0] == 23)
        {
          time[0] = 0;
        }
        else
        {
          time[0]++;
        }
        break;
      case CC:
        if (time[0] == 0)
        {
          time[0] = 23;
        }
        else
        {
          time[0]--;
        }
        break;
      }
      break;

    case 1:
      switch (input)
      {
      case PR:
        time[3] = 102;
        state = 2;
        break;
      case CW:
        if (time[1] == 59)
        {
          time[1] = 0;
        }
        else
        {
          time[1]++;
        }
        break;
      case CC:
        if (time[1] == 0)
        {
          time[1] = 59;
        }
        else
        {
          time[1]--;
        }
        break;
      }
      break;

    case 2:
      switch (input)
      {
      case PR:
        rtc.setTime(time[0], time[1], time[2]);
        finished = true;
        break;
      case CW:
        if (time[2] == 59)
        {
          time[2] = 0;
        }
        else
        {
          time[2]++;
        }
        break;
      case CC:
        if (time[2] == 0)
        {
          time[2] = 59;
        }
        else
        {
          time[2]--;
        }
        break;
      }
      break;
    }
    input = NA;
    DrawSetTime(time);
  } while (!finished);
}

void DrawSetTime(byte time[])
{
  char buffer[5];
  Display.firstPage();
  do
  {
    //Caption
    Display.setFont(u8g2_font_helvB12_tr);
    Display.drawStr((128 - Display.getStrWidth("Time")) / 2, 13, "Time");
    Display.drawHLine(0, 15, 128);

    //Item
    Display.setFont(u8g2_font_helvR24_tr);
    sprintf(buffer, "%02d:%02d:%02d", time[0], time[1], time[2]);
    Display.drawStr(2, 52, buffer);
    Display.drawBox(time[3], 57, 14, 2);
  } while (Display.nextPage());
}

void setup()
{
  Serial.begin(9600);
  Display.begin();
  Display.enableUTF8Print();
  rtc.initClock();
  rtc.setDateTime(21, 0, 8, 0, 17, 18, 5, 35);

  // Pin Change Interrput Initialisation
  cli();
  PCICR |= 0b00000001;  // turn on port b
  PCMSK0 |= 0b00000111; // turn on pins PCINT0, PCINT1, PCINT2
  sei();

  Serial.println("Initialized");
  delay(2000);
}

void loop()
{
  if (input != NA)
  {
    CheckState();
  }
  if (state > 0)
  {
    DrawMenu();
  }
  else
  {
    //DrawTime();
    char time[20];
    char old[20];
    strcpy(time, rtc.formatTime());

    if (strcmp(old, time))
    {
        DrawTime(time);
        strcpy(old, time);
    }
  }
}

void DrawMenu()
{
  static char menuItemStrings[][9] = {
      "Time",
      "Alarm",
      "Settings",
      "Back"};

  Display.firstPage();
  do
  {
    //Caption
    Display.setFont(u8g2_font_helvB12_tr);
    Display.drawStr((128 - Display.getStrWidth("Menu")) / 2, 13, "Menu");
    Display.drawHLine(0, 15, 128);

    //Item
    Display.setFont(u8g2_font_helvR24_tr);
    Display.drawStr((128 - Display.getStrWidth(menuItemStrings[state - 1])) / 2, 15 + Display.getAscent() + ((64 - 15 - Display.getAscent()) / 2), menuItemStrings[state - 1]);
  } while (Display.nextPage());
}

void DrawTime(char* time)
{
  Display.firstPage();
  do
  {
    //Item
    Display.setFont(u8g2_font_helvR24_tr);
    Display.drawStr(2, 52, time);
  } while (Display.nextPage());
}

ISR(PCINT0_vect)
{
  volatile unsigned char val = r.process();
  // if the encoder has been turned, check the direction
  if (val == r.clockwise())
  {
    Serial.println("Clockwise");
    input = CW;
  }
  else if (val == r.counterClockwise())
  {
    Serial.println("Counter-Clockwise");
    input = CC;
  }

  // Check to see if the button has been pressed.
  // Passes in a debounce delay of 20 milliseconds
  if (r.buttonPressedReleased(20))
  {
    Serial.println("Button pressed");
    input = PR;
  }
}
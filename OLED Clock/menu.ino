#include <stddef.h>
#include <Arduino.h>
#include <avr/interrupt.h>
#include <rotary.h>
#include <U8g2lib.h>
#include <Rtc_Pcf8563.h>

extern HardwareSerial Serial;

//function prototypes
void DrawMenu();
void DrawTime();
void SetTime();

// Initialize the Rotary object
// Rotary(Encoder Pin 1, Encoder Pin 2, Button Pin)
Rotary r = Rotary(8, 9, 10);

//Initialize Display object
U8G2_SSD1306_128X64_NONAME_1_HW_I2C Display(U8G2_R0, /*reset=*/U8X8_PIN_NONE);

//init the real time clock
Rtc_Pcf8563 rtc;

typedef void(*funcPtr)(void); //typedef für Funktionszeiger
funcPtr currentStateFunction;  //aktuelle Funktion

enum states
{
    CLOCK = 0,
    SETTIME,
    ALARM,
    SETTINGS,
    BACK
} currentState;

volatile enum inputs
{
    CW = 0,
    CC,
    PR,
    HL,
    NA
} input;

typedef struct
{
  funcPtr currentStateFunc; //aktuelle Zustandsfunktion
  states nextSate; //nächster Zustand
  funcPtr actionToDo; //Aktion bei Zustandsübergang
} stateElement;

stateElement ttable[5][3] = /* ttable[CurrentState][Input] => NextStateElement */
{
                        /* CW */                            /* CC */                        /* PR */
    /* CLOCK */         { {DrawTime, CLOCK, NULL},       {DrawTime, CLOCK, NULL},        {DrawMenu, SETTIME, NULL}    },
    /* SETTIME */       { {DrawMenu, ALARM, NULL},       {DrawMenu, BACK, NULL},         {DrawMenu, SETTIME, SetTime} },
    /* ALARM */         { {DrawMenu, SETTINGS, NULL},    {DrawMenu, SETTIME, NULL},      {DrawMenu, ALARM, NULL}      },
    /* SETTINGS */      { {DrawMenu, BACK, NULL},        {DrawMenu, ALARM, NULL},        {DrawMenu, SETTINGS, NULL}   },
    /* BACK */          { {DrawMenu, SETTIME, NULL},     {DrawMenu, SETTINGS, NULL},     {DrawTime, CLOCK, NULL}      }
};

void setup()
{
  Serial.begin(9600);
  Display.begin();
  Display.enableUTF8Print();
  Display.setContrast(1);
  rtc.initClock();
  rtc.setDateTime(21, 0, 8, 0, 17, 18, 5, 35);

  // Pin Change Interrput Initialisation
  cli();
  PCICR |= 0b00000001;  // turn on port b
  PCMSK0 |= 0b00000111; // turn on pins PCINT0, PCINT1, PCINT2
  sei();

  //Ausgangszustand setzen
  input = NA;
  currentState = CLOCK;
  currentStateFunction = DrawTime;

  Serial.println("Initialized");
}

void loop()
{
  EventTrigger();

  //aktuelle Zustandsfunktion ausführen
  if (currentStateFunction != NULL)
    (*currentStateFunction)();
}

//Zustand ändern
void StateChange(int i)
{
  //hole Element in Abhänigkeit von aktuellem Zustand und Ereignis
  stateElement stateEvaluation = ttable[currentState][i];

  //setze nächsten Zustand
  currentState = stateEvaluation.nextSate;

  //setze aktuelle Zustandsfunktion
  currentStateFunction = stateEvaluation.currentStateFunc;

  //Aktion ausführen
  if (stateEvaluation.actionToDo != NULL)
    (*stateEvaluation.actionToDo)();
}

//Events abfragen
void EventTrigger()
{
  if (input != NA)
  {
    StateChange(input);
    input = NA;
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
    Display.drawStr((128 - Display.getStrWidth(menuItemStrings[currentState - 1])) / 2, 15 + Display.getAscent() + ((64 - 15 - Display.getAscent()) / 2), menuItemStrings[currentState - 1]);
  } while (Display.nextPage());
}

void DrawTime()
{
  char time[20];
  char old[20];
  strcpy(time, rtc.formatTime());

  if (strcmp(old, time))
  {
    Display.firstPage();
    do
    {
      //Item
      Display.setFont(u8g2_font_helvR24_tr);
      Display.drawStr(2, 52, time);
    } while (Display.nextPage());
    strcpy(old, time);
  }
}

void SetTime() {
  input = NA;
  byte time[4] = {rtc.getHour(), rtc.getMinute(), rtc.getSecond(), 12}; // hour, minute, second, position
  int i = 0;
  int max = 24;
  while (i < 3)
  {
    switch (input)
    {
    case CW:
      time[i] = (time[i] + 1) % max;
      input = NA;
      break;
    case CC:
      time[i] = (time[i] - 1 + max) % max;
      input = NA;
      break;
    case PR:
      time[3] += 45;
      max = 60;
      i++;
      input = NA;
    }
    DrawSetTime(time);   
  }
  rtc.setTime(time[0], time[1], time[2]);
}


void DrawSetTime(byte time[])
{
  char buffer[9];
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
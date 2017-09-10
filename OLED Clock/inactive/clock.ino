#include <Rtc_Pcf8563.h>

extern HardwareSerial Serial;

//init the real time clock
Rtc_Pcf8563 rtc;

void setup()
{
    Serial.begin(9600);
    Serial.println("Set RTC");
    rtc.initClock();
    rtc.setDateTime(21, 0, 8, 0, 17, 18, 5, 35);
}
void loop()
{
    char time[20];
    char old[20];
    strcpy(time, rtc.formatTime());

    if (strcmp(old, time))
    {
        Serial.println(time);
        strcpy(old, time);
    }
}

#include <Wire.h>
#include <Time.h>
#include <DS3232RTC.h>

const char * const weekdays[] = {
  "Err", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

void setup() {
  Serial.begin(9600);
  while (!Serial) continue;
}

void loop() {
  tmElements_t tm;
  if (RTC.get(&tm)) {
    char buf[24];
    sprintf(buf, "%02d:%02d:%02d %s %d/%d/%d",
      tm.Hour, tm.Minute, tm.Second,
      weekdays[tm.Wday], tm.Day, tm.Month,
      tmYearToCalendar(tm.Year));
  
    Serial.println(buf);
  } else {
    Serial.println("RTC read error!");
  }

  delay(1000);
}
#include <Wire.h>
#include <Time.h>
#include <DS3232RTC.h>

const char * const weekdays[] = {
  "Err", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

const char * const months[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void setup() {
  Serial.begin(9600);
  while (!Serial) continue;
  
  tmElements_t tm;
  
  if (parse(&tm, __TIME__, __DATE__)) {
    // Set the time and immediately check it so
    // that the time displayed is that of the RTC.
    if (RTC.set(tm) && RTC.get(&tm)) {
      Serial.print("RTC configured: ");
      
      char buf[24];
      sprintf(buf, "%02d:%02d:%02d %s %d/%d/%d",
        tm.Hour, tm.Minute, tm.Second,
        weekdays[tm.Wday], tm.Day, tm.Month,
        tmYearToCalendar(tm.Year));
  
      Serial.println(buf);
    } else {
      Serial.println("RTC communication error!");
    }
  } else {
    Serial.println("Could not parse information.");
    Serial.print("Time = \"");
    Serial.print(__TIME__);
    Serial.print("\", Date = \"");
    Serial.print(__DATE__);
    Serial.println("\".");
  }
}

void loop() {}

bool parse(tmElements_t *tm, const char *strTime, const char *strDate) {
  int hour, minute, second, day, year;
  char month[8];

  bool parsedTime = sscanf(strTime, "%d:%d:%d", &hour, &minute, &second) == 3,
       parsedDate = sscanf(strDate, "%s %d %d", month, &day, &year) == 3;

  if (!parsedTime || !parsedDate) return false;
  
  uint8_t monthIndex = 0;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(month, months[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;

  tm->Second = second;
  tm->Minute = minute;
  tm->Hour   = hour;
  tm->Day    = day;
  tm->Month  = monthIndex + 1;
  tm->Year   = CalendarYrToTm(year);
  tm->Wday   = weekday(makeTime(*tm));
  
  return true;
}
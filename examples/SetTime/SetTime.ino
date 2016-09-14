#include <DS3232RTC.h>

const char weekdays[8][4] = {
  "Err", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

const char months[12][4] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void setup() {
  fdevopen([](char c, FILE (*)){Serial.write(c); return 1;}, NULL);
  Serial.begin(115200);
  while (!Serial) continue;

  tmElements_t tm;

  if (parse(tm, __TIME__, __DATE__)) {
    // Set the time and immediately check it so
    // that the time displayed is that of the RTC.
    if (RTC.set(tm) && RTC.get(tm)) {
      printf(
        "%02d:%02d:%02d %s %d/%d/%d\n",
        tm.Hour,
        tm.Minute,
        tm.Second,
        weekdays[tm.Wday],
        tm.Day,
        tm.Month,
        tmYearToCalendar(tm.Year)
      );
    } else {
      printf("RTC communication error!\n");
    }
  } else {
    printf("Could not parse information.\n");
    printf("Time = \"%s\", Date = \"%s\"\n", __TIME__, __DATE__);
  }
}

void loop() {}

bool parse(tmElements_t &tm, const char *strTime, const char *strDate) {
  int hour, minute, second, day, year;
  char month[8];

  bool parsedTime = sscanf(strTime, "%d:%d:%d", &hour, &minute, &second) == 3;
  bool parsedDate = sscanf(strDate, "%s %d %d", month, &day, &year) == 3;

  if (!parsedTime || !parsedDate) return false;

  uint8_t monthIndex = 0;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(month, months[monthIndex]) == 0) break;
  }
  if (monthIndex == 12) return false;

  tm.Second = second;
  tm.Minute = minute;
  tm.Hour   = hour;
  tm.Day    = day;
  tm.Month  = monthIndex;
  tm.Year   = CalendarYrToTm(year);
  tm.Wday   = weekday(makeTime(tm));

  return true;
}

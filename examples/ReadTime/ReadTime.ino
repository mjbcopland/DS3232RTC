#include <DS3232RTC.h>

const char weekdays[8][4] = {
  "Err", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

void setup() {
  fdevopen([](char c, FILE (*)){Serial.write(c); return 1;}, NULL);
  Serial.begin(115200);
  while (!Serial) continue;
}

void loop() {
  static tmElements_t tm;

  if (RTC.get(tm)) {
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
    printf("RTC read error!\n");
  }

  delay(1000);
}

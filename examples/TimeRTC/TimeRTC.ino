/*
 * TimeRTC.pde
 * Example code illustrating Time library with Real Time Clock.
 * This example is identical to the example provided with the Time Library
 * except that the #include statement and the function to get the time have
 * been slightly changed.
 */

#include <DS3232RTC.h>

const char weekdays[7][4] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

void setup() {
  fdevopen([](char c, FILE (*)){Serial.write(c); return 1;}, NULL);
  Serial.begin(115200);
  while (!Serial) continue;

  time_t t;
  if (RTC.get(t)) {
    setSyncProvider([]{time_t t; return RTC.get(t) ? t : 0;});

    if (timeStatus() != timeSet) {
      printf("Unable to sync with the RTC. Using internal clock.\n");
    } else {
      printf("RTC has set the system time.\n");
    }
  } else {
    printf("RTC read error! Please check the circuitry.\n");
    Serial.flush();
    exit(EXIT_FAILURE);
  }
}

void loop() {
  printf(
    "%02d:%02d:%02d %s %d/%d/%d\n",
    hour(),
    minute(),
    second(),
    weekdays[weekday()],
    day(),
    month(),
    year()
  );
  delay(1000);
}

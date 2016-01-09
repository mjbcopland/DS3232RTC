/*
 * TimeRTC.pde
 * Example code illustrating Time library with Real Time Clock.
 * This example is identical to the example provided with the Time Library
 * except that the #include statement and the function to get the time have
 * been slightly changed.
 */

#include <DS3232RTC.h>
#include <Time.h>         //http://www.arduino.cc/playground/Code/Time  
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)

void setup() {
    Serial.begin(9600);
    while (!Serial) continue;

    time_t t;
    if (RTC.get(&t)) {
        setSyncProvider([](){return RTC.get(&t) ? t : 0;});   // the function to get the time from the RTC
        if(timeStatus() != timeSet) {
            Serial.println("Unable to sync with the RTC");
        } else {
            Serial.println("RTC has set the system time");      
        }
    } else {
        Serial.print("RTC read error!  Please check the circuitry.");
    }
}

void loop() {
    digitalClockDisplay();  
    delay(1000);
}

void digitalClockDisplay() {
    // digital clock display of the time
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(' ');
    Serial.print(day());
    Serial.print(' ');
    Serial.print(month());
    Serial.print(' ');
    Serial.print(year()); 
    Serial.println(); 
}

void printDigits(int digits) {
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(':');
    if (digits < 10) Serial.print('0');
    Serial.print(digits);
}
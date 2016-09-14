/*----------------------------------------------------------------------*
 * DS3232RTC.h - Arduino library for the Maxim Integrated DS3232        *
 * Real-Time Clock. This library is intended for use with the Time      *
 * library (http://www.pjrc.com/teensy/td_libs_Time.html)               *
 *                                                                      *
 * This library is a modified verion of Jack Christensen's DS3232RTC    *
 * library. http://github.com/JChristensen/DS3232RTC                    *
 *                                                                      *
 * This library differs from Christensen's library wherever I felt      *
 * that a change would increase my ease of use. Functions to support    *
 * additional features of the DS3232 persist from Christensen's library *
 * however they are untested with this library.                         *
 *                                                                      *
 * This library will work with the DS3231 RTC, which has the same       *
 * features of the DS3232 except: (1) Battery-backed SRAM, (2) Battery- *
 * backed 32kHz output (BB32kHz bit in Control/Status register 0x0F),   *
 * and (3) Adjustable temperature sensor sample rate (CRATE1:0 bits in  *
 * the Control/Status register).                                        *
 *                                                                      *
 * Whether used with the DS3232 or DS3231, the user is responsible for  *
 * ensuring reads and writes do not exceed the device's address space   *
 * (0x00-0x12 for DS3231, 0x00-0xFF for DS3232); no bounds checking     *
 * is done by this library.                                             *
 *                                                                      *
 * Michael Copland                                                      *
 * September 2015                                                       *
 *                                                                      *
 * CC BY-SA 4.0                                                         *
 * http://creativecommons.org/licenses/by-sa/4.0/                       *
 *----------------------------------------------------------------------*/

#pragma once

#include <TimeLib.h>

// DS3232 Register Addresses -- default values are correct
typedef enum {
  RTC_SECONDS,
  RTC_MINUTES,
  RTC_HOURS,
  RTC_DAY,
  RTC_DATE,
  RTC_MONTH,
  RTC_YEAR,
  ALM1_SECONDS,
  ALM1_MINUTES,
  ALM1_HOURS,
  ALM1_DAYDATE,
  ALM2_MINUTES,
  ALM2_HOURS,
  ALM2_DAYDATE,
  RTC_CONTROL,
  RTC_STATUS,
  RTC_AGING,
  TEMP_MSB,
  TEMP_LSB,
  SRAM_START_ADDR = 0x14, // first SRAM address
  SRAM_SIZE = 236         // number of uint8_ts of SRAM
} register_t;

// Control register bit values -- default values are correct
typedef enum {
  A1IE,
  A2IE,
  INTCN,
  RS1,
  RS2,
  CONV,
  BBSQW,
  EOSC
} controlBitValues_t;

// Status register bit values -- default values are correct
typedef enum {
  A1F,
  A2F,
  BSY,
  EN32KHZ,
  CRATE0,
  CRATE1,
  BB32KHZ,
  OSF
} statusBitValues_t;

// Square-wave output frequency (TS2, RS1 bits)
typedef enum {
  SQWAVE_1_HZ,
  SQWAVE_1024_HZ,
  SQWAVE_4096_HZ,
  SQWAVE_8192_HZ,
  SQWAVE_NONE
} sqwaveFreqs_t;

// Alarm masks
typedef enum {
  MATCH_ANY     = 0x0F,
  MATCH_SECONDS = 0x0E,
  MATCH_MINUTES = 0x0C,
  MATCH_HOURS   = 0x08,
  MATCH_DAY     = 0x10,
  MATCH_DATE    = 0x00,

  /* syntactic sugar for matching at 0 */
  EVERY_SECOND  = MATCH_ANY,
  EVERY_MINUTE  = MATCH_SECONDS,
  EVERY_HOUR    = MATCH_MINUTES,
  EVERY_DAY     = MATCH_HOURS,
  EVERY_WEEK    = MATCH_DAY,
  EVERY_YEAR    = MATCH_DATE
} alarmTypes_t;

typedef enum {
  ALARM_1,
  ALARM_2
} alarm_t;

typedef enum {
  DYDT_FLAG  = 6, // Day/Date flag bit in alarm Day/Date registers (day is high)
  HR1224     = 6, // Hours register 12 or 24 hour mode (24 hour mode == 0)
  DS1307_CH  = 7, // for DS1307 compatibility, Clock Halt bit in Seconds register
  CENTURY    = 7, // Century bit in Month register
  ALARM_MASK = 7
} otherBitValues_t;

typedef enum {
  almSecond, almMinute, almHour, almDayDate, almNbrFields
} almByteFields;

typedef struct {
  uint8_t Second, Minute, Hour, DayDate;
} almElements_t, AlarmElements, *almElementsPtr_t;

class DS3232RTC {
public:
  DS3232RTC();

  static bool get(time_t &t);
  static bool get(tmElements_t &tm);

  static bool set(time_t t);
  static bool set(tmElements_t tm);

  static bool write(register_t addr, uint8_t value);
  static bool write(register_t addr, uint8_t *values, uint8_t n);

  static uint8_t read(register_t addr);
  static uint8_t read(register_t addr, uint8_t *values, uint8_t n);

  static bool setAlarm(alarm_t alarmNumber, alarmTypes_t alarmType, almElements_t tm = {0,0,0,0});

  static bool getAlarm(alarm_t alarmNumber, alarmTypes_t *alarmType, almElements_t *tm);

  static bool alarmInterrupt(alarm_t alarmNumber, bool alarmEnabled);

  static bool alarm(alarm_t alarmNumber, bool clearAlarm = true);

  static bool squareWave(sqwaveFreqs_t freq);

  static bool oscStopped(bool clearOSF = true);

  static int16_t temperature(void);

private:
  static const uint8_t I2C_ADDR = 0x68;
  static inline uint8_t dec2bcd(uint8_t n) __attribute__((always_inline)) { return n + 6*(n / 10); }
  static inline uint8_t bcd2dec(uint8_t n) __attribute__((always_inline)) { return n - 6*(n / 16); }
};

extern DS3232RTC RTC;

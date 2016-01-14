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

#include <DS3232RTC.h>
#include <Wire.h>

/*----------------------------------------------------------------------*
 * Constructor.                                                         *
 *----------------------------------------------------------------------*/
DS3232RTC::DS3232RTC() {
  Wire.begin();
}

/*----------------------------------------------------------------------*
 * Reads the current time from the RTC and stores it in a time_t value. *
 * Returns true if successful.                                          *
 * Assumes 24hr mode and that the Century bit is 0.                     *
 * If using a DS1307, assumes that the oscillator is running.           *
 *----------------------------------------------------------------------*/
bool DS3232RTC::get(time_t *t) {
  tmElements_t tm;

  if (get(&tm)) {
    *t = makeTime(tm);
    return true;
  } else {
    return false;
  }
}

/*----------------------------------------------------------------------*
 * Reads the current time from the RTC and returns it in a tmElements_t *
 * structure. Returns true if successful.                               *
 * Assumes 24hr mode and that the Century bit is 0.                     *
 * If using a DS1307, assumes that the oscillator is running.           *
 *----------------------------------------------------------------------*/
bool DS3232RTC::get(tmElements_t *tm) { 
  uint8_t *ptr = (uint8_t*)tm;
  if (read(RTC_SECONDS, ptr, tmNbrFields) == tmNbrFields) {
    for (uint8_t i = 0; i < tmNbrFields; i++) {
      ptr[i] = bcd2dec(ptr[i]);
    }
    tm->Year = y2kYearToTm(tm->Year);
    return true;
  } else {
    return false;
  }
}

/*----------------------------------------------------------------------*
 * Sets the RTC to the given time_t value.                              *
 * Returns true if successful.                                          *
 *----------------------------------------------------------------------*/
bool DS3232RTC::set(time_t t) {
  tmElements_t tm;
  breakTime(t, tm);
  return set(tm);
}

/*----------------------------------------------------------------------*
 * Sets the RTC's time from a tmElements_t structure.                   *
 * Returns true if successful.                                          *
 *----------------------------------------------------------------------*/
bool DS3232RTC::set(tmElements_t tm) {
  tm.Year = tmYearToY2k(tm.Year);

  uint8_t *ptr = (uint8_t*)&tm;
  for (uint8_t i = 0; i < tmNbrFields; i++) {
    ptr[i] = dec2bcd(ptr[i]);
  }

  return write(RTC_SECONDS, ptr, tmNbrFields);
}

/*----------------------------------------------------------------------*
 * Write a single byte to RTC RAM.                                      *
 * Valid address range is 0x00 - 0xFF, no checking.                     *
 * Returns true if successful.                                          *
 *----------------------------------------------------------------------*/
bool DS3232RTC::write(register_t addr, uint8_t value) {
  return write(addr, &value, 1);
}

/*----------------------------------------------------------------------*
 * Write multiple bytes to RTC RAM.                                     *
 * Valid address range is 0x00 - 0xFF, no checking.                     *
 * Number of bytes (n) must be between 1 and 31 (Wire library           *
 * limitation).                                                         *
 * Returns true if successful.                                          *
 *----------------------------------------------------------------------*/
bool DS3232RTC::write(register_t addr, uint8_t *values, uint8_t n) {
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(addr);
  for (uint8_t i = 0; i < n; i++) Wire.write(values[i]);
  return Wire.endTransmission() == 0;
}

/*----------------------------------------------------------------------*
 * Read a single byte from RTC RAM.                                     *
 * Valid address range is 0x00 - 0xFF, no checking.                     *
 * Returns the byte of data at @addr                                    *
 *----------------------------------------------------------------------*/
uint8_t DS3232RTC::read(register_t addr) {
  uint8_t b = 0;
  read(addr, &b, 1);
  return b;
}

/*----------------------------------------------------------------------*
 * Read multiple bytes from RTC RAM.                                    *
 * Valid address range is 0x00 - 0xFF, no checking.                     *
 * Number of bytes (n) must be between 1 and 32 (Wire library           *
 * limitation).                                                         *
 * Returns the number of bytes read.                                    *
 *----------------------------------------------------------------------*/
uint8_t DS3232RTC::read(register_t addr, uint8_t *values, uint8_t n) {
  uint8_t r = Wire.requestFrom(RTC_ADDR, n, addr, 1, true);

  for (uint8_t i = 0; i < r; i++) values[i] = Wire.read();
  return r;
}

bool DS3232RTC::getAlarm(alarm_t alarmNumber, alarmTypes_t *alarmType, almElements_t *tm) {
  uint8_t *ptr = (uint8_t*)tm;

  switch (alarmNumber) {
    case ALARM_1:
      if (!read(ALM1_SECONDS, ptr, almNbrFields) == almNbrFields) return false;
      break;

    case ALARM_2:
      tm->Second = 0;
      if (!read(ALM2_MINUTES, ptr+1, almNbrFields-1) == almNbrFields-1) return false;
      break;

    default:
      return false;
  }

  *alarmType = MATCH_ANY;

  for (uint8_t i = 0; i < almNbrFields; i++) {
    if (ptr[i] & _BV(ALARM_MASK)) {
      *alarmType = (alarmTypes_t)(*alarmType | _BV(i));
    }
  }

  if (ptr[almDayDate] & _BV(DYDT_FLAG)) {
    *alarmType = (alarmTypes_t)(*alarmType | MATCH_DAY);
  }

  return true;
}

/*----------------------------------------------------------------------*
 * Set an alarm time. Sets the alarm registers only.  To cause the      *
 * INT pin to be asserted on alarm match, use alarmInterrupt().         *
 * This method can set either Alarm 1 or Alarm 2, depending on the      *
 * value of alarmNumber (use a value from the alarm_t enumeration).     *
 * Returns true if successful.                                          *
 *----------------------------------------------------------------------*/
bool DS3232RTC::setAlarm(alarm_t alarmNumber, alarmTypes_t alarmType, almElements_t tm) {
  uint8_t *ptr = (uint8_t*)&tm;
  for (uint8_t i = 0; i < almNbrFields; i++) {
    ptr[i] = bcd2dec(ptr[i]);
  }

  switch (alarmType) {
    case MATCH_ANY:
      tm.Second  |= _BV(ALARM_MASK);
    case MATCH_SECONDS:
      tm.Minute  |= _BV(ALARM_MASK);
    case MATCH_MINUTES:
      tm.Hour    |= _BV(ALARM_MASK);
    case MATCH_HOURS:
      tm.DayDate |= _BV(ALARM_MASK);
    case MATCH_DAY:
      tm.DayDate |= _BV(DYDT_FLAG);
    case MATCH_DATE:
    default:
      break;
  }
  
  switch (alarmNumber) {
    case ALARM_1:
      return write(ALM1_SECONDS, ptr, almNbrFields);

    case ALARM_2:
      return write(ALM2_MINUTES, ptr+1, almNbrFields-1);

    default:
      return false;
  }
}

/*----------------------------------------------------------------------*
 * Enable or disable an alarm "interrupt" which asserts the INT pin     *
 * on the RTC.                                                          *
 * Returns true if successful.                                          *
 *----------------------------------------------------------------------*/
bool DS3232RTC::alarmInterrupt(alarm_t alarmNumber, bool interruptEnabled) {
  uint8_t controlReg = read(RTC_CONTROL),
          mask = _BV(A1IE) << alarmNumber;

  if (interruptEnabled && !(controlReg & mask))
    return write(RTC_CONTROL, controlReg | mask);
  else if (!interruptEnabled && (controlReg & mask))
    return write(RTC_CONTROL, controlReg & ~mask);
  else
    return true;
}

/*----------------------------------------------------------------------*
 * Returns true or false depending on whether the given alarm has been  *
 * triggered, and resets the alarm flag bit.                            *
 *----------------------------------------------------------------------*/
bool DS3232RTC::alarm(alarm_t alarmNumber, bool clearAlarm) {
  uint8_t statusReg = read(RTC_STATUS),
          mask = _BV(A1F) << alarmNumber;

  if (statusReg & mask) {
    if (clearAlarm) write(RTC_STATUS, statusReg & ~mask);
    return true;
  } else {
    return false;
  }
}

/*----------------------------------------------------------------------*
 * Enable or disable the square wave output.                            *
 * Use a value from the sqwaveFreqs_t enumeration for the argument.     *
 * Returns true if successful.                                          *
 *----------------------------------------------------------------------*/
bool DS3232RTC::squareWave(sqwaveFreqs_t freq) {
  uint8_t controlReg = read(RTC_CONTROL),
          mask = 0b11100011; // mask all but rate select and interrupt control

  if ((freq >= SQWAVE_NONE) && !(controlReg & _BV(INTCN)))
    return write(RTC_CONTROL, controlReg | _BV(INTCN));
  else if ((freq < SQWAVE_NONE) && ((controlReg & mask) != (freq << RS1)))
    return write(RTC_CONTROL, (controlReg & mask) | (freq << RS1));
  else
    return true;
}

/*----------------------------------------------------------------------*
 * Returns the value of the oscillator stop flag (OSF) bit in the       *
 * control/status register which indicates that the oscillator is or    *
 * was stopped, and that the timekeeping data may be invalid.           *
 * Optionally clears the OSF bit depending on the argument passed.      *
 *----------------------------------------------------------------------*/
bool DS3232RTC::oscStopped(bool clearOSF) {
  uint8_t s = read(RTC_STATUS);
  
  if (s & _BV(OSF)) {
    if (clearOSF) write(RTC_STATUS, s & ~_BV(OSF));
    return true;
  } else {
    return false;
  }
}

/*----------------------------------------------------------------------*
 * Returns the temperature in Celsius times four.                       *
 *----------------------------------------------------------------------*/
int16_t DS3232RTC::temperature() {
  uint16_t temp;
  if (read(TEMP_MSB, (uint8_t*)&temp, 2) != 2) return false;
  return temp / 64;
}

DS3232RTC RTC;
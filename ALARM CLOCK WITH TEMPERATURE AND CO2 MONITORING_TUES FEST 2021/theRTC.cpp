#include <Arduino.h>
#include <Wire.h>
// Libraries: real-time clock and calendar ( Library: DS3231, by Andrew Wickert, version 1.0.7 )
#include <DS3231.h>
// project includes
#include "hwconfig.h"
#include "theData.h"
// own declarations
#include "theRTC.h"

// real-time clock and calendar
static DS3231 rtc;

// timestamp last called
static unsigned long timer = 0;

// internal routines
static void process_theRTC_readDate(void);
static void process_theRTC_readTime(void);
static inline int adjust(const int value, const int min, const int max, const bool increment);

//----------------------------------------------------------

// time and date - we need it in order to adjust the values easily
static struct {
  int year;
  int month;
  int day;
  int dow;
  int hour;
  int minute;
  int seconds;
} myDateTime;

// initialization - called once at the device start
void theRTC_init(void)
{
  WIRE_RTC.begin();
  WIRE_RTC.setClock(SPEED_RTC);
}

static void process_theRTC_readDate(void)
{
  // read the date from RTC
  const int year = rtc.getYear();
  bool century = false;
  const int month = rtc.getMonth(century);
  const int day = rtc.getDate();
  const int dow = rtc.getDoW();

  // store the values for adjustments
  myDateTime.year = year;
  myDateTime.month = month;
  myDateTime.day = day;
  myDateTime.dow = dow;

  // report the readed out values to theDisplay
  theData_reportRTC_date(year, month, day, dow);
}

static void process_theRTC_readTime(void)
{
  // read the time from RTC
  bool h12, pm;
  const int hr = rtc.getHour(h12, pm);
  const int hour = ( h12 && pm && (hour != 12) ) ? (hr + 12) : hr;
  const int minute = rtc.getMinute();
  const int seconds = rtc.getSecond();

  // store the time for adjustments
  myDateTime.hour = hour;
  myDateTime.minute = minute;
  myDateTime.seconds = seconds;

  // report the readed out values to theDisplay
  theData_reportRTC_time(hour, minute, seconds);
}


// periodic function, called pretty fast, so we have to take
// care execute it with specific periodicy
void theRTC_process(const unsigned long timestamp)
{
  // if the time since last execution exceeds specified period
  if ( ( timestamp - timer ) >= PERIOD_RTC ) 
  {
    process_theRTC_readDate();
    process_theRTC_readTime();

    // remember when the function was executed last time
    timer = timestamp;
  }
}

static inline int adjust(const int value, const int min, const int max, const bool increment)
{
  if ( increment )
  {
    return (value < max) ? (value + 1) : (min);
  }
  return (value > min) ? (value - 1) : (max);
}

// For the time/date adjustment, we will just change the value
// in the RTC module, and then the new value will be read-out
// from RTC module and presented on display

void theRTC_adjust_year(const bool increment)
{
  rtc.setYear(adjust(myDateTime.year, 0, 99, increment));
}

void theRTC_adjust_month(const bool increment)
{
  rtc.setMonth(adjust(myDateTime.month, 1, 12, increment));
}

void theRTC_adjust_day(const bool increment)
{
  int max = 31; // most of months are 31 day

  switch(myDateTime.month) {
  case 2: // February - let's consider all the years to be leap
    max = 29;
    break;
  case 4: // April     = 30 days
  case 6: // June      = 30 days
  case 9: // September = 30 days
  case 11:// November  = 30 days
    max = 30;
    break;
  }

  rtc.setDate(adjust(myDateTime.day, 1, max, increment));
}

void theRTC_adjust_day_of_week(const bool increment)
{
  rtc.setDoW(adjust(myDateTime.dow, 1, 7, increment));
}

void theRTC_adjust_hour(const bool increment)
{
  rtc.setHour(adjust(myDateTime.hour, 0, 23, increment));
}

void theRTC_adjust_minute(const bool increment)
{
  rtc.setMinute(adjust(myDateTime.minute, 0, 59, increment));
}

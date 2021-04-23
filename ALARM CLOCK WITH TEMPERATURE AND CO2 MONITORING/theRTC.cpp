#include <Arduino.h>
#include <Wire.h>
// Libraries: real-time clock and calendar ( Library: DS3231, by Andrew Wickert, version 1.0.7 )
#include <DS3231.h>

#include "hwconfig.h"
#include "theData.h"

#include "theRTC.h"


static DS3231 rtc;


static unsigned long timer = 0;


static void process_theRTC_readDate(void);
static void process_theRTC_readTime(void);
static inline int adjust(const int value, const int min, const int max, const bool increment);


static struct {
  int year;
  int month;
  int day;
  int dow;
  int hour;
  int minute;
  int seconds;
} myDateTime;


void theRTC_init(void)
{
  WIRE_RTC.begin();
  WIRE_RTC.setClock(SPEED_RTC);
}

static void process_theRTC_readDate(void)
{

  const int year = rtc.getYear();
  bool century = false;
  const int month = rtc.getMonth(century);
  const int day = rtc.getDate();
  const int dow = rtc.getDoW();


  myDateTime.year = year;
  myDateTime.month = month;
  myDateTime.day = day;
  myDateTime.dow = dow;

  theData_reportRTC_date(year, month, day, dow);
}

static void process_theRTC_readTime(void)
{

  bool h12, pm;
  const int hr = rtc.getHour(h12, pm);
  const int hour = ( h12 && pm && (hour != 12) ) ? (hr + 12) : hr;
  const int minute = rtc.getMinute();
  const int seconds = rtc.getSecond();


  myDateTime.hour = hour;
  myDateTime.minute = minute;
  myDateTime.seconds = seconds;


  theData_reportRTC_time(hour, minute, seconds);
}



void theRTC_process(const unsigned long timestamp)
{

  if ( ( timestamp - timer ) >= PERIOD_RTC ) 
  {
    process_theRTC_readDate();
    process_theRTC_readTime();


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
  int max = 31; 

  switch(myDateTime.month) {
  case 2: 
    max = 29;
    break;
  case 4: 
  case 6: 
  case 9: 
  case 11:
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

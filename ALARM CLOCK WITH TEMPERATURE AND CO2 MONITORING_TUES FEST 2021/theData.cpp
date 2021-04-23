#include <Arduino.h>
#include <Wire.h>
// Libraries: flash memory storage ( Library: DueFlashStorage, by Sebastian Nilsson, version 1.0.0 )
#include <DueFlashStorage.h>
// project includes
#include "hwconfig.h"
#include "theRTC.h"
#include "theBuzzer.h"
// own declarations
#include "theData.h"

DueFlashStorage storage;

// in order to not reference the DS18B20 library
#define INVALID_TEMPERATURE     (-7040)
// these 2 routines have forward declaraions below
//static float toCelsius(int16_t raw);
//static float toFahrenheit(int16_t raw);

// timestamp last called
static unsigned long timer_flash = 0;
static unsigned long timer_blink = 0;

typedef enum {
  adj_none,
  adj_year,
  adj_month,
  adj_day,
  adj_dow,
  adj_hour,
  adj_minute,
  adj_alarm_enable,
  adj_alarm_hour,
  adj_alarm_minute,
  adj_max
} blink_element_t;

// what are we currently adjusting - we will start from 'no adjustment is active'
static blink_element_t blink_element = adj_none;

#define DAYS_OF_WEEK      7
#define MONTHS            12

#define DOW_LEN           3
#define MNS_LEN           3
#define CO2_LEN           5

#define DATE_LEN          16
#define TIME_LEN          16

// some useful constants for date
static const char cstrDayOfWeek[DOW_LEN * (DAYS_OF_WEEK + 1) + 1] = { "???MonTueWedThuFriSatSun" };
static const char cstrMonths   [MNS_LEN * (MONTHS       + 1) + 1] = { "???JanFebMarAprMayJunJulAugSepOctNovDec" };

// date string representation
static const char cstrDate_failure[DATE_LEN + 1] = "---, -- --- 20--";
static char strDate[DATE_LEN + 1] = "---, -- --- 20--";

// time string representation
static const char cstrTime_failure[TIME_LEN + 1] = "--:--";
static char strTime[TIME_LEN + 1] = "--:--";

// flashing dot in the clock
static bool flashing_dot = false;

// alarm representation
static const char *const cstrAlarmEnabled[2] = { "--OFF--", "-alarm-" };
static char strAlarm[TIME_LEN + 1] = "--:--";

// CO2 string representation
static const char* const cstrCO2_failure = "----";
static char strCO2[CO2_LEN + 1] = "     ";

// temperature sensors values
#define TEMP_LEN                7
static const char* const cstrTemp_failure = "-------";
static unsigned int reported_temp_count = 0;
static int16_t reported_temps[COUNT_TERMO];
static char strTemp[COUNT_TERMO][TEMP_LEN+1];

// here's what do we need to flash with adjusting value
static bool isFahrenheit = false;
static bool blink_adjustment = false;

// the 'alarm' data storage
static struct {
  bool enabled;
  uint8_t hour;
  uint8_t minute;
} alarm;

// internal routines - see description below
// configuration storing / reading
static void read_nvm_config(void);
static void write_nvm_alarm(void);
static void write_nvm_degrees(void);
// string manipulations
static void inline set_int(char* const pStr, const unsigned int pos, const unsigned int value, const char leadingZero);
static void inline set_char(char* const pStr, unsigned int pos, unsigned int count, const char space);
static void inline set_str(char *const pStr, const unsigned int pos, 
     char const* const pSubStr, const unsigned int size, const unsigned int value, const unsigned int max);
// string manipulations for temperature
static void set_string_temp(char* const pStr, const int16_t value, const bool isFahrenheitP);
// temperature conversions
static float toCelsius(int16_t raw);
static float toFahrenheit(int16_t raw);
// alarm string manipulations
static void theData_set_alarm_string(void);
// alarm adjustments
static void theData_alarm_enable(const bool increment);
static inline int adjust(const int value, const int min, const int max, const bool increment);
static void theData_alarm_hr(const bool increment);
static void theData_alarm_min(const bool increment);

//----------------------------------------------------------

// reads the configuration from non-volatile storage
// (alarm data and degrees representation)
// should be called once on startup
static void read_nvm_config(void)
{
  // we will use magic number approach to find out if there is real data
  // (after flash erase all the values will be 0xFF, and if we have something
  // already stored by us, we will set one extra byte to 'magic number' value,
  // and that is how we can find out if the stored data are actual)
  if ( storage.read(0) == MAGIC_NUMBER )
  {
    alarm.enabled = (storage.read(1) == NVM_TRUE);
    alarm.hour = storage.read(2);
    alarm.minute = storage.read(3);
  }

  if ( storage.read(4) == MAGIC_NUMBER )
  {
    isFahrenheit = (storage.read(5) == NVM_TRUE);
  }
}

// write the alarm data to non-volatile storage
static void write_nvm_alarm(void)
{
  storage.write(1, (alarm.enabled) ? (NVM_TRUE) : (NVM_FALSE) );
  storage.write(2, alarm.hour);
  storage.write(3, alarm.minute);
  storage.write(0, MAGIC_NUMBER);
}

static void write_nvm_degrees(void)
{
  storage.write(5, (isFahrenheit) ? (NVM_TRUE) : (NVM_FALSE) );
  storage.write(4, MAGIC_NUMBER);
}

// initialization - called once at the device start
void theData_init(void)
{
  theData_stopBlinker();      // we are starting from no adjustment is running
  // read the configuration from non-volatile memory: alarm and degree presentation (celsius/farenheit)
  read_nvm_config();
  // prepare the alarm presentation string
  theData_set_alarm_string(); // init the alarm string representation
  // all the termo sensors are "failure" before we read any data
  theData_reportTermo_sensorCount(0);
}

// periodic function, called pretty fast, so we have to take
// care execute it with specific periodicy
void theData_process(const unsigned long timestamp)
{
  // if adjustment is active - check the timer and change the blinking state
  if ( (blink_element != adj_none) && ( ( timestamp - timer_blink ) >= PERIOD_DISPLAY_BLINK ) )
  {
    blink_adjustment = !blink_adjustment;
    timer_blink = timestamp;
  }

  // if the time since last execution exceeds specified period - flash the dot in the clock
  if ( ( timestamp - timer_flash ) >= PERIOD_DISPLAY_FLASH ) 
  {
    flashing_dot = !flashing_dot;
    timer_flash = timestamp;
  }
}

void theData_reportCO2_value(const int value)
{
  // if we have received invalid value
  if ( ( value < 0 ) || (value > 9999) )
  {
    theData_reportCO2_failure();
    return;
  }
  sprintf(strCO2, "%d", value);
}

void theData_reportCO2_failure(void)
{
  memcpy(strCO2, cstrCO2_failure, CO2_LEN);
}

const char* const theData_getDisplay_CO2(void)
{
  return strCO2;
}

// helper function to set int value to string, changes 2 chars: [pos] and [pos+1]. 'leadingZero' is char to replace leading zero
static void inline set_int(char* const pStr, const unsigned int pos, const unsigned int value, const char leadingZero)
{
  pStr[pos + 0] = (value < 10) ? (leadingZero) : '0' + (value / 10);
  pStr[pos + 1] = '0' + (value % 10);
}

// helper function to set <count> chars in string <pStr> to <space> char
static void inline set_char(char* const pStr, unsigned int pos, unsigned int count, const char space)
{
  while(count--)
  {
    pStr[pos++] = space;
  }
}

// helper function to set month or day-of-week for appropriate value
static void inline set_str(char *const pStr, const unsigned int pos, char const* const pSubStr, const unsigned int size, const unsigned int value, const unsigned int max)
{
  // if value > max, we should use '0' index, otherwize - value
  // also we are multiplying by 'size' in order to get correct substring index
  const unsigned int pos_substr = ( (value > max) ? (0) : (value) ) * size;

  memcpy( &(pStr[pos]), &(pSubStr[pos_substr]), size);
}

void theData_reportRTC_date(const int year, const int month, const int day, const int dow)
{
  set_str(strDate, 0, cstrDayOfWeek, DOW_LEN, dow, 7);    // day of week
  set_int(strDate, 5, day, ' ');                          // day
  set_str(strDate, 8, cstrMonths,    MNS_LEN, month, 12); // month
  set_int(strDate, 14, year, '0');                        // year
}

void theData_reportRTC_time(const int hour, const int minute, const int seconds)
{
  static bool bAlarmReady = true;

  set_int(strTime, 0, hour,  '0');      // hour
  // flashing dot - handled in theData_getDisplay_getTime()
  set_int(strTime, 3, minute, '0');     // minute

  // if alarm is not enabled - it is never ready to proceed
  if ( ! alarm.enabled ) {
    bAlarmReady = false;
    return;
  }

  // if alarm is enabled - 
  if ( ( alarm.hour != hour ) || (alarm.minute != minute) )
  {
    bAlarmReady = true;
  }

  if ( ( alarm.hour == hour ) && (alarm.minute == minute) && ( bAlarmReady ) && ( ! theBuzzer_isBuzzing() ) ) 
  {
    bAlarmReady = false;
    theBuzzer_start();
  }
}

void theData_reportRTC_failure(void)
{
  memcpy(strDate, cstrDate_failure, DATE_LEN);
  memcpy(strTime, cstrTime_failure, TIME_LEN);
}

const char* const theData_getDisplay_getDate(void)
{
  if ( ( (int)blink_element < (int)adj_year ) || ( (int)blink_element > (int)adj_dow ) || ( ! blink_adjustment ) )
  {
    return strDate;
  }

  static char strBlinkDate[DATE_LEN + 1];
  memcpy(strBlinkDate, strDate, DATE_LEN + 1);

  switch ( blink_element ) {
  case adj_year:  set_char(strBlinkDate, 12, 4, ' '); break;
  case adj_month: set_char(strBlinkDate,  8, 3, ' '); break;
  case adj_day:   set_char(strBlinkDate,  5, 2, ' '); break;
  case adj_dow:   set_char(strBlinkDate,  0, 3, ' '); break;
  }

  return strBlinkDate;
}

const char *const theData_getDisplay_getTime(void)
{
  // flashing dot
  strTime[2] = (flashing_dot) ? (':') : (' ');

  if ( ( (int)blink_element < (int)adj_hour ) || ( (int)blink_element > (int)adj_minute ) || ( ! blink_adjustment ) )
  {
    return strTime;
  }

  static char strBlinkTime[TIME_LEN + 1];
  memcpy(strBlinkTime, strTime, TIME_LEN + 1);

  switch ( blink_element ) {
  case adj_hour:  set_char(strBlinkTime,  0, 2, ' '); break;
  case adj_minute:set_char(strBlinkTime,  3, 2, ' '); break;
  }

  return strBlinkTime;
}

const char *const theData_getDisplay_getAlarm(void)
{
  if ( blink_element == adj_alarm_enable )
  {
    if ( ! blink_adjustment ) {
      return cstrAlarmEnabled[(alarm.enabled) ? (1) : (0)];
    }
    return NULL;
  }

  if ( ! alarm.enabled ) return NULL;

  if ( ( ( blink_element != adj_alarm_hour ) && ( blink_element != adj_alarm_minute ) ) || ( ! blink_adjustment ) )
  {
    return strAlarm;
  }

  static char strBlinkAlarm[TIME_LEN + 1];
  memcpy(strBlinkAlarm, strAlarm, TIME_LEN + 1);

  switch ( blink_element ) {
  case adj_alarm_hour:  set_char(strBlinkAlarm,  0, 2, ' '); break;
  case adj_alarm_minute:set_char(strBlinkAlarm,  3, 2, ' '); break;
  }

  return strBlinkAlarm;
}

void theData_reportTermo_sensorCount(const unsigned int count)
{
  reported_temp_count = count;
  if ( reported_temp_count > COUNT_TERMO )  reported_temp_count = COUNT_TERMO;

  for(int i = reported_temp_count; i < COUNT_TERMO; i++ )
  {
    theData_reportTermo_failure(i);
  }
}

static void set_string_temp(char* const pStr, const int16_t value, const bool isFahrenheitP)
{
  const float temp = (isFahrenheitP) ? ( toFahrenheit(value) ) : ( toCelsius(value) );
  sprintf(pStr, "%.2f ", temp);
}

void theData_reportTermo_value(const unsigned int sensor, const int16_t value)
{
  if ( sensor >= COUNT_TERMO) return;

  reported_temps[sensor] = value;
  set_string_temp(strTemp[sensor], value, isFahrenheit);
}

void theData_reportTermo_failure(const unsigned int sensor)
{
  if ( sensor >= COUNT_TERMO) return;

  reported_temps[sensor] = INVALID_TEMPERATURE;
  memcpy(&(strTemp[sensor][0]), cstrTemp_failure, TEMP_LEN+1);
}

unsigned int theData_getDisplay_getTermoSensorsCount(void)
{
  return reported_temp_count;
}

const char* const theData_getDisplay_getTermoString(const unsigned int sensor, unsigned int *const type)
{
  if ( (sensor >= COUNT_TERMO) ) return cstrTemp_failure;

  if( reported_temps[sensor] == INVALID_TEMPERATURE )
  {
    *type = 2;
    return cstrTemp_failure;
  }

  *type = (isFahrenheit) ? (0) : (1);
  return strTemp[sensor];
}

bool theData_isCelsius(void)
{
  return (isFahrenheit == false);
}

void theData_setCelsius(const bool isCelsius)
{
  isFahrenheit = (isCelsius == false);
  for(unsigned int i = 0; i < reported_temp_count; i++ ) {
    theData_reportTermo_value(i, reported_temps[i]);
  }
  write_nvm_degrees();
}

// convert from raw to Celsius
static float toCelsius(int16_t raw) 
{
	// C = RAW/128
	return (float) raw * 0.0078125f;
}

// convert from raw to Fahrenheit
static float toFahrenheit(int16_t raw) 
{
	// F = (C*1.8)+32 = (RAW/128*1.8)+32 = (RAW*0.0140625)+32
	return ((float) raw * 0.0140625f) + 32.0f;
}

void theData_stopBlinker(void)
{
  blink_adjustment = false;
  blink_element = adj_none;
}

void theData_nextBlinker(void)
{
  blink_element = (blink_element_t)((int)blink_element + 1);

  const int max_element = (alarm.enabled) ? ((int)adj_alarm_minute) : ((int)adj_alarm_enable);

  if ( (int)blink_element > max_element )
  {
    theData_stopBlinker();
  }
}

static void theData_set_alarm_string(void)
{
  if ( ( alarm.hour < 0) || (alarm.hour > 23 ) || ( alarm.minute < 0 ) || (alarm.minute > 59) ) {
    alarm.hour = 23;
    alarm.minute = 59;
    alarm.enabled = false;
  }
  set_int(strAlarm, 0, alarm.hour,   '0');      // alarm hour
  set_int(strAlarm, 3, alarm.minute, '0');      // alarm minute
}

static void theData_alarm_enable(const bool increment)
{
  (void)increment;
  alarm.enabled = !alarm.enabled;
  write_nvm_alarm();
}

static inline int adjust(const int value, const int min, const int max, const bool increment)
{
  if ( increment )
  {
    return (value < max) ? (value + 1) : (min);
  }
  return (value > min) ? (value - 1) : (max);
}

static void theData_alarm_hr(const bool increment)
{
  alarm.hour = adjust(alarm.hour, 0, 23, increment);
  set_int(strAlarm, 0, alarm.hour,   '0');      // alarm hour
  write_nvm_alarm();
}

static void theData_alarm_min(const bool increment)
{
  alarm.minute = adjust(alarm.minute, 0, 59, increment);
  set_int(strAlarm, 3, alarm.minute, '0');      // alarm minute
  write_nvm_alarm();
}

void theData_nextValue(void)
{
  switch(blink_element) {

  case adj_year:        theRTC_adjust_year(true);       break;
  case adj_month:       theRTC_adjust_month(true);      break;
  case adj_day:         theRTC_adjust_day(true);        break;
  case adj_dow:         theRTC_adjust_day_of_week(true);break;
  case adj_hour:        theRTC_adjust_hour(true);       break;
  case adj_minute:      theRTC_adjust_minute(true);     break;
  case adj_alarm_enable:theData_alarm_enable(true);     break;
  case adj_alarm_hour:  theData_alarm_hr(true);         break;
  case adj_alarm_minute:theData_alarm_min(true);        break;

  }
}

void theData_prevValue(void)
{
  switch(blink_element) {

  case adj_year:        theRTC_adjust_year(false);        break;
  case adj_month:       theRTC_adjust_month(false);       break;
  case adj_day:         theRTC_adjust_day(false);         break;
  case adj_dow:         theRTC_adjust_day_of_week(false); break;
  case adj_hour:        theRTC_adjust_hour(false);        break;
  case adj_minute:      theRTC_adjust_minute(false);      break;
  case adj_alarm_enable:theData_alarm_enable(false);      break;
  case adj_alarm_hour:  theData_alarm_hr(false);          break;
  case adj_alarm_minute:theData_alarm_min(false);         break;

  }
}

bool theData_isAdjusting(void)
{
  return (blink_element != adj_none);
}

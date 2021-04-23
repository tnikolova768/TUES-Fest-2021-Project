#if !defined(__THE_CLOCK_THE_DATA_HEADER_INCLUDED_)
#define __THE_CLOCK_THE_DATA_HEADER_INCLUDED_

extern void theData_init(void);
extern void theData_process(const unsigned long timestamp);

// theCO2 module should report to us
extern void theData_reportCO2_value(const int value);
extern void theData_reportCO2_failure(void);

// theDisplay module should get the CO2 values for displaying
extern const char* const theData_getDisplay_CO2(void);

// theRTC module should report to us
extern void theData_reportRTC_date(const int year, const int month, const int day, const int dow);
extern void theData_reportRTC_time(const int hour, const int minute, const int seconds);
extern void theData_reportRTC_failure(void);

// theDisplay module should get the Date and Time values for displaying
extern const char* const theData_getDisplay_getDate(void);
extern const char* const theData_getDisplay_getTime(void);
extern const char* const theData_getDisplay_getAlarm(void);

// theTermo module should report to us
extern void theData_reportTermo_sensorCount(const unsigned int count);
extern void theData_reportTermo_value(const unsigned int sensor, const int16_t value);
extern void theData_reportTermo_failure(const unsigned int sensor);
extern bool theData_isCelsius(void);
extern void theData_setCelsius(const bool isCelsius);

// theDisplay module should get the sensor count, temperatures and its type for displaying
extern unsigned int theData_getDisplay_getTermoSensorsCount(void);
// here type will return '0' for Celsius, '1' for Fahrenheit, and '2' for failure state
extern const char* const theData_getDisplay_getTermoString(const unsigned int sensor, unsigned int *const type);

// theKeys will control the time/date/alarm adjustment through the following routines
extern void theData_stopBlinker(void);      // exit the adjustment mode
extern void theData_nextBlinker(void);      // start the adjustment mode or switch to next elemet for adjusting
extern void theData_nextValue(void);        // set the adjusting element to its next value (increment)
extern void theData_prevValue(void);        // set the adjusting element to its previous value (decrement)
extern bool theData_isAdjusting(void);      // check if we are currently in adjustment mode


#endif // __THE_CLOCK_THE_DATA_HEADER_INCLUDED_

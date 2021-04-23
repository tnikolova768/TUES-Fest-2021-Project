#if !defined(__THE_CLOCK_THE_DATA_HEADER_INCLUDED_)
#define __THE_CLOCK_THE_DATA_HEADER_INCLUDED_

extern void theData_init(void);
extern void theData_process(const unsigned long timestamp);


extern void theData_reportCO2_value(const int value);
extern void theData_reportCO2_failure(void);


extern const char* const theData_getDisplay_CO2(void);


extern void theData_reportRTC_date(const int year, const int month, const int day, const int dow);
extern void theData_reportRTC_time(const int hour, const int minute, const int seconds);
extern void theData_reportRTC_failure(void);


extern const char* const theData_getDisplay_getDate(void);
extern const char* const theData_getDisplay_getTime(void);
extern const char* const theData_getDisplay_getAlarm(void);

extern void theData_reportTermo_sensorCount(const unsigned int count);
extern void theData_reportTermo_value(const unsigned int sensor, const int16_t value);
extern void theData_reportTermo_failure(const unsigned int sensor);
extern bool theData_isCelsius(void);
extern void theData_setCelsius(const bool isCelsius);


extern unsigned int theData_getDisplay_getTermoSensorsCount(void);

extern const char* const theData_getDisplay_getTermoString(const unsigned int sensor, unsigned int *const type);

extern void theData_stopBlinker(void);      
extern void theData_nextBlinker(void);      
extern void theData_nextValue(void);        
extern void theData_prevValue(void);        
extern bool theData_isAdjusting(void);     


#endif // __THE_CLOCK_THE_DATA_HEADER_INCLUDED_

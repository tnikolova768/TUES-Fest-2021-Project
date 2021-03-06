#if !defined(__THE_CLOCK_THE_RTC_HEADER_INCLUDED_)
#define __THE_CLOCK_THE_RTC_HEADER_INCLUDED_

extern void theRTC_init(void);
extern void theRTC_process(const unsigned long timestamp);

// routines for adjusting the RTC (ds3231)
// when 'increment' is true, we are incrementing,
// when 'increment' is false, we are decrementing the value
extern void theRTC_adjust_year(const bool increment);
extern void theRTC_adjust_month(const bool increment);
extern void theRTC_adjust_day(const bool increment);
extern void theRTC_adjust_day_of_week(const bool increment);
extern void theRTC_adjust_hour(const bool increment);
extern void theRTC_adjust_minute(const bool increment);


#endif // __THE_CLOCK_THE_RTC_HEADER_INCLUDED_

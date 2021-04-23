#if !defined(__THE_CLOCK_THE_BUZZER_HEADER_INCLUDED_)
#define __THE_CLOCK_THE_BUZZER_HEADER_INCLUDED_

extern void theBuzzer_init(void);
extern void theBuzzer_process(const unsigned long timestamp);

extern void theBuzzer_start(void);
extern void theBuzzer_stop(void);
extern bool theBuzzer_isBuzzing(void);


#endif // __THE_CLOCK_THE_BUZZER_HEADER_INCLUDED_

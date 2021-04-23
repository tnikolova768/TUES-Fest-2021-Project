#if !defined(__THE_CLOCK_THE_HARDWARE_CONFIGURATION_HEADER_INCLUDED_)
#define __THE_CLOCK_THE_HARDWARE_CONFIGURATION_HEADER_INCLUDED_

// Libraries internal: PWM_Lib (https://github.com/antodom/pwm_lib)
// Libraries: CO2 sensor driver ( Library: MH-Z19, by Jonathan Dempsey, version 1.5.3 )
// Libraries: Display driver ( Library: Adafruit SH110x, by Adafruit, version 1.2.1 ) !!! +dependencies!!!
// Libraries: temperature sensors driver ( Library: Dallas Temperature, by Miles Burton, version 3.9.0 ) !!! +dependencies!!!
// Libraries: real-time clock and calendar ( Library: DS3231, by Andrew Wickert, version 1.0.7 )
// Libraries: flash memory storage ( Library: DueFlashStorage, by Sebastian Nilsson, version 1.0.0 )


#define STARTUP_DELAY         1000      


#define ONE_WIRE_BUS          (8)
#define BUZZER                (9)           
#define BUTTON_SET            (10)
#define BUTTON_PLUS           (11)
#define BUTTON_MINUS          (12)
#define LED_INTERNAL          (13)


#define BUZZER_PWM_PIN        PWML4_PC21    

#define BUZZER_FREQUENCY      (2700)        
#define BUZZER_PERIOD         ((100000000) / (BUZZER_FREQUENCY))
#define BUZZER_DUTY           (BUZZER_PERIOD / 2)     

#define ADDRESS_DISPLAY       (0x3C)       

#define SERIAL_CO2            Serial3      
#define WIRE_RTC              Wire          
#define WIRE_DISPLAY          Wire1         


#define COUNT_TERMO           (4)           


#define TEMP_MAX_ERRORS_BEFORE_REINIT   (10)


#define SPEED_CO2             (9600)       
#define SPEED_DISPLAY         (400000)      
#define SPEED_RTC             (400000)      


#define PERIOD_CO2            (1000)        
#define PERIOD_RTC            (500)         
#define PERIOD_TERMO_INIT     (1500)       
#define PERIOD_TERMO_REQUEST  (200)         
#define PERIOD_TERMO_READ     (100)       
#define PERIOD_DISPLAY_SHOW   (75)         
#define PERIOD_DISPLAY_FLASH  (500)      
#define PERIOD_DISPLAY_BLINK  (300)        
#define PERIOD_BEEP           (500)         
#define PERIOD_ALARM          (60000)       
#define PERIOD_LED            (1000)      

#define MAGIC_NUMBER          (0x55)        
#define NVM_TRUE              (0x01)        
#define NVM_FALSE             (0x02)


#endif // __THE_CLOCK_THE_HARDWARE_CONFIGURATION_HEADER_INCLUDED_

#if !defined(__THE_CLOCK_THE_HARDWARE_CONFIGURATION_HEADER_INCLUDED_)
#define __THE_CLOCK_THE_HARDWARE_CONFIGURATION_HEADER_INCLUDED_

// Libraries internal: PWM_Lib (https://github.com/antodom/pwm_lib)
// Libraries: CO2 sensor driver ( Library: MH-Z19, by Jonathan Dempsey, version 1.5.3 )
// Libraries: Display driver ( Library: Adafruit SH110x, by Adafruit, version 1.2.1 ) !!! +dependencies!!!
// Libraries: temperature sensors driver ( Library: Dallas Temperature, by Miles Burton, version 3.9.0 ) !!! +dependencies!!!
// Libraries: real-time clock and calendar ( Library: DS3231, by Andrew Wickert, version 1.0.7 )
// Libraries: flash memory storage ( Library: DueFlashStorage, by Sebastian Nilsson, version 1.0.0 )


#define STARTUP_DELAY         1000          // delay before hardware initialization

// used Arduino Pins list
#define ONE_WIRE_BUS          (8)
#define BUZZER                (9)           // not used from here, see BUZZER_PWM_PIN
#define BUTTON_SET            (10)
#define BUTTON_PLUS           (11)
#define BUTTON_MINUS          (12)
#define LED_INTERNAL          (13)

// buzzer PWM used, pin 8 = C.21 = PWML4
#define BUZZER_PWM_PIN        PWML4_PC21    // the actual PWM pin

#define BUZZER_FREQUENCY      (2700)          // 2.7 kHz = 2700 Hz
#define BUZZER_PERIOD         ((100000000) / (BUZZER_FREQUENCY))
#define BUZZER_DUTY           (BUZZER_PERIOD / 2)     // 50% duty cycle = 1/2 time from BUZZER_PERIOD

#define ADDRESS_DISPLAY       (0x3C)        // I2C Address for the display is 0x3C by default

// used Arduino communication list
#define SERIAL_CO2            Serial3       // use UART3
#define WIRE_RTC              Wire          // WARNING! Cannot be changed for RTC DS3231 Library!
#define WIRE_DISPLAY          Wire1         // Display

// temperature sensors count
#define COUNT_TERMO           (4)           // we expect to have 4 sensors

// after this much of attempts failed we will try to re-initialize the ds18b20 bus and sensors
#define TEMP_MAX_ERRORS_BEFORE_REINIT   (10)

// used communication speed list (baud rates)
#define SPEED_CO2             (9600)        // default communication speed of MH-Z19
#define SPEED_DISPLAY         (400000)      // should be changed inside the Library for Display
#define SPEED_RTC             (400000)      // 400k is working fine (change to 100k if any problems)

// period for executing the routines
#define PERIOD_CO2            (1000)        // every 1 sec should be fine
#define PERIOD_RTC            (500)         // every 0.5s should be good
#define PERIOD_TERMO_INIT     (1500)        // time needed for DS18b20 to init the bus and read the sensors
#define PERIOD_TERMO_REQUEST  (200)         // time needed for sent the request
#define PERIOD_TERMO_READ     (100)         // time between reading the sensors
#define PERIOD_DISPLAY_SHOW   (75)          // 75ms is ok, that will give us ~ 13fps
#define PERIOD_DISPLAY_FLASH  (500)         // 500ms ':' is flashing on the clock
#define PERIOD_DISPLAY_BLINK  (300)         // 300ms is blinking element on the clock
#define PERIOD_BEEP           (500)         // 500ms beep, 500ms silent
#define PERIOD_ALARM          (60000)       // 1 min alarm sound
#define PERIOD_LED            (1000)        // 1 second LED blink period

#define MAGIC_NUMBER          (0x55)        // magic number to see if the value in nvm is OK
#define NVM_TRUE              (0x01)        // just to vary from 0 and 1 values
#define NVM_FALSE             (0x02)


#endif // __THE_CLOCK_THE_HARDWARE_CONFIGURATION_HEADER_INCLUDED_

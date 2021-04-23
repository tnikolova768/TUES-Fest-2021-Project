
# TheClock

Small project based on **Arduino Due** : the clock, the alarm, the termometer, the CO2 meter.

## Part list:

* 1x Arduino Due 
* 1x OLED 128x64 SH1107 Display - I²C
* 4x DS18B20 temperature sensors - 1-Wire™
* 1x DS3231 real-time clock and calendar - I²C
* 1x MH-Z19 CO₂  sensor - UART
* 3x Hardware Buttons - GPIO
* 1x Buzzer PWM 2.7kHz controlled - PWM pin
* 1x internal LED - GPIO

## Software concept

The software is written with simpliest approach - "**linear modules invoking**". So, in the main file there's just an initialization of each module and a loop-function that takes current timestamp (milliseconds since the software starts) and unconditionally calls a single function from each and every module with this timestamp.

Each module called the<ModuleName> has both header (.h file) and source (.cpp file), it must have initialization function called the<ModuleName>_init() with no arguments, and periodical function called the<ModuleName>_process(unsigned long timestamp). That's the rule. If no initialization is needed for a module, there should be an empty initialization function, if no periodical function is needed - there should be an empty one inside, if no timestamp is needed, it still should be presented as an argument. That approach simplifies the maintenance, support and modifications.

The *_process function can trust that *_init function was called before its very first call.

The *_process function should be as less-time-consuming as it is possible.

If the *_process function should perform its actions once per XXX milliseconds, there should be a check, like the following:

```C
static unsigned long timer = 0;
...
if ( ( timestamp - timer ) >= MODULE_PERIODIC_TIME ) {
...
}
```

All the constants that could be changed one day (like pin assignments, timings for module, quantity of sensors, filters depth, etc.) should be placed in a single file for all modules, eg. hwconfig.h

However, it is allowed to place a very module-specific constants in the 

All the variables inside the module must be static, to provide a flexibility of namings in other modules. All the internal functions inside the module must be static, to provide a flexibility of namings in other modules.

The incapsulation concept must be followed strictly, if any variable in the module should be accessed from the outside, there should be appropriate getter- and setter- functions for it.

## Used libraries

* PWM_Lib - **included in the project**
* MH-Z19, by Jonathan Dempsey, version 1.5.3
* Adafruit SH110x, by Adafruit, version 1.2.1
  * Adafruit Gfx Library, by Adafruit, version 1.10.6 - **dependency**
* Dallas Temperature, by Miles Burton, version 3.9.0
  * OneWire, by Jim Studt, version 2.3.5 - **dependency**
* DS3231, by Andrew Wickert, version 1.0.7
* DueFlashStorage, by Sebastian Nilsson, version 1.0.0

## Modules description

**Note** All the modules have at least 2 interfaces to main project file (theClock.ino):

```
void the<ModuleName>_init(void);
void the<ModuleName>_process(const unsigned long timestamp);
```

It does not make sense to specify it for each and every module - it is exactly the same, so when "no interfaces" is specified, it means "no other except these 2".

### theLed

**Responsibility**:
The module is responsible for blinking the internal LED.

**Scheduling**
Routine is executed 1/20 of originally configured time (1000ms), so it is 50ms.

**Libraries**:
1. PWM_Lib - to use PWM 2.7kHz for Buzzer on specified pin (included internally)

**Tasks**:
1. if no alarm is active now, the LED should blinks once a second for 50ms.
2. if the alarm is active, the LED should blinks 50ms ON / 50ms OFF. *in real world due to time consumed by other modules it is not that strict, but it looks nice*

**Connectivity**:
1. theBuzzer - check if alarm is active

**interfaces**:
**(NONE)**

**Comments**
**(NONE)**

### theKeys

**Responsibility**:
The module is responsible for user input (3 buttons handling).

**Scheduling**
As far as other routines are taking some time to be executed, there's no need to have a schedule in the keys handling.

**Libraries**:
**(NONE)**

**Tasks**:
1. If button "Set" is pressed:
  * If alarm is active - stop the alarm
  * Forward the command for next adjusting element
2. If button "+" is pressed:
  * If alarm is active - stop the alarm
  * If we are NOT adjusting any value now (date/time/alarm settings) - switch between Celsius and Farenheit temperature representation
  * If we are adjusting any value now - change the it to next possible value (increment it).
3. If button "-" is pressed:
  * If alarm is active - stop the alarm
  * If we are NOT adjusting any value now (date/time/alarm settings) - switch between Celsius and Farenheit temperature representation
  * If we are adjusting any value now - change the it to previous possible value (decrement it).

**Connectivity**:
1. theBuzzer - check if alarm is active
2. theBuzzer - deactivate the alarm
3. theData - change the adjusting value
4. theData - change adjusting value to previous/next possible.
5. theData - change the temperature representation value (Celsius/Fahrenheit)

**Interfaces**:
**(NONE)**

**Comments**
The actions are taken only in case when previously (during previous routine call) the button was released, and now it is pressed.

### theCO2

**Responsibility**:
The module is responsible for reading the CO2 sensor (MH-Z19) and forward the data to theData module.

**Scheduling**
Every second, it does not make any sense to read the sensor out due to it is reading new value every 2 seconds.

**Libraries**:
1. MH-Z19, by Jonathan Dempsey, version 1.5.3

**Tasks**:
1. On schedule, read the CO2 value from sensor and provide it to data model (module theData)

**Connectivity**:
1. theData - provide new integer value of CO2 (in ppm) to the data model
1. theData - provide failure (that is actually zero value, but the separate interface routine is introduced for failures)

**Interfaces**:
**(NONE)**

**Comments**
**(NONE)**

### theBuzzer

**Responsibility**:
The module is responsible for Alarm, including time activities (alarm is activated for 60 seconds, with 500ms buzzing and 500ms silent).

**Scheduling**
When alarm is inactive, no actions are taken.
When alarm is active, every 500ms the buzzer status is changed between 'buzzing' and 'silent', and after 60sec since alarm is started it is deactivated.

**Libraries**:
* PWM_Lib (https://github.com/antodom/pwm_lib) - **included in the project**

**Tasks**:
1. If alarm is inactive, no actions should be taken.
2. If alarm is activated, store the timestamp in order to disable it after 60 seconds
3. every 500ms, change the buzzer status (buzzing / silent)
4. if alarm is deactivated by calling a interface function, deactivate the alarm.

**Connectivity**:
**(NONE)**

**Interfaces**:
```
void theBuzzer_start(void);
void theBuzzer_stop(void);
bool theBuzzer_isBuzzing(void);
```

**Comments**
**(NONE)**

### theDisplay

**Responsibility**:
The module is responsible for drawing all the data on the display.

**Scheduling**
Every 150ms display is redrawn - all the data is drawn from scratch using double-buffer provided by Adafruit libraries.

**Libraries**:
* Adafruit SH110x, by Adafruit, version 1.2.1
  * Adafruit Gfx Library, by Adafruit, version 1.10.6 - **dependency**

**Tasks**:
Receive all the inputs from data model (theData) and draw it on the display every 150ms, that gives us ~ 7fps (frames per second) refresh rate.

**Connectivity**:
1. theData - receive the date string
2. theData - receive the time string
3. theData - receive the alarm string
4. theData - receive the CO2 string
5. theData - receive the temperature sensors count
6. theData - receive the temperature sensor value for N sensors (N=4 in our case)

**Interfaces**:
**(NONE)**

**Comments**
All the magic with flashing dot in the clock, or flashing 'adjusting' value are happening in the data model. The dipslay module is only responsible for displaying the data.

### theRTC

**Responsibility**:
The module is responsible for reading and writing real-time clock and calendar.

**Scheduling**
* every 500ms the date and time is reading out from DS3231 hardware module

**Libraries**:
* DS3231, by Andrew Wickert, version 1.0.7

**Tasks**:
1. On the schedule, read the time and date out from the hardware.
2. When the appropriate adjusting function is called, increment or decrement the appropriate value (year/month/day/day-of-week/hour/minute).

**Connectivity**:
1. theData - report the time
2. theData - report the date

**Interfaces**:

```
void theRTC_adjust_year(const bool increment);
void theRTC_adjust_month(const bool increment);
void theRTC_adjust_day(const bool increment);
void theRTC_adjust_day_of_week(const bool increment);
void theRTC_adjust_hour(const bool increment);
void theRTC_adjust_minute(const bool increment);
```

**Comments**
* There is only one adjusting function per parameter, with an argument of 'increment'. It is set to 'true' when we need to increment the value, and 'false' for decrementing.
* The adjusting functions are changing the value in DS3231, and the new value will be received on the scheduled reading, no internal updates of variables are performed. This approach is a bit slower, but very robust and reliable - if you see the value on the display, you can be positively sure it was updated.
* No error reports are provided to data model. The DS3231 is on the I2C bus, and if something will happens with the hardware interface, the software will hangs. As far as it is the key feature of the device, it does not make sense to do any checking and make it up-and-running when we have issues with DS3231.

### theTermo

**Responsibility**:
The module is responsible for reading out the temperature sensors ds18b20 connected to OneWire bus

**Scheduling**
* Initialization time is 1.5seconds - time needed to initialize the OneWire bus, read the sensor's serial numbers and find out how many sensors are connected now. The time was found by experimental way.
* Conversion time is 200ms, it is the time needed to make the sensors do a temperature measures. The time was found by experimental way.
* Reading time is 100ms, it is the time required to transmit 9-bit temperature for one sensor. The time is 98ms, but it was rounded in order to have some extra fail-safety.

**Libraries**:
* Dallas Temperature, by Miles Burton, version 3.9.0
  * OneWire, by Jim Studt, version 2.3.5 - **dependency**

**Tasks**:
1. On initialization, start the OneWire and activate the device enumeration process.
2. After successful enumeration process, check if the temperature sensors connected, and report to data model (theData) its count.
3. Initiate the temperature conversion process for all sensors
4. Read all the sensors one-by-one (every 100ms one sensor read out) and report the values to data model (theData).
5. in case when not all the sensors have reported the temperature (failures on the bus), or there's less sensors than expected (4 in our case), after 10 reading-outs go to step 1 - re-initialize the OneWire bus.
6. if no errors occured, and the sensors are as many as expected (4 in our case), go to step 3.

**Connectivity**:
1. theData - report sensors count
2. theData - report sensor N value (in raw internal data)
3. theData - report sensor N failure

**Interfaces**:
**(NONE)**

**Comments**
* The module is implemented as state-machine, and it is one of the most complex modules in our system.
* All the Celsuis/Fahrenheit conversion is happening in the data model (theData).

### theData

**Responsibility**:
This is the Data Model. All the information is reported to this module, processed and prepared for processing/displaying/etc.

**Scheduling**
500ms for blinking the dot in Time
300ms for blinking the parameter if adjustment is active

**Libraries**:
* DueFlashStorage, by Sebastian Nilsson, version 1.0.0

**Tasks**:
* reads the alarm state and time on initialization
* reads the Celsius/Fahrenheit representation on initialization
* Stores the Alarm state (enable/disable) and alarm time in the NVM
* Stores the Celsius/Fahrenheit state in NVM
* receives the Date as integers, and provides it to theDisplay as string
* receives the Time as integer, and provides it to theDisplay as string with blinking dot
* receives the CO2 data in ppm as integer, and provides it to theDisplay as string
* receives the temperature sensors count and values in raw as integer, and provides it to theDisplay as string in Celsius or Fahrenheit, depends on the settings
* activates the alarm if it is enabled and the current time is equal to alarm time
* starts/switches/stops the parameter adjustment
* forwards the adjusting command (increment/decrement) to currently adjusting parameter adjuster (incrementer/decrementer)
* holds the alarm enabled/disabled, alarm hours and alarm minutes adjuster (incrementer/decrementer)
* calculates the raw ds18b20 values to Celsius or Fahrenheit, depends on settings

**Connectivity**:
1. theRTC - adjustment (increment/decrement) the year
1. theRTC - adjustment (increment/decrement) the month
1. theRTC - adjustment (increment/decrement) the day
1. theRTC - adjustment (increment/decrement) the day of week
1. theRTC - adjustment (increment/decrement) the hour
1. theRTC - adjustment (increment/decrement) the minute
2. theBuzzer - check if alarm is already active
3. theBuzzer - activate the alarm

**Interfaces**:

```
// theCO2 module should report to us
void theData_reportCO2_value(const int value);
void theData_reportCO2_failure(void);

// theDisplay module should get the CO2 values for displaying
const char* const theData_getDisplay_CO2(void);

// theRTC module should report to us
void theData_reportRTC_date(const int year, const int month, const int day, const int dow);
void theData_reportRTC_time(const int hour, const int minute, const int seconds);
void theData_reportRTC_failure(void);

// theDisplay module should get the Date and Time values for displaying
const char* const theData_getDisplay_getDate(void);
const char* const theData_getDisplay_getTime(void);
const char* const theData_getDisplay_getAlarm(void);

// theTermo module should report to us
void theData_reportTermo_sensorCount(const unsigned int count);
void theData_reportTermo_value(const unsigned int sensor, const int16_t value);
void theData_reportTermo_failure(const unsigned int sensor);
bool theData_isCelsius(void);
void theData_setCelsius(const bool isCelsius);

// theDisplay module should get the sensor count, temperatures and its type for displaying
unsigned int theData_getDisplay_getTermoSensorsCount(void);
// here type will return '0' for Celsius, '1' for Fahrenheit, and '2' for failure state
const char* const theData_getDisplay_getTermoString(const unsigned int sensor, unsigned int *const type);

// theKeys will control the time/date/alarm adjustment through the following routines
void theData_stopBlinker(void);      // exit the adjustment mode
void theData_nextBlinker(void);      // start the adjustment mode or switch to next elemet for adjusting
void theData_nextValue(void);        // set the adjusting element to its next value (increment)
void theData_prevValue(void);        // set the adjusting element to its previous value (decrement)
bool theData_isAdjusting(void);      // check if we are currently in adjustment mode
```

## Wiring diagram

![](Photo11-Working.jpg) 

* Display SH1107 128x64 OLED is connected to 3v3 power and I2C1 (pin numbers are not signed), no need to have external pull-ups due to it is soldered to display PCB.
* OneWire sensors are on pin 8 with 3v3 power. The resistor of around 5k is required between 3v3 and Data pins (1 for all the sensors)
* Buzzer is connected to 5v power line (due to high current consumption), and pin 9
* Buttons "Set"/"+"/"-" are connected to pins 10/11/12, debouncing capacitors and pull-up resistors are also recommended.
* MH-Z19 is connected to 5v power line (due to high current consumption), and UART3 (pins 14/15 for TX/RX)
* DS3231 is connected to 3v3 power, and main I2C (pin 21/20 for SCL/SDA), no need for pull-ups on I2C due to internal pull-ups on this interface.

![](Schematics.JPG) 

**THE END**

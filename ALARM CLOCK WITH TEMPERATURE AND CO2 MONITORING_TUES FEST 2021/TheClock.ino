#include <Arduino.h>
// Libraries: none
// Project includes
#include "hwconfig.h"     // hardware configuration - pins, speeds, buses, delays, timings, etc.
#include "theData.h"      // module that stored the data and provides it to display
#include "theRTC.h"       // Real Time Clock (DS3231) processing
#include "theCO2.h"       // CO2 sensor (MH-Z19) processing
#include "theTermo.h"     // Temperature sensors (DS18B20) processing
#include "theDisplay.h"   // Display (SH1107 OLED 128x64) processing
#include "theBuzzer.h"    // buzzer PWM 4.7kHz
#include "theKeys.h"      // buttons handling
#include "theLED.h"       // LED handling

// initialization - called once on device start
void setup() {
  // delay of STARTUP_DELAY = 1sec is recommended in order to display started correctly
  delay(STARTUP_DELAY);

  Serial.begin(115200);

  // initialization of all the used modules
  theData_init();
  theRTC_init();
  theCO2_init();
  theTermo_init();
  theDisplay_init();
  theBuzzer_init();
  theKeys_init();
  theLEDs_init();
}

// this function is called constantly by arduino framework core
void loop() {
  // all routines are time-scheduled, so it make sense to get ticks (1tick = 1ms)
  // once, and give it as parameter to all the functions
  const unsigned long timestamp = millis();

  // process all our modules one by one
  theData_process(timestamp);
  theRTC_process(timestamp);
  theCO2_process(timestamp);
  theTermo_process(timestamp);
  theDisplay_process(timestamp);
  theBuzzer_process(timestamp);
  theKeys_process(timestamp);
  theLEDs_process(timestamp);
}

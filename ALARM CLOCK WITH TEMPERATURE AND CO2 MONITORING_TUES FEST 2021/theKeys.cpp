#include <Arduino.h>
// Libraries: none
// project includes
#include "hwconfig.h"
#include "theData.h"
#include "theBuzzer.h"
// own declarations
#include "theKeys.h"

// previous key states
static bool btnSet = false;
static bool btnPlus = false;
static bool btnMinus = false;

// internal functions
static bool btnPressed(const int pin, bool &oldState);
static bool inline isStopBuzzer(void);

//----------------------------------------------------------

void theKeys_init(void)
{
  // all the buttons are INPUTs
  pinMode(BUTTON_SET, INPUT);
  pinMode(BUTTON_PLUS, INPUT);
  pinMode(BUTTON_MINUS, INPUT);
}

// check if the button was recently pressed
// (it IS when previous state was 'not pressed', and now it became 'pressed')
static bool btnPressed(const int pin, bool &oldState)
{
  // button is released - we have high level due to current flow over the resistor of 10k to input pin
  // button is pressed - we have low level due to current flow through the 10k resistor to GND
  bool newState = (digitalRead(pin) == LOW);
  // button state is not changed
  if( oldState == newState) return false;

  // remember the old state
  oldState = newState;
  // and return if it is pressed
  return newState;
}

// helper routine to stop buzzer by pressing any key if the alarm is active,
// it will return true if the button is handled here and no need to proceed
// with further processing for this button now
static bool inline isStopBuzzer(void)
{
  // if alarm is not active now - just return 'false' as key is not processed
  if ( ! theBuzzer_isBuzzing() ) return false;
  // if alarm is active, stop it and return 'true' as key is already processed
  theBuzzer_stop();
  return true;
}

void theKeys_process(const unsigned long timestamp)
{
  // we are not using timestamp now, so we will tell the compiler that we are aware of it
  (void)timestamp;

  // process "SET" button
  if ( btnPressed(BUTTON_SET, btnSet) )
  {
    // if the alarm was started - stop it, otherwise ...
    if ( ! isStopBuzzer() )
    // ... start adjustment or switch to next element
    {
      theData_nextBlinker();
    }
  }

  // process "+" button
  if ( btnPressed(BUTTON_PLUS, btnPlus) )
  {
    // if the alarm was started - stop it, otherwise ...
    if ( ! isStopBuzzer() ) 
    {
      // ... if currently we are adjusting clock/alarm - do it
      if ( theData_isAdjusting() ) 
      {
        theData_nextValue();
      }
      else
      // otherwise switch between Celsius and Farenheit
      {
        theData_setCelsius(!theData_isCelsius());
      }
    }
  }

  // process "-" button
  if ( btnPressed(BUTTON_MINUS, btnMinus) )
  {
    // if the alarm was started - stop it, otherwise ...
    if ( ! isStopBuzzer() ) 
    {
      // ... if currently we are adjusting clock/alarm - do it
      if ( theData_isAdjusting() ) 
      {
        theData_prevValue();
      }
      else
      // otherwise switch between Celsius and Farenheit
      {
        theData_setCelsius(!theData_isCelsius());
      }
    }
  }
}

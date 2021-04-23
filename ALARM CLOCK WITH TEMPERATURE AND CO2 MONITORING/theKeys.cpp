#include <Arduino.h>
// Libraries: none

#include "hwconfig.h"
#include "theData.h"
#include "theBuzzer.h"

#include "theKeys.h"


static bool btnSet = false;
static bool btnPlus = false;
static bool btnMinus = false;


static bool btnPressed(const int pin, bool &oldState);
static bool inline isStopBuzzer(void);


void theKeys_init(void)
{

  pinMode(BUTTON_SET, INPUT);
  pinMode(BUTTON_PLUS, INPUT);
  pinMode(BUTTON_MINUS, INPUT);
}


static bool btnPressed(const int pin, bool &oldState)
{
  
  bool newState = (digitalRead(pin) == LOW);

  if( oldState == newState) return false;


  oldState = newState;

  return newState;
}


static bool inline isStopBuzzer(void)
{
  if ( ! theBuzzer_isBuzzing() ) return false;
 
  theBuzzer_stop();
  return true;
}

void theKeys_process(const unsigned long timestamp)
{
  
  (void)timestamp;


  if ( btnPressed(BUTTON_SET, btnSet) )
  {
    
    if ( ! isStopBuzzer() )
   
    {
      theData_nextBlinker();
    }
  }


  if ( btnPressed(BUTTON_PLUS, btnPlus) )
  {

    if ( ! isStopBuzzer() ) 
    {
    
      if ( theData_isAdjusting() ) 
      {
        theData_nextValue();
      }
      else

      {
        theData_setCelsius(!theData_isCelsius());
      }
    }
  }


  if ( btnPressed(BUTTON_MINUS, btnMinus) )
  {

    if ( ! isStopBuzzer() ) 
    {

      if ( theData_isAdjusting() ) 
      {
        theData_prevValue();
      }
      else

      {
        theData_setCelsius(!theData_isCelsius());
      }
    }
  }
}

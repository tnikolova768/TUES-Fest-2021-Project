#include <Arduino.h>
// Libraries: none
// project includes
#include "hwconfig.h"
#include "theBuzzer.h"
// own declarations
#include "theLED.h"

// timestamp last called
static unsigned long timer = 0;
static unsigned int counter = 0;

#define LED_SUBPERIOD     20

//----------------------------------------------------------

void theLEDs_init(void)
{
  // the LED is OUTPUT
  pinMode(LED_INTERNAL, OUTPUT);
}

void theLEDs_process(const unsigned long timestamp)
{
  // if the time since last execution exceeds specified period
  if ( ( timestamp - timer ) >= (PERIOD_LED / LED_SUBPERIOD) ) 
  {
    if ( ++counter >= (LED_SUBPERIOD) ) counter = 0;
  
    if ( theBuzzer_isBuzzing() ) 
    {
      // if alarm is active, blink really fast [ 50ms ON / 50ms OFF ]
	  // (in real world, due to significant timings of handling the other
	  // routines, it will not look that smooth, but still it is a nice effect)
      digitalWrite(LED_INTERNAL, ((counter % 2) == 0) ? HIGH : LOW);
    }
    else
    {
      // if no alarm, blink [ 50ms ON / 950ms OFF ]
      digitalWrite(LED_INTERNAL, (counter == 0) ? HIGH : LOW);
    }

    // remember when the function was executed last time
    timer = timestamp;
  }
}

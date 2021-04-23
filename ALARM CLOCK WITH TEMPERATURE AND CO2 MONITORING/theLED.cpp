#include <Arduino.h>
// Libraries: none

#include "hwconfig.h"
#include "theBuzzer.h"

#include "theLED.h"


static unsigned long timer = 0;
static unsigned int counter = 0;

#define LED_SUBPERIOD     20

void theLEDs_init(void)
{
  
  pinMode(LED_INTERNAL, OUTPUT);
}

void theLEDs_process(const unsigned long timestamp)
{

  if ( ( timestamp - timer ) >= (PERIOD_LED / LED_SUBPERIOD) ) 
  {
    if ( ++counter >= (LED_SUBPERIOD) ) counter = 0;
  
    if ( theBuzzer_isBuzzing() ) 
    {

      digitalWrite(LED_INTERNAL, ((counter % 2) == 0) ? HIGH : LOW);
    }
    else
    {

      digitalWrite(LED_INTERNAL, (counter == 0) ? HIGH : LOW);
    }


    timer = timestamp;
  }
}

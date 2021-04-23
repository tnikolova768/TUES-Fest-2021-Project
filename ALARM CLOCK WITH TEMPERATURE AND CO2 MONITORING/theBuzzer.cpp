#include <Arduino.h>
#include <Wire.h>
// Libraries internal: PWM_Lib (https://github.com/antodom/pwm_lib)
#include "pwm_lib.h"
// project includes
#include "hwconfig.h"
// own declarations
#include "theBuzzer.h"

// PWM channel
arduino_due::pwm_lib::pwm<arduino_due::pwm_lib::pwm_pin::BUZZER_PWM_PIN> pwm_pin;

// timestamp that will be used for have 1/2 sec beep - 1/2 sec silent
// in case when alarm is enabled
static unsigned long timer = 0;
// timestamp that will be used for have 1 minute total interval
// for alarm sound (both 'beep' and 'silent')
static unsigned long timer_beep = 0;
// boolean flag to indicate whether alarm is currently active
// (this 1 minute with 1/2 sec on / 1/2 sec off part is on-running)
static bool bActive = false;
// boolean flag to indicate if we are currently in this
// 1/2 sec beep, or 1/2 sec silent part
static bool bBuzzing = false;

// internal routines - see description below
static inline void buzzer_beep(void);
static inline void buzzer_silent(void);
static void buzzer_do(void);

//----------------------------------------------------------

// initialization - called once at the device start
void theBuzzer_init(void)
{
  // make the pin output
  pinMode(BUZZER, OUTPUT);
  // set the level to LOW as initial value
  digitalWrite(BUZZER, LOW);
  // make sure the PWM is not running - in case if the routine will
  // be called from somewhere outside the initial setup
  theBuzzer_stop();
}

// internal routine
// run the PWM - make the buzzer beeping
static inline void buzzer_beep(void)
{
  pwm_pin.start(BUZZER_PERIOD, BUZZER_DUTY);
}

// internal routine
// stop the pwm - make the buzzer be silent
static inline void buzzer_silent(void)
{
  pwm_pin.stop();
}

// internal routine
// make the buzzer beeping or silent, depends of 'bBuzzing'
static void buzzer_do(void)
{
  if ( bBuzzing )
  {
    buzzer_beep();
  }
  else
  {
    buzzer_silent();
  }
}

// periodic function, called pretty fast, so we have to take
// care execute it with specific periodicy
void theBuzzer_process(const unsigned long timestamp)
{
  // nothing to do if buzzer is inactive
  if ( ! bActive ) {
    // no alarm active - just make sure all the timestamps
    // are set to current values
    timer = timestamp;
    timer_beep = timestamp;
    // alarm is inactive - nothing more to do here
    return;
  }

  // if the alarm is acttve and the time since last execution exceeds specified period (1/2 sec)
  if ( ( timestamp - timer ) >= PERIOD_BEEP )
  {
    // if we are already buzzing for PERIOD_ALARM (1 min?)
    if ( ( timestamp - timer_beep) >= PERIOD_ALARM )
    {
      // set the buzzer inactive
      theBuzzer_stop();
      // nothing more to do
      return;
    }

    // if we were beeping during last second - make silence,
    // if we were silent during last second - make it beeping.
    bBuzzing = !bBuzzing;
    buzzer_do();

    // remember when the function was executed last time
    timer = timestamp;
  }
}

// the routine to activate the alarm.
// should be called once, and during next 60 seconds
// the buzzer will activates for 1/2 sec on / 1/2 sec off.
// use theBuzzer_stop() to stop it,
// use theBuzzer_isBuzzing() to find out if the alarm is
// currently active.
void theBuzzer_start(void)
{
  bBuzzing = true;
  bActive = true;

  buzzer_do();
}

// stops the alarm - see theBuzzer_start() for more details
void theBuzzer_stop(void)
{
  bBuzzing = false;
  bActive = false;

  buzzer_do();
}

// check if the alarm is currently running - see theBuzzer_start() for details
bool theBuzzer_isBuzzing(void)
{
  return bActive;
}

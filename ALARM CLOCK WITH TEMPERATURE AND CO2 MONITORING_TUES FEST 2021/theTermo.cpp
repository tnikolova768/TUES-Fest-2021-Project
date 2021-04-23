#include <Arduino.h>
#include <Wire.h>
// Libraries: temperature sensors driver ( Library: Dallas Temperature, by Miles Burton, version 3.9.0 )
#include <OneWire.h>    // dependency lib
#include <DallasTemperature.h>
// project includes
#include "hwconfig.h"
#include "theData.h"
// own declarations
#include "theTermo.h"

// temperature sensor
static OneWire *pOneWire = NULL;
static DallasTemperature *pSensors = NULL;

// timestamp when the action was performed
static unsigned long timer = 0;
// milliseconds til next action - we have it as variable
// because it could vary depends on the state machine state
static unsigned long timer_period = PERIOD_TERMO_INIT;
// count of currently found sensors
static unsigned int count = 0;
// index of current sensor to read - we do not want to read all of it at once
// in order to spread the time of other devices to be blocked
static unsigned int current = 0;
// flag to request the timestamp for next operation
// (as a side effect of approach with single time for all the routines in main loop)
static bool bRequestTime = false;
// error flag - the data should be prepared before we will read it
static bool errorFlag = 0;
static unsigned int errorCount = 0;

// state machine states
typedef enum {
  state_init,     // actually, it is 'wait for init'
  state_request,  // actually, it is 'wait for request'
  state_read      // actually, it is 'wait for read'
} state_t;

// our state machine current state
static state_t state = state_init;

// internal routines - see details below
static void deinit(void);
static void read_sensor(const unsigned int sensor);
static unsigned int get_sensor_count(void);
static void inline set_state(const state_t stateP, const unsigned long periodP);
static void set_state_init(void);
static void set_state_request(void);
static void set_state_read(void);

//----------------------------------------------------------

static void deinit(void)
{
  if ( pSensors != NULL )
  {
    delete pSensors;
    pSensors = NULL;
  }

  if ( pOneWire != NULL )
  {
    delete pOneWire;
    pOneWire = NULL;
  }
}

// initialization - called once at the device start
void theTermo_init(void)
{
  deinit();

  pOneWire = new OneWire(ONE_WIRE_BUS);
  pSensors = new DallasTemperature(pOneWire);

  set_state_init();
}

// get the temperature raw value by sensor index on our bus. There are simple functions
// to read the value in Celsius or Fahrenheit in the library, but we can read only one
// at once, not both (we have to send request for conversion before next read). In order
// to use both C and F at the single read-out, we prefer to read the 'raw' value and do a
// conversion to either of degrees at our side, and there's no simple way to read the raw
// value in the library by sensor index, only by sensor address (aka serial number).
// So we have to do some trick with device address here.
// after successful/failed read, the result will be reported to theData.
static void read_sensor(const unsigned int sensor)
{
  DeviceAddress deviceAddress;
  if (!pSensors->getAddress(deviceAddress, sensor)) 
  {
    theData_reportTermo_failure(sensor);
    errorFlag = true;
  }
  else 
  {
    const int16_t temp = pSensors->getTemp(deviceAddress);
    theData_reportTermo_value(sensor, temp);
  }
}

// check the sensors count and report to theData if it was changed
static unsigned int get_sensor_count(void)
{
  const unsigned int new_count = pSensors->getDeviceCount();
  if ( new_count != count )
  {
    count = new_count;
    theData_reportTermo_sensorCount(count);
  }

  return count;
}

// change the state machine state and set the appropriate timer period
static void inline set_state(const state_t stateP, const unsigned long periodP)
{
  state = stateP;
  timer_period = periodP;
  bRequestTime = true;
}

// set the state machine state to 'wait init'
// and do the appropriate actions (reset 1-wire, check how many sensors are, read its serials,
// reset the error counter, reset the sensors count).
static void set_state_init(void)
{
  pSensors->setWaitForConversion(false);
  pSensors->begin();
  count = 0;
  theData_reportTermo_sensorCount(0);
  errorCount = 0;
  set_state(state_init, PERIOD_TERMO_INIT);
}

// set the 'wait conversion request' state,
// do appropriate actioins (request the temperature conversion, reset the current
// sensor, reset the error flag)
static void set_state_request(void)
{
  pSensors->requestTemperatures();
  set_state(state_request, PERIOD_TERMO_REQUEST);
  current = 0;
  errorFlag = false;
}

// periodic function, called pretty fast, so we have to take
// care execute it with specific periodicy
void theTermo_process(const unsigned long timestamp)
{
  // if the timestamp was requested, we will obtain it and do no other actions - return
  if ( bRequestTime )
  {
    timer = timestamp;
    bRequestTime = false;
    return;
  }

  // process only if the appropriate period (selected for current state machine state) is reached
  if ( ( timestamp - timer) >= timer_period )
  {
    // the state machine handler
    switch(state) {

    // we will enter this state on startup, or if the error occurs
    case state_init:
      // initialization was successful, we have sensors on the bus
      if ( get_sensor_count() > 0 )
      {
        // request a temperature conversion, and switch to state 'wait request'
        set_state_request();
        // ...and leave switch-case block
        break;
      }
      // otherwise, if we are still here, try to re-init (re-read) the 1-wire.
      // anyway, with 0 sensors there's nothing more to do.
      set_state_init();
      // ...and leave switch-case block
      break;

    // this state will be entered once during sensors reading
    case state_request:
      // the temperature convesion is performed, switch to 'read' state
      set_state(state_read, PERIOD_TERMO_READ);
      // ...and leave switch-case block
      break;

    // this state will be entered to read the sensors one-by-one,
    // and after switch to 'wait request' for another round,
    // or to 'wait init' in case of error
    case state_read:
      // is there any more sensors to read ?
      if ( current < count )
      {
        // read the sensor temperature and report it to theData
        read_sensor(current);
        // switch to next sensor for next time
        ++current;
        // and keep the current state (but with timer running again)
        set_state(state_read, PERIOD_TERMO_READ);
        // ...and leave switch-case block
        break;
      }

      // let's consider there was an error
      ++errorCount;
      // but if there was no error and we have at least expected amount of sensors - reset the error counter
      if ( ( count >= COUNT_TERMO) && ( ! errorFlag ) )  errorCount = 0;

      // if there were less reading errors than allowed in a raw (one after another)
      // (in other words if we should not re-intialize the sensors yet)
      if ( errorCount < TEMP_MAX_ERRORS_BEFORE_REINIT )
      {
        // sent the request, read the sensors one after another again,
        // in other words - send a conversion request and switch to 'wait request' state
        set_state_request();
        // ...and leave switch-case block
        break;
      }

      // if we are here, it means there were too many errors in a raw, so let's
      // reset the 1-wire bus and re-read the sensor configuration
      set_state_init();
      break;

    // unhandled state-machine states. we should never be here, but as a safety measure
    // we will reset the bus and set the valid state machine state ('wait init').
    default:
      set_state_init();
      break;

    }
  }
}

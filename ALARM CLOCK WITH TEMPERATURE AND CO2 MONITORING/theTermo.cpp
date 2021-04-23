#include <Arduino.h>
#include <Wire.h>
// Libraries: temperature sensors driver ( Library: Dallas Temperature, by Miles Burton, version 3.9.0 )
#include <OneWire.h>    // dependency lib
#include <DallasTemperature.h>

#include "hwconfig.h"
#include "theData.h"

#include "theTermo.h"


static OneWire *pOneWire = NULL;
static DallasTemperature *pSensors = NULL;


static unsigned long timer = 0;

static unsigned long timer_period = PERIOD_TERMO_INIT;

static unsigned int count = 0;

static unsigned int current = 0;

static bool bRequestTime = false;

static bool errorFlag = 0;
static unsigned int errorCount = 0;


typedef enum {
  state_init,     
  state_request,  
  state_read      
} state_t;


static state_t state = state_init;


static void deinit(void);
static void read_sensor(const unsigned int sensor);
static unsigned int get_sensor_count(void);
static void inline set_state(const state_t stateP, const unsigned long periodP);
static void set_state_init(void);
static void set_state_request(void);
static void set_state_read(void);


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


void theTermo_init(void)
{
  deinit();

  pOneWire = new OneWire(ONE_WIRE_BUS);
  pSensors = new DallasTemperature(pOneWire);

  set_state_init();
}


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

static void inline set_state(const state_t stateP, const unsigned long periodP)
{
  state = stateP;
  timer_period = periodP;
  bRequestTime = true;
}


static void set_state_init(void)
{
  pSensors->setWaitForConversion(false);
  pSensors->begin();
  count = 0;
  theData_reportTermo_sensorCount(0);
  errorCount = 0;
  set_state(state_init, PERIOD_TERMO_INIT);
}


static void set_state_request(void)
{
  pSensors->requestTemperatures();
  set_state(state_request, PERIOD_TERMO_REQUEST);
  current = 0;
  errorFlag = false;
}

void theTermo_process(const unsigned long timestamp)
{

  if ( bRequestTime )
  {
    timer = timestamp;
    bRequestTime = false;
    return;
  }

  
  if ( ( timestamp - timer) >= timer_period )
  {
  
    switch(state) {

   
    case state_init:
 
      if ( get_sensor_count() > 0 )
      {
        set_state_request();
       
        break;
      }
      
      set_state_init();
      
      break;

 
    case state_request:

      set_state(state_read, PERIOD_TERMO_READ);
   
      break;

 
    case state_read:
     
      if ( current < count )
      {

        read_sensor(current);

        ++current;
        
        set_state(state_read, PERIOD_TERMO_READ);
       
        break;
      }

 
      ++errorCount;
      
      if ( ( count >= COUNT_TERMO) && ( ! errorFlag ) )  errorCount = 0;

    
      if ( errorCount < TEMP_MAX_ERRORS_BEFORE_REINIT )
      {
        
        set_state_request();
       
        break;
      }


      set_state_init();
      break;

    
    default:
      set_state_init();
      break;

    }
  }
}

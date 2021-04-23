#include <Arduino.h>
#include <Wire.h>
// Libraries: CO2 sensor driver ( Library: MH-Z19, by Jonathan Dempsey, version 1.5.3 )
#include <MHZ19.h>
// project includes
#include "hwconfig.h"
#include "theData.h"
// own declarations
#include "theCO2.h"

// CO2 sensor
static MHZ19 mhz19;

// timestamp last processed, we need to process the data every 1.5 - 2 sec,
// there's no need to do it more often
static unsigned long timer = 0;

//----------------------------------------------------------

// initialization - called once at the device start
void theCO2_init(void)
{
  // run the serial port (uart) on specific port and specific speed (baud rate)
  SERIAL_CO2.begin(SPEED_CO2);
  // attach the serial communication to MH-Z19 sensor
  mhz19.begin(SERIAL_CO2);
}

// periodic function, called pretty fast, so we have to take
// care execute it with specific periodicy
void theCO2_process(const unsigned long timestamp)
{
  // if the time since last execution exceeds specified period
  if ( ( timestamp - timer ) >= PERIOD_CO2 ) 
  {
    // read the CO2 value in ppm (part-per-million)
    const int co2 = mhz19.getCO2();

    // if the value is OK - report value to theDisplay
    if ( co2 != 0 )
    {
      theData_reportCO2_value(co2);
    }
    else
    {
      // if the value is NOT OK - report failure to theDisplay
      theData_reportCO2_failure();
    }

    // remember when the function was executed last time
    timer = timestamp;
  }
}

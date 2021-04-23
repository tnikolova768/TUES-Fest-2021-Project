#include <Arduino.h>
#include <Wire.h>
// Libraries: Display driver ( Library: Adafruit SH110x, by Adafruit, version 1.2.1 )
#include <Adafruit_GFX.h>   // dependency lib
#include <Adafruit_SH110X.h>
// project includes
#include "hwconfig.h"
#include "theData.h"
// own declarations
#include "theDisplay.h"

// display class instance
static Adafruit_SH110X *pDisplay = NULL;

// our static functions
static void deinit(void);
static void theDisplay_showTime(void);
static void theDisplay_showDate(void);
static void theDisplay_showCO2(void);
static void theDisplay_showTermo(void);
static void theDisplay_showAlarm(void);

// timestamp last called
static unsigned long timer  = 0;

//----------------------------------------------------------

static void deinit(void)
{
  if ( pDisplay != NULL ) {
    delete pDisplay;
    pDisplay = NULL;
  }
}

// initialization - called once at the device start
void theDisplay_init(void)
{
  deinit();   // to avoid memory leaks

  WIRE_DISPLAY.begin(SPEED_DISPLAY);
  pDisplay = new Adafruit_SH110X(64, 128, &WIRE_DISPLAY);

  pDisplay->begin(ADDRESS_DISPLAY, true);
  pDisplay->clearDisplay();
  pDisplay->display();

  pDisplay->setRotation(1);
  pDisplay->setTextColor(SH110X_WHITE);
}

// periodic function, called pretty fast, so we have to take
// care execute it with specific periodicy
void theDisplay_process(const unsigned long timestamp)
{
  // if the time since last execution exceeds specified period
  if ( ( timestamp - timer ) >= PERIOD_DISPLAY_SHOW ) 
  {
    pDisplay->clearDisplay();

    theDisplay_showTime();
    theDisplay_showDate();
    theDisplay_showCO2();
    theDisplay_showTermo();
    theDisplay_showAlarm();

    pDisplay->display();

    // remember when the function was executed last time
    timer = timestamp;
  }
}

static void theDisplay_showTime(void)
{
  pDisplay->setCursor(2,18);
  pDisplay->setTextSize(2);
  pDisplay->print(theData_getDisplay_getTime());
  
  pDisplay->drawFastHLine(0, 38, 62, SH110X_WHITE);
}

static void theDisplay_showDate(void)
{
  pDisplay->setCursor(0,0);
  pDisplay->setTextSize(1);
  pDisplay->print(theData_getDisplay_getDate());

  pDisplay->drawFastHLine(0, 9, 128, SH110X_WHITE);
}

static void theDisplay_showCO2(void)
{
  pDisplay->setCursor(70,11);
  pDisplay->setTextSize(1);
  pDisplay->print("CO");
  pDisplay->setCursor(pDisplay->getCursorX(), pDisplay->getCursorY() + 4);
  pDisplay->print("2");
  pDisplay->setCursor(pDisplay->getCursorX(), pDisplay->getCursorY() - 4);
  pDisplay->print(": ");
  pDisplay->print(theData_getDisplay_CO2());
}

static void theDisplay_showTermo(void)
{
  static const char* const cstrDegree[2] = { "F", "C" };

  const unsigned int count = theData_getDisplay_getTermoSensorsCount();

  for( int i = 0; i < count; i++ )
  {
    unsigned int type = 3;
    pDisplay->setCursor(70, 25 + (10 * i));
    pDisplay->print(theData_getDisplay_getTermoString(i, &type));
    if( type < 2)
    {
      pDisplay->setCursor(pDisplay->getCursorX(), pDisplay->getCursorY() - 4);
      pDisplay->print("o");
      pDisplay->setCursor(pDisplay->getCursorX(), pDisplay->getCursorY() + 4);
      pDisplay->print(cstrDegree[type]);
    }
  }
}

static void theDisplay_showAlarm(void)
{
  const char* const pAlarm = theData_getDisplay_getAlarm();

  if ( pAlarm != NULL )
  {
    pDisplay->setCursor(10, 42);
    pDisplay->setTextSize(1);
    pDisplay->print(pAlarm);
  }
}

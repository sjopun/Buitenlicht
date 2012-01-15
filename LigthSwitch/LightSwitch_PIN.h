/*
 * LightSwitch.h
 * by sjopun
 *
 * Automatisch buitenlicht.  
 *
 * 
 */

#ifndef LightSwitch_h
#define LightSwitch_h

// analog inputs //
///////////////////
int PIN_LIGHT_SENSOR       = 14;    // ADC input pin for light measurement

int PIN_SENSOR_VOORDEUR    = 15;    // used as digital inputs
int PIN_SENSOR_PIR         = 16;
int PIN_SENSOR_GARAGE      = 17;
int PIN_SENSOR_SCHUTTING   = 18;
int PIN_SENSOR_KEUKEN      = 19; 
int PIN_SENSOR_GARAGE_TUIN = 11;    // MOSI

// digital input //
///////////////////
int PIN_TEST             = 2;   // switch all light on/off/active

// outputs //
/////////////
int PIN_NIGHT_INDICATOR  = 13;  // pin for the LED indicator, night is ON
 
int PIN_LAMP_VOOR        = 7;   // controls for 220V light
int PIN_LAMP_KEUKEN      = 6;
int PIN_LAMP_GARAGE      = 5;
int PIN_LAMP_TUIN        = 4;
int PIN_LAMP_ACHTER      = 3;

#endif

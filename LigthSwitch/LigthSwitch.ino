/*
 * LightSwitch.c
 * date: 4-Jan-2011
 * author: sjoerd punter
 *
 * Besturing voor automatisch buitenlicht.  
 * 
 *
 * 
 */

//#include "LightSwitch.h"
#include "LightSwitch_PIN.h"
#include "LightSwitch_DayNight.h";
#include "Timer_One.h"

/*
 * LightSwitch.ino
 * 10-Jan-2012
 *
 */

//boolean YES  = true;
//boolean NO   = false;
boolean ON   = true;
boolean OFF  = false;
boolean Tracing;


int LightToggleValue = 50;   // for BPX21 normal 40.
                             // zon = 90, kamer = 55, onder tafel=43
int IsNight          = false;
 
long          IntervalTime = 5000; // 5 seconds
unsigned long CurrentMillis;
unsigned long PreviousMillis;
unsigned long Previous_1Sec;


// const
int LIGHT_ON  = LOW;
int LIGHT_OFF = HIGH;

int TimeON_default = 36; // * 5 seconds = 3 minutes
int TimeON_test    =  2; // * 5 sec = 10 sec
// var
int TimeON         =  TimeON_default; 

// more vars
int SensorVoordeur       = OFF;
int SensorPIR            = OFF;
int SensorGarageAuto     = OFF;
int SensorSchutting      = OFF;
int SensorKeukenDeur     = OFF;
int SensorGarageTuinDeur = OFF;

// start with no time, lights off
int TimeLampVoor   = 0;
int TimeLampKeuken = 0;
int TimeLampGarage = 0;
int TimeLampTuin   = 0;
int TimeLampAchter = 0;


// setup is only run at power up
////////////////////////////////
void setup() {
 
//  Timer1.initialize(500000);         // initialize timer1, and set a 1/2 second period
  Timer1.initialize(1000000);         // initialize timer1, and set a 1 second period
  Timer1.pwm(9, 512);                // setup pwm on pin 9, 50% duty cycle
  Timer1.attachInterrupt( LED_flash );  // attaches callback() as a timer overflow interrupt
  
  
  // digital outputs //
  pinMode( PIN_NIGHT_INDICATOR,  OUTPUT);   // declare the ledPin as OUTPUT
  
  pinMode( PIN_LAMP_VOOR,        OUTPUT);   // declare the ledPin as OUTPUT
  pinMode( PIN_LAMP_KEUKEN,      OUTPUT);   // declare the ledPin as OUTPUT
  pinMode( PIN_LAMP_GARAGE,      OUTPUT);   // declare the ledPin as OUTPUT
  pinMode( PIN_LAMP_TUIN,        OUTPUT);   // declare the ledPin as OUTPUT
  pinMode( PIN_LAMP_ACHTER,      OUTPUT);   // declare the ledPin as OUTPUT
 
  // initial switch all lights off
  digitalWrite( PIN_LAMP_VOOR,   HIGH);
  digitalWrite( PIN_LAMP_KEUKEN, HIGH);
  digitalWrite( PIN_LAMP_GARAGE, HIGH);
  digitalWrite( PIN_LAMP_TUIN,    HIGH);
  digitalWrite( PIN_LAMP_ACHTER, HIGH);

  
  // test -------------------------
  digitalWrite( PIN_LAMP_VOOR,   LOW);
  delay( 100 );
  digitalWrite( PIN_LAMP_KEUKEN, LOW);
  delay( 100 );
  digitalWrite( PIN_LAMP_GARAGE, LOW);
  delay( 100 );
  digitalWrite( PIN_LAMP_TUIN,   LOW);
  delay( 100 );
  digitalWrite( PIN_LAMP_ACHTER, LOW);
  delay( 500 );
  // test -------------------------

  
  // ADC input, BPX61 photo diode //
  pinMode( PIN_LIGHT_SENSOR,     INPUT );
  
  // digital inputs, door switches
  pinMode( PIN_SENSOR_VOORDEUR,    INPUT );
  pinMode( PIN_SENSOR_PIR,         INPUT );
  pinMode( PIN_SENSOR_GARAGE,      INPUT );
  pinMode( PIN_SENSOR_SCHUTTING,   INPUT );
  pinMode( PIN_SENSOR_KEUKEN,      INPUT );  
  pinMode( PIN_SENSOR_GARAGE_TUIN, INPUT );
  

  // enable internal pull up resistor of the analog inputs
/*
  digitalWrite( PIN_SENSOR_VOORDEUR,    HIGH );
  digitalWrite( PIN_SENSOR_PIR,         HIGH );
  digitalWrite( PIN_SENSOR_GARAGE,      HIGH );
  digitalWrite( PIN_SENSOR_SCHUTTING,   HIGH );
  digitalWrite( PIN_SENSOR_KEUKEN,      HIGH );
*/  
  digitalWrite( PIN_SENSOR_GARAGE_TUIN, HIGH );  // $$$ for now, connect 330E to +5V

 
  Serial.begin( 9600 ); 
  Serial.flush();
  delay( 100 );
  Serial.println("Buitenlicht v0.7 (28-Oct-2011)");
  Serial.println("'?' voor help" );
  delay( 100 );
  Serial.flush();
  
} // setup end


// main program
void loop() 
{

  int ON_IS_LOW_VALUE  = 10;
  int ON_IS_HIGH_VALUE = 900;


  // read the inputs
  //////////////////
  if ( digitalRead( PIN_SENSOR_VOORDEUR )    == LOW ) { SensorVoordeur       = ON; } else { SensorVoordeur       = OFF; };
  if ( digitalRead( PIN_SENSOR_PIR )         == LOW ) { SensorPIR            = ON; } else { SensorPIR            = OFF; };
  if ( digitalRead( PIN_SENSOR_GARAGE )      == LOW ) { SensorGarageAuto     = ON; } else { SensorGarageAuto     = OFF; };
  if ( digitalRead( PIN_SENSOR_SCHUTTING )   == LOW ) { SensorSchutting      = ON; } else { SensorSchutting      = OFF; };
  if ( digitalRead( PIN_SENSOR_KEUKEN )      == LOW ) { SensorKeukenDeur     = ON; } else { SensorKeukenDeur     = OFF; };
  if ( digitalRead( PIN_SENSOR_GARAGE_TUIN ) == LOW ) { SensorGarageTuinDeur = ON; } else { SensorGarageTuinDeur = OFF; };
 
  if( IsNight == true ) // night
  {
    // check the sensor variable and set the time the lamp burns
    ////////////////////////////////////////////////////////////
    if ( SensorVoordeur       == ON ) { TimeLampVoor   = TimeON; };
    if ( SensorPIR            == ON ) { TimeLampVoor   = TimeON; };
    if ( SensorGarageAuto     == ON ) { TimeLampVoor   = TimeON; TimeLampGarage = TimeON; };
    if ( SensorSchutting      == ON ) { TimeLampVoor   = TimeON; TimeLampKeuken = TimeON; };
    if ( SensorKeukenDeur     == ON ) { TimeLampKeuken = TimeON; TimeLampTuin   = TimeON; TimeLampAchter = TimeON; };
    if ( SensorGarageTuinDeur == ON ) { TimeLampTuin   = TimeON; TimeLampGarage = TimeON; };
  }

  // switch light on or off, on when variable is > 0, off when variable is 0.
  // each variables is keeping the time in 5 seconds steps.
  // in the timing routine the value is decremented until zero
  //////////////////////////////////////////////////////////////////////////
  if ( TimeLampVoor   > 0 ) { digitalWrite( PIN_LAMP_VOOR,   LIGHT_ON ); } else { digitalWrite( PIN_LAMP_VOOR,   LIGHT_OFF ); }
  if ( TimeLampKeuken > 0 ) { digitalWrite( PIN_LAMP_KEUKEN, LIGHT_ON ); } else { digitalWrite( PIN_LAMP_KEUKEN, LIGHT_OFF ); }
  if ( TimeLampTuin   > 0 ) { digitalWrite( PIN_LAMP_GARAGE, LIGHT_ON ); } else { digitalWrite( PIN_LAMP_GARAGE, LIGHT_OFF ); }
  if ( TimeLampAchter > 0 ) { digitalWrite( PIN_LAMP_ACHTER, LIGHT_ON ); } else { digitalWrite( PIN_LAMP_ACHTER, LIGHT_OFF ); }
  if ( TimeLampGarage > 0 ) { digitalWrite( PIN_LAMP_TUIN,   LIGHT_ON ); } else { digitalWrite( PIN_LAMP_TUIN,   LIGHT_OFF ); }
 
 
  int DAY   = 1;
  int NIGHT = 2;

  // each 5 seconds the check for day or night is done.
  // also the count down of the light ON time is done here.
  /////////////////////////////////////////////////////////
  CurrentMillis = millis();  
  
  if ( (CurrentMillis - PreviousMillis) > IntervalTime )
  {
    PreviousMillis = CurrentMillis;   
   
   switch ( CheckDayOrNight( LightToggleValue ) )
   {
     case 1: //DAY:
       LED_off();
       IsNight        = false;
 
       // switch off the lights, it is day time
       TimeLampVoor   = 0;
       TimeLampKeuken = 0;
       TimeLampTuin   = 0;
       TimeLampAchter = 0;
       TimeLampGarage = 0;
       
       if ( Tracing == true )
       {
         Serial.println( "*** Dag" );
       }
   
     break;
     
     case 2: //NIGHT:
       LED_on();  // on board LED
       IsNight = true;
   
       // count down the ligth on time
       if ( TimeLampVoor   > 0 ) { TimeLampVoor   --; }
       if ( TimeLampKeuken > 0 ) { TimeLampKeuken --; }
       if ( TimeLampTuin   > 0 ) { TimeLampTuin   --; }
       if ( TimeLampAchter > 0 ) { TimeLampAchter --; }
       if ( TimeLampGarage > 0 ) { TimeLampGarage --; }
   
       if ( Tracing == true )
       {
         Serial.println( "@@@ Nacht" );
       }

     break;
     
     default:
       Serial.println( "ERROR: Day/Night Undefined" );
     break;
   } //switch end



  // for debugging the system
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
  if ( Tracing == true )
  {
    if ( TimeLampVoor   > 0 ) { Serial.print( "^ TimeLampVoor      " ); Serial.println( TimeLampVoor ); }
    if ( TimeLampKeuken > 0 ) { Serial.print( "^ TimeLampKeuken    " ); Serial.println( TimeLampKeuken ); }
    if ( TimeLampTuin   > 0 ) { Serial.print( "^ TimeLampTuin      " ); Serial.println( TimeLampTuin ); }
    if ( TimeLampAchter > 0 ) { Serial.print( "^ TimeLampAchter    " ); Serial.println( TimeLampAchter ); }
    if ( TimeLampGarage > 0 ) { Serial.print( "^ TimeLampGarage    " ); Serial.println( TimeLampGarage ); }
   
   
    if ( SensorVoordeur       == ON ) { Serial.print( ")( Voordeur         open " );   } else 
                                      { Serial.print( "   Voordeur        dicht " ); } 
                                        Serial.println( digitalRead( PIN_SENSOR_VOORDEUR )  );
    
    if ( SensorPIR            == ON ) { Serial.print( ")( PIR detect       open " ); } else 
                                      { Serial.print( "   PIR detect      dicht " ); }
                                        Serial.println( digitalRead( PIN_SENSOR_PIR )       );
    
    if ( SensorGarageAuto     == ON ) { Serial.print( ")( Garage Auto      open " ); } else 
                                      { Serial.print( "   Garage Auto     dicht " ); }
                                        Serial.println( digitalRead( PIN_SENSOR_GARAGE )    );
        
    if ( SensorSchutting      == ON ) { Serial.print( ")( Schutting        open " );  } else 
                                      { Serial.print( "   Schutting       dicht " ); }
                                        Serial.println( digitalRead( PIN_SENSOR_SCHUTTING ) );
   
    if ( SensorKeukenDeur     == ON ) { Serial.print( ")( Keuken           open " );     } else 
                                      { Serial.print( "   Keuken          dicht " ); }
                                        Serial.println( digitalRead( PIN_SENSOR_KEUKEN )    );
        
    if ( SensorGarageTuinDeur == ON ) { Serial.print( ")( Garage Tuindeur  open " );        } else 
                                      { Serial.print( "   Garage Tuindeur dicht " ); }
                                        Serial.println( digitalRead( PIN_SENSOR_VOORDEUR )  );

   } // end debug true
  
 } // end if IntervalTime
 
 
  // for debug and test

  char ReadCharacter;
  
  if ( Serial.available() )
  {
    ReadCharacter = Serial.read();
    Serial.flush();
    
    Serial.print( "> " );
    Serial.println( ReadCharacter );
  
    switch( ReadCharacter )
    {
      case '1':
       TimeLampVoor = TimeON;
       Serial.println( "Lamp voordeur" );
      break;
      
      case '2':
        TimeLampKeuken = TimeON;
        Serial.println( "Lamp keuken aan" );
      break;
      
      case '3':
        TimeLampGarage = TimeON;
        Serial.println( "Lamp achter aan" ); 
      break;
      
      case '4':
         TimeLampTuin = TimeON;
        Serial.println( "Lamp tuin aan" );     
      break; 
      
      case '5':
        TimeLampAchter = TimeON;
        Serial.println( "Lamp achter aan" );
      break;
      
      case 'A':
      case 'a':
         TimeLampVoor   = TimeON;
         TimeLampKeuken = TimeON;
         TimeLampTuin   = TimeON;
         TimeLampAchter = TimeON;
         TimeLampGarage = TimeON;
         Serial.println( "Alles aan" );
      break;  
             
      case '0':
         TimeLampVoor   = 0;
         TimeLampKeuken = 0;
         TimeLampTuin   = 0;
         TimeLampAchter = 0;
         TimeLampGarage = 0;
         LightToggleValue = 40;  // normal setting
         TimeON           = TimeON_default; // sec
         Serial.println( "Reset" );
      break;
      
      case 'N':
      case 'n':
        LightToggleValue = 250;  // force night setting
         Serial.println( "Nacht geforceerd" );
      break;
      
      case 'T':
      case 't':
        if (Tracing == false )
        { Serial.println( "Tracing ON" );
          Tracing = true;
        }
        else
        { Serial.println( "Tracing OFF" );
          Tracing = false;
        }
      break;
      
      
      case 'X':
      case 'x':
        Serial.println( "Aan tijd [1..2000 sec] " );
        TimeON = TimeON_test; // sec
       // TimeON = Serial.read();
        Serial.print( "  TimeON = ");
        Serial.println( TimeON );
       break;


      case '?':
     // default:
       // Serial.println( ReadCharacter );
       Serial.println( "  0)Reset T)racing N)acht toggle 1)Voor 2)Keuken 3)Tuin 4)Achter 5)Garage A)lles aan" );
      break;  
    }

  } // end if Serial.available()
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // loop



// code for indicator LED
/////////////////////////

void LED_flash( )
{
 
  if ( digitalRead(PIN_NIGHT_INDICATOR) == LOW )
  {
      digitalWrite( PIN_NIGHT_INDICATOR, HIGH );
  }
  else
  {
      digitalWrite( PIN_NIGHT_INDICATOR, LOW );
  }
  
 // digitalWrite( PIN_NIGHT_INDICATOR, LOW );
 //   digitalWrite( PIN_NIGHT_INDICATOR, digitalRead(PIN_NIGHT_INDICATOR) ^ 1);
  
} //end LED_flash

void LED_on( )
{
 
  digitalWrite( PIN_NIGHT_INDICATOR, LOW);
  
} //end LED_on

void LED_off( )
{
 
  digitalWrite( PIN_NIGHT_INDICATOR, HIGH);
  
} //end LED_on


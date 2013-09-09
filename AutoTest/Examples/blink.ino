#define AUTOTEST   // by commenting this line out, you will have a standard blink program

#ifdef AUTOTEST
#include "AutomaticTesting.h"
/**
 * @name extendSerialOut
 * Extends the Serial out from Auto test with extra data. Do not forget to define the variables as extern.
 * As they might be defined in another source file. In the link it will be resolved.
 * if you are using variables defined lower in the program, you could add the same statement for
 * the variable in this routine like: extern unsigned long ctr
 * Or put this routing below the variables declarations
 */
void extendSerialOut(){

	Serial.print(millis());
	//
	// add more fields by first print a semicolon and next the variable
	//
}
#endif


#define LED 13								// we will use the LED on pin 13

void setup()
{

#ifdef AUTOTEST
	Serial.begin(115200);					// set the baudrate you wish to use. this will work fine on Arduino 1.0.5
	autotest.begin(extendSerialOut); 		// we add user functionality here
#endif

	pinMode(13,OUTPUT);
}
void loop()
{
	digitalWrite(LED, !digitalRead(LED)); 	// toggle the LED pin
	delay(500);								// blink 500mSec every second
}

/**
 * @file AutoTest.cpp
 *
 *  \n Created on	: 27 aug. 2013
 *  \n Author		: Nico Verduin
 *	\n Email		: info@verelec.com
 *  \n Website		: www.verelec.nl
 *
 * @mainpage
 * \n Class methods file for capturing all pinMode(), digitalRead() and digitalWrite() operations from the program where this is included.
 * Testcases can be input to the digitalRead() calls and both read and write changes are sent to Serial in CSV format
 *\n
 * \n The approach taken is to map the actual pin number to an array index. This uses less memory (RAM). Otherwise we would
 * need to allocate memory for each pin (atmega2560 would mean > 50 pins) wich would be quite a lot of scarce RAM.
 * PinMode is used to actually set the mode for the pin.
 * digtalRead()s and digitalWrite()s map the pin to the array index an the values are either set or read
 * from pinVAL which is mapped parallel to pinMap.
 *\n
 * \n This approach also allows inputs and outputs to be mixed in the Excel sheet. So no need to define the inputs
 * first and then the outputs. The testcases are generated in the same order as the pin order.
 *\n
 * \n When a digitalRead Takes place, a check is made if the waiting test case is allowed to be executed. If so
 * the test case data is copied to pinVAL and an actionText field. At the same time a new test case
 * is read into memory. We need to do it this way as the delay time before a test case becomes active is also
 * stored in the test case. The limit of testcases is 10000 although that number will probably never be reached
 * due to Flash Memory size limitations.
 *\n
 * \n As a program will probably do a lot of reading (polling) of pins there is a method called "doNotDisplayReadsIf(uint8_t val)".
 * this method allow to block the printing of data to serial out when the digitalread reads the value val. This will make
 * the output more processable.
 *\n
 * \n To enable users to add more data on the Serial out, the begin method has 2 options:
 * \n a) begin() sets up the class without the ability to extend with user data
 * \n b) begin(user funcion) does the same as above but enables the user to output additional data (like counters) to the serial out
 *\n
 * \n Setting up
 * \n In the folder of the sketch you need to copy the files from the folder "Put these files in your sketch Folder".
 * \n Using Excel open the "generate TestSets.xls" and edit the "test set" sheet. I personally use the next tab "test results"
 * to put my test results in.
 * \n Next click the generate testcases button and you can compile link and upload.
 *\n
 * The generate button generates 3 files:
 * \n FieldLengths.h contains field count and size defines
 * \n PinHeaders.h contains all the pin name definitions
 * \n TestCases.h contains all the testcases.
 *\n
 * The files copied from "Put these files in your sketch Folder" folder are:
 * \n AutomaticTesting.h
 * \n generateTestSets.xls
 *
 * AutomaticTesting.h combines all the generated .h files in the correct format. DO NOT MODIFY THIS FILE
 *
 * In the sketch itself the following code is required:
 *
 * \n #define AUTOTEST
 * \n #ifdef AUTOTEST
 *
 * \n #include "AutomaticTesting.h"
 * \n void extendSerialOut(){
 * \n   //
 * \n   // add micros() output to Serial out
 * \n   // if more fields required separate them with a semicolon
 * \n   //
 * \n   Serial.print(micros);
 * \n 	  Serial.print(";");
 * \n 	  Serial.print(myField);
 * \n  }
 * \n
 * \n #endif
 *
 * You can skip AUTOTEST but that allows me to switch of automatic testing with one statement
 *
 * In the setup() {
 * you need to add the following statements:
 * \n  #ifdef AUTOTEST
 * \n 	  Serial.begin(115200);
 * \n 	  autotest.begin(extendSerialOut);
 * \n  #endif
 *
 * autotest.begin(extendSerialOut) adds the ability to extend output. Serial.begin speaks for itself
 */

 /**
 * Revision Control
 * 
 * Latest Revsion
 * ____________________
 *
 * Revision	: $Revision: 7 $
 * Date		: $Date: 2013-09-11 19:06:55 +0200 (Wed, 11 Sep 2013) $
 * Author	: $Author: Nico $
 *
 */
#include <Arduino.h>
#include "AutoTest.h"


/**
 * @name doNotDisplayReadsIf(uint8_t val)
 * @param val	is either 1 or 0
 * If a read operations gets result val then the displayPins operation is not called avoiding many
 * polling reads
 */
void AutoTest::doNotDisplayReadsIf(uint8_t val) {

	omitDisplayIf = val;
}
/**
 * @name AutoTest
 * @param numberOfPins			Number of pins used in the test cases
 * @param numberOfInputPins 	number of pins used in the test cases
 * @param maxFieldLength 		maximum length of pin names that exist in the test cases
 * @param maxDescriptionLength 	Maximum length of test case descriptions in test cases
 * @param pointerPinHeaders  	address of pinHeaders file in FLASH memory
 * @param pointerTestCases 		address of testCases file in FLASH memory
 * Constructor
 */
AutoTest::AutoTest(	uint8_t numberOfPins		,
					uint8_t numberOfInputPins	,
					uint8_t maxFieldLength		,
					uint8_t maxDescriptionLength,
					PGM_P 	pointerPinHeaders	,
					PGM_P 	pointerTestCases	) {
	//
	// save all the parameters in the class object
	//
	Number_Of_Pins 			= numberOfPins;
	Number_Of_Input_Pins 	= numberOfInputPins;
	Max_Field_Length		= maxFieldLength;
	Max_Description_Length	= maxDescriptionLength;
	pinHeaders 				= pointerPinHeaders;
	testCases				= pointerTestCases;
	//
	// allocate arrays with the correct sizes and quantities
	//
	nextInputValues 		= (uint16_t *) 	malloc(Number_Of_Input_Pins * sizeof(uint16_t));
	pinMap					= (uint8_t *) 	malloc((numberOfPins * sizeof(uint8_t)));
	pinVal					= (uint16_t *) 	malloc(numberOfPins * sizeof(uint16_t));
	pinDescriptions 		= (char *) 	  	malloc((numberOfPins * maxFieldLength));
	//
	// other initializations
	//
	omitDisplayIf = 99;					// display both reads and writes
}
/**
 * @name begin()
 * Normal initialization of the AutoTest library. This function does not call an extended display pins function
 */
void AutoTest::begin() {
	//
	// set the extended displayPins function to NULL
	//
	callExtendDisplayPins = NULL;
	//
	// and do the actual initialization
	//
	_begin();
}

/**
 * @name begin(void (*extendDisplayPins)())
 * @param extendDisplayPins function pointer to user function that will be called when displayPins() is executed
 * Normal initialization of the AutoTest library but now with a function pointer to extend display pins/ using this
 * method allows the user to add additional fields to the Serial Output
 */
void AutoTest::begin(void (*extendDisplayPins)()) {
	//
	// save the function pointer
	//
	callExtendDisplayPins = extendDisplayPins;
	//
	// and do the actual initialization
	//
	_begin();
}

/**
 * @name _begin()
 * This is the initialization method for AutoTest. All the pin descriptions and definitions are put in a
 * couple of arrays and stored for later use. also the first test case is loaded
 */
void AutoTest::_begin(){
	PGM_P 			recordPtr;				// points to a record in Flash memory
	unsigned int 	recordLength;			// length of the current record in Flash memory
	char			pin[3];					// pin id in string form
	char			*field = (char *)malloc(Max_Field_Length); // result field for field pin description when obtained from Flash memory
	uint8_t			iPin;					// pin number in integer form
	uint8_t			numberOfPins;			// keeps track of pins while loading arrays

	//
	// initialize the pinMap & pinVal
	//
	uint8_t j = 0;
	for (uint8_t i = 0; i < Number_Of_Pins; i++) {
		//
		// calculate array offset
		//
		j = i * 2;
		pinMap[j] 	= 255;			// default value for pin not used
		pinMap[j+1]	= INPUT;		// default all pins are set to INPUT
		pinVal[i] 	= 0;			// default value = 0
	}

	//
	// copy the pinHeaders list from flash to RAM
	//
	recordPtr = pinHeaders;				// points to the first character in the Field array
	//
	// get the length of the first record in the array in Flash Memory
	//
	recordLength = getRecordLength(recordPtr);
	//
	// process all defined pins
	//
	numberOfPins 		= 0;			// point to first index in pin array (pinMap)
	while (recordLength != 0) {
		//
		// get the pin, convert it to an int and put it in the array
		//
		recordPtr 				= getToken(recordPtr, pin, ',');// find the pin number and copy it to RAM
		iPin 					= atoi(pin);					// convert to int
		pinMap[numberOfPins * 2]= iPin;							// and save it in the array. This keeps the order of the Excel file
		//
		// getToken returns the first byte address after the token so we can continue to the next fiedl
		//
		recordPtr 				= getToken(recordPtr, field, '\n'); // get pin name
		//
		// and copy it in the pinDescriptions array
		//
		strcpy(&pinDescriptions[(numberOfPins * Max_Field_Length)], field);
		numberOfPins++;								// increment number of pins used as we need this later for the testcases
		//
		// get the new record length
		//
		recordLength 	= getRecordLength(recordPtr);
	}

	//
	// We now have an array with all the input / output pins used in the same order as the Excel sheet
	//
	nextTestCaseNumber 	= -1;			// start with the first testCase(number is incremented first so it becomes 0)
	activateTestCase 	= 0L;			// set wait time to known value
	testCasePtr 		= testCases;	// point to first testcase
	getTestCase();						// get the first testcase
}

/**
 * @name callPinMode(uint8_t pin, uint8_t mode)
 * @param pin 	pin number of Arduino board
 * @param mode	INPUT, INPUT_PULLUP or OUTPUT
 * Rerouting of standard pinMode function. The pin modes are stored into array pinMap(column 1). The first column
 * is the mapping to the real pin. If the pin is an INPUT_PULLUP then the value is set to "1"
 * Doing it this way avoids creating an entry for every pin on the Arduino board
 */
void AutoTest::callPinMode(uint8_t pin, uint8_t mode) {		// replacement function for pinMode()

	unsigned int pinIndex;			// index to pinMap for this pin. It maps the actual pin to the index in pinMap
	//
	// First we have to find the index in the indexed pinMap
	//
	pinIndex = getPinIndex(pin);
	//
	// accept it only if it is described in the excel sheet
	//
	if (pinIndex != Number_Of_Pins) {
		//
		// we found the index in the pinMap array so set mode
		//
		pinMap[((pinIndex * 2)+1)] = mode;
		//
		// set pin value. If mode is PULLUP then the input value is default 1
		//
		if (mode == INPUT || mode == OUTPUT) {
			pinVal[pinIndex] = LOW;					// default value is 0
		} else {
			pinVal[pinIndex] = HIGH;				// with pullup it is 1
		}
	} else {
		//
		// this pin is not defined in the test set so let the user know
		//
		Serial.println("");
		Serial.print("pinMode (");
		Serial.print(pin);
		Serial.print(",");
		Serial.print(mode);
		Serial.println(") invalid pin");
	}
}

/**
 * @name callDigitalRead(int pin)
 * @param pin 	pin number of Arduino board
 * @return int	value read from Pin
 * rerouting of standard digitalRead function. This function reads the pin from the digital Array. However if
 * a testcase becomes active, the value from the test set is written to it before the read takes place
 */
uint8_t AutoTest::callDigitalRead(uint8_t pin) {		// replacement function for digitalRead()
	uint8_t 	pinIndex;							// mapping pin to pinMap
	uint8_t		val;								// value to return

	activateNextTestCase();								// if there is a testcase, it gets Activated
														// if not it is ignored and all values stay the same
	pinIndex = getPinIndex(pin);
	//
	// check if it is a valid pin
	//
	if (pinIndex != Number_Of_Pins) {
		//
		// get value
		//
		val = pinVal[pinIndex];
		//
		// check if we actually have to display this info
		//
		if (val != omitDisplayIf) {
			//
			// now inform the user of this read
			//
			strcpy(actionText, "pin ");
			strcat(actionText, (char *)&pinDescriptions[(pinIndex * Max_Field_Length)]);
			strcat(actionText, " read");
			displayPins();
		}
		//
		// and return the test case value for this pin
		//
		return val;
	} else {
		//
		// this pin is not defined in the test set so let the user know
		//
		Serial.println("");
		Serial.print("digitalRead(");
		Serial.print(pin);
		Serial.println(") invalid pin");
		return 0;
	}
}
/**
 * @name callAnalogRead(int pin)
 * @param pin 	Analog pin number of Arduino board
 * @return int	value read from Pin
 * rerouting of standard digitalRead function. This function reads the pin from the digital Array. However if
 * a testcase becomes active, the value from the test set is written to it before the read takes place
 */
int AutoTest::callAnalogRead(uint8_t pin) {			// replacement function for digitalRead()
	uint8_t 	 pinIndex;							// mapping pin to pinMap
	int  		 val;								// value to return (0 - 1023)

	activateNextTestCase();							// if there is a testcase, it gets Activated
													// if not it is ignored and all values stay the same
	//
	// borrowed this from the original to convert a channel to the correct pin
	//
	#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	if (pin < 54) pin += 54; // allow for channel or pin numbers
#elif defined(__AVR_ATmega32U4__)
	if (pin < 18) pin += 18; // allow for channel or pin numbers
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
	if (pin < 24) pin += 24; // allow for channel or pin numbers
#elif defined(analogPinToChannel) && (defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__))
	pin = analogPinToChannel(pin);
#else
	if (pin < 14) pin += 14; // allow for channel or pin numbers
#endif

	pinIndex = getPinIndex(pin);
	//
	// check if it is a valid pin
	//
	if (pinIndex != Number_Of_Pins) {
		//
		// get value
		//
		val = pinVal[pinIndex];
		//
		// check if we actually have to display this info
		//
		if (val != omitDisplayIf) {
			//
			// now inform the user of this read
			//
			strcpy(actionText, "pin ");
			strcat(actionText, (char *)&pinDescriptions[(pinIndex * Max_Field_Length)]);
			strcat(actionText, " read");
			displayPins();
		}
		//
		// and return the test case value for this pin
		//
		return val;
	} else {
		//
		// this pin is not defined in the test set so let the user know
		//
		Serial.println("");
		Serial.print("analogRead(");
		Serial.print(pin);
		Serial.println(") invalid pin");
		return 0;
	}
}

/**
 * @name callDigitalWrite(uint8_t pin, uint8_t val)
 * @param pin 	pin number of Arduino board
 * @param val	value to write to Pin
 * rerouting of standard digitalWrite function. this function sets the pin value in the pinArray (containing all digital pins)
 */
void AutoTest::callDigitalWrite(uint8_t pin, uint8_t val) {		// replacement function for digitalWrite()

	uint8_t pinIndex;					// maps the pin to the pinMap array index
	char 	level[5];					// string for HIGH and LOW text
	//
	// set the correct value
	//
	pinIndex		= getPinIndex(pin);	// get pinMap index
	//
	// check if it is a valid pin
	//
	if (pinIndex != Number_Of_Pins) {
		//
		// check if this is a valid value
		//
		if (val != LOW && val != HIGH) {
			//
			// The user is trying to send junk to pin
			//
			Serial.println("");
			Serial.print("digitalWrite(");
			Serial.print(pin);
			Serial.print(",");
			Serial.print(val);
			Serial.println(") error. Value sent is not HIGH or LOW");
		} else {
			//
			// evrything is valid so perform write
			//
			pinVal[pinIndex] = val;
			//
			// now inform the user of this write
			//
			if (val == 0) {
				strcpy(level, "LOW");
			} else {
				strcpy(level, "HIGH");
			}
			strcpy(actionText, "pin ");
			strcat(actionText, (char *)&pinDescriptions[(pinIndex * Max_Field_Length)]);
			strcat(actionText, " set to ");
			strcat(actionText, level);
			displayPins();
		}
	} else {
		//
		// this pin is not defined in the test set so let the user know
		//
		Serial.println("");
		Serial.print("digitalWrite(");
		Serial.print(pin);
		Serial.print(",");
		Serial.print(val);
		Serial.println(") invalid pin");
	}
}
/**
 * @name callAnalogWrite(uint8_t pin, uint8_t val)
 * @param pin 	pin number of Arduino board
 * @param val	value to write to PWM pin
 * rerouting of standard digitalWrite function. this function sets the pin value in the pinArray (containing all digital pins)
 * Keep in mind we do not check if this is a valid PWM pin
 */
void AutoTest::callAnalogWrite(uint8_t pin, uint8_t val) {		// replacement function for digitalWrite()

	uint8_t pinIndex;					// maps the pin to the pinMap array index
	char valString[4];					// string value of pwm value
	//
	// set the correct value
	//
	pinIndex		= getPinIndex(pin);	// get pinMap index
	//
	// check if it is a valid pin
	//
	if (pinIndex < Number_Of_Pins) {
		//
		// No need to check the value as it can be any value from 0-255
		//
		pinVal[pinIndex] = val;
		//
		// now inform the user of this write
		//
		strcpy(actionText, "pin ");
		strcat(actionText, (char *)&pinDescriptions[(pinIndex * Max_Field_Length)]);
		strcat(actionText, " set to ");
		itoa(val, valString, 10);
		strcat(actionText, valString);
		displayPins();
	} else {
		//
		// this pin is not defined in the test set so let the user know
		//
		Serial.println("");
		Serial.print("digitalWrite(");
		Serial.print(pin);
		Serial.print(",");
		Serial.print(val);
		Serial.println(") invalid pin");
	}
}

/**
 * @name displayPins()
 * @param pin 	pin number of Arduino board
 * @param val	value to write to Pin
 * rerouting of standard digitalWrite function. this function sets the pin value in the pinArray (containing all digital pins)
 *
 */
void AutoTest::displayPins() {
	//
	// print the action text
	//
	Serial.println("");
	Serial.print(actionText);
	Serial.print(CSV_SEPARATOR);
	//
	// check each pin if it is defined in the program. Defined means it was programmed through pinMode()
	//
	for (uint8_t i = 0; i < Number_Of_Pins; i++) {
		Serial.print(pinVal[i]);		// print Value
		Serial.print(CSV_SEPARATOR);				// print a separator
	}
	//
	// check if the user wants more output
	//
	if (callExtendDisplayPins != NULL) {
		callExtendDisplayPins();					// allow the user to extend the info to serial
	}
}
/**
 * @name getTestCase()
 * @returns testCaseAvailable 0 = no more test cases 1 = still testcases
 * Reads the testcase if there is one and returns 1. Otherwise returns 0
 *
 */
uint8_t AutoTest::getTestCase(){

	uint8_t  returnCode;				// result of this operation
	char	 pinValue[5];				// value of pin as a string (could be 0-1023
	char	 delayTime[10];				// delay time as string
	uint16_t recordLength;				// returns the length of a record in testcases

	//
	// check if there is any testcase left
	//
	if (nextTestCaseNumber >= 10000) {
		//
		// limit reached
		//
		nextTestCaseNumber = 10000;
		returnCode = 1;
	} else {
		nextTestCaseNumber++;
	}
	//
	// anything to process
	//
	if (nextTestCaseNumber < 10000) {
		//
		// check if we have a test case
		//
		recordLength = getRecordLength(testCasePtr);
		if (recordLength != 0) {
			//
			// we have a testcase So let's get it
			//
			// Get description
			//
			testCasePtr = getToken(testCasePtr, nextTestCaseDescription, ',');
			//
			// get the test case values
			//
			for(uint8_t i = 0; i < Number_Of_Input_Pins; i++) {
				//
				// copy the values from the FLASH memory
				//
				testCasePtr = getToken(testCasePtr, pinValue, ',');
				//
				// convert the sting to an int and put in the nextInputValues array
				//
				nextInputValues[i] = atoi(pinValue);
			}
			//
			// get the delay time of this test case
			//
			testCasePtr 		= getToken(testCasePtr, delayTime, '\n');
			activationDelay 	= atoi(delayTime);
			activateTestCase 	= millis() + activationDelay;
			//
			// set the return code
			//
			returnCode = 1;

		} else {
			//
			// no more testcases
			//
			nextTestCaseNumber = 10000;
			returnCode = 0;
		}
	} else {
		//
		// no more test cases
		//
		returnCode = 0;
	}
	return returnCode;
}

/**
 * @name getRecordlength ()
 * @param ptr points to a memory locationin flash Memeory
 * @returns int length of the record
 * This function returns the number of bytes to the next newline character from ptr location
 */
int AutoTest::getRecordLength(PGM_P ptr) {
	//
	// find the newline character in Flash
	//
	PGM_P temp = strchr_P (ptr, '\n');
	//
	// return the difference
	//
	return temp - ptr;
}

/**
 * @name getToken(PGM_P sourcePtr, char * destPtr, uint8_t token)
 * @param sourcePtr	Source in Flash memory
 * @param destPtr	Destination in RAM
 * @param token		token to be scanned for
 * @returns PGM_P	pointer to Flash address 1 after token address
 * Checks the string in Flahs Memory if it finds the requested token.
 * Once found, it will copy all data up to the token and terminate it with a '\0'
 */
PGM_P AutoTest::getToken(PGM_P sourcePtr, char * destPtr, uint8_t token) {
	PGM_P tempPtr;			// temporary pointer
	unsigned int length;	// length of the field
	//
	// locate token
	//
	tempPtr = strchr_P(sourcePtr, token);
	//
	// determine length field
	//
	length 	= tempPtr - sourcePtr;
	//
	// copy text from Flash to RAM
	//
	strncpy_P(destPtr, sourcePtr, length);
	//
	// terminate string with \0

	destPtr[length] = '\0';
	//
	// done
	//
	tempPtr++;
	return tempPtr; 		// return the next byte after the token
}
/**
 * @name getPinIndex(uint8_t pin)
 * @param pin pin number to search for
 * @returns uint8_t with the index in pinMap.
 * Searches pinMap for the pin and returns the index of it in pinMap. If not found, the first index out of range is returned
 */
uint8_t AutoTest::getPinIndex(uint8_t pin) {

	uint8_t returnPin = Number_Of_Pins;				// first pin out of range
	//
	// search the array
	//
	for (uint8_t i = 0; i < Number_Of_Pins; i++) {
		if (pinMap[(i * 2)] == pin) {
			//
			// found it and save i for the exit
			//
			returnPin = i;
			break;
		}
	}
	//
	// return the found value or the out of bounds value
	//
	return returnPin;
}
/**
 * @name activateTestcase()
 * Activates the current load testcase if there is one
 */
void AutoTest::activateNextTestCase(){
	//
	// check if there are anymore testcases
	//
	if (nextTestCaseNumber != 10000) {
		//
		// now check if we can activate this Testcase
		//
		if (millis() > activateTestCase) {
			//
			// time to activate the testcase
			// copy the test case description
			//
			strcpy(actionText, nextTestCaseDescription);
			//
			// copy the only the input pins
			//
			uint8_t j = 0;
			for (uint8_t i = 0; i < Number_Of_Pins; i++) {
				if(pinMap[((i * 2) + 1)] == INPUT || pinMap[((i * 2) + 1)] == INPUT_PULLUP){
					//
					// this is an input pin. If an analog read takes place this should still work fine
					// als all pins are defined as INPUT
					//
					pinVal[i] = nextInputValues[j];
					j++;
				}
			}
			//
			// and let the user know this test cases is activated
			//
			displayPins();
			//
			// set the next testcase ready
			//
			getTestCase();
		}
	}

}

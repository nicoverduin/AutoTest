/**
 * AutoTest.h
 *
 *  Created on	: 24 aug. 2013
 *  Author		: Nico Verduin
 *	Email		: info@verelec.com
 *  Website		: www.verelec.nl
 *
 * Include file for capturing all pinMode(), digitalRead() and digitalWrite() operations from the program where this is included.
 * Testcases can be input to the digitalRead() calls and both read and write changes are sent to Serial in CSV format
 *
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


#ifndef AUTOTEST_H_
#define AUTOTEST_H_

#include "Arduino.h"
//#include "FieldLengths.h"

//
// needs better process here
//
#define CSV_SEPARATOR ";"
//
/**
 * @class AutoTest
 * Class for handling autotest facility into Arduino programs. It captures all the digitalRead and digitalWrite functions
 * insode the program where it is included and repleces these functions by allowing test input to be put into read operations
 * and outputs all Write + read pins to the Serial
 */
class AutoTest {
public:
	AutoTest(uint8_t, uint8_t, uint8_t, uint8_t, PGM_P, PGM_P);

	void begin(void (*)());								// initialize the auto test with an extend display pins function
	void begin();										// initializes the AutoTest with no extend display function
	uint8_t  callDigitalRead(uint8_t pin);					// replacement function for digitalRead()
	int  callAnalogRead(uint8_t pin);					// replacement function for analogRead()
	void callDigitalWrite(uint8_t pin, uint8_t val);	// replacement function for digitalWrite()
	void callAnalogWrite(uint8_t pin, uint8_t val);		// replacement function for analogWrite()
	void callPinMode(uint8_t pin, uint8_t mode);		// replacement function for pinMode()
	void doNotDisplayReadsIf(uint8_t val);				// omits displaying pin info if pin in Read has value x

private:
	//
	// variables used for activating a new test case
	//
	int				nextTestCaseNumber;					// number of next test case
	uint16_t		* nextInputValues;					// array containing the input values for the next test case.
	unsigned long 	activationDelay;					// delay before the next test case is activated
	unsigned long 	activateTestCase;					// moment when to activate the test case
	PGM_P			testCasePtr;						// points to current Test case in FLASH
	PGM_P			pinHeaders;							// pointer ot PinHeaders in Flash
	PGM_P			testCases;							// pointer to testCases in Flash
	char			nextTestCaseDescription[26];		// description of next test case
	uint8_t			Number_Of_Pins;						// number of pins filled in constructor
	uint8_t			Max_Field_Length;					// field length filled in constructor
	uint8_t			Number_Of_Input_Pins;				// number of input pins filled in constructor
	uint8_t			Max_Description_Length;				// max length description filled in constructor
	uint8_t			omitDisplayIf;						// contains value when to omit displaypins with read operations. if 99 all values are displayed
	//
	// other variables used
	//
	void 			(*callExtendDisplayPins)();			// function pointer to extend display pins
	char			actionText[26];						// Action test to display with output (max 25 positions)
	//
	// Array created to the number of pins defined in the excel sheet. Memory is allocated during the construction
	// pinMap has 2 columns:
	// column 0 reference to real pin number
	// column 1 mode (INPUT, OUTPUT, INPUT_PULLUP)
	//
	uint8_t 		*pinMap;			// maps the pins
	//
	// pinVal corresponds to pinMap and contains the values. The reason to do it this way
	// is to allow analog values for the pin which can have a value from 0-1023
	//
	uint16_t		*pinVal;			// maps to digital or analog values

	//
	// pinDescriptions contains an array with the names of each pin. Memory is allocated during the construction
	//
	char 			*pinDescriptions;
	//
	// methods
	//
	void 	_begin();									// does the actual initialization
	void 	displayPins();								// outputs the pin values etc to Serial
	uint8_t getTestCase();								// points to the next testcase and checks if we are through
	int 	getRecordLength(PGM_P);						// gets the length of a record from Flash
	uint8_t getPinIndex(uint8_t);						// searches pin Array and returns index for pin
	PGM_P 	getToken(PGM_P sourcePtr, char * destPtr, uint8_t token); // copies a string up to a token
	void	activateNextTestCase();						// activates the loaded testcase

};

#endif /* AUTOTEST_H_ */

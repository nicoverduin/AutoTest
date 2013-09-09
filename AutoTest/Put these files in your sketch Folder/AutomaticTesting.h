/**
 * ReRoute.h
 *
 *  Created on	: 8 sep. 2013
 *  Author		: Nico Verduin
 *	Email		: info@verelec.com
 *  Website		: www.verelec.nl
 *
 * Revision Control
 * 
 * Latest Revsion
 * ____________________
 *
 * Revision	: $Revision$
 * Date		: $Date$
 * Author	: $Author$
 *
 */

#ifndef AUTOMATIC_TESTING_H_
#define AUTOMATIC_TESTING_H
#include <AutoTest.h>
//
// get the generated headers. The trick used here is that there are files generated from the Excel sheet. These files
// are put in between the const and ";"
//
const PROGMEM char pinHeaders[] =
#include "pinHeaders.h"
;
const PROGMEM char testCases[] =
#include "TestCases.h"
;
#include "FieldLengths.h"
//
// end getting generated headers
//

//
// Create the AutTest object
//
AutoTest autotest(NUMBER_OF_PINS,NUMBER_OF_INPUT_PINS,MAX_FIELD_LENGTH,MAX_DESCRIPTION_LENGTH, pinHeaders, testCases);
//
// redefine functions to replace the original pinMode(), digitalRead() and digitalWrite() functions
//
#define pinMode(a,b)		autotest.callPinMode(a,b)
#define digitalRead(a)		autotest.callDigitalRead(a)
#define digitalWrite(a,b)	autotest.callDigitalWrite(a, b)

#endif /* AUTOMATIC_TESTING_H */

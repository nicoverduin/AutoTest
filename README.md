### AutoTest

# Arduino Class for automatic testing on Arduino

Class methods file for capturing all pinMode(), digitalRead() and digitalWrite() operations from the program where this is included.
Testcases can be input to the digitalRead() calls and both read and write changes are sent to Serial in CSV format

The approach taken is to map the actual pin number to an array index. This uses less memory (RAM). Otherwise we would need to allocate memory for each pin (atmega2560 would mean > 50 pins) which would be quite a lot of scarce RAM.
PinMode() is used to actually set the mode for the pin. Default is INPUT
digtalRead()s and digitalWrite()s map the pin to the array index an the values are either set or read
from pinMap.

This approach also allows inputs and outputs to be mixed in the Excel sheet. So no need to define the inputs first and then the outputs. The testcases are generated in the same order as the pin order.

When a digitalRead Takes place, a check is made if the waiting test case is allowed to be executed. If so the test case data is copied to pinMap and an actionText field. At the same time a new test case
is read into memory. We need to do it this way as the delay time before a test case becomes active is also stored in the test case. The limit of testcases is 10000 although that number will probably never be reached due to Flash Memory size limitations.

As a program will probably do a lot of reading (polling) of pins there is a method called
`doNotDisplayReadsIf(uint8_t val);`

this method allows to block the printing of data to serial out when the digitalread reads the value val. This will make the output more processable.

To enable users to add more data on the Serial out, the begin() method has 2 options:
* begin() sets up the class without the ability to extend with user data
* begin(user function) does the same as above but enables the user to output additional data (like counters) to the serial out

# Setting up
In the folder of the sketch you need to copy the files from the folder .**Put these files in your sketch Folder**
Using Excel open the **generate TestSets.xls** and edit the **test set** sheet. I personally use the next tab **test results** to put my test results in.

Next click the generate testcases button and you can compile link and upload.

The generate button generates 3 files:
* **FieldLengths.h** contains field count and size defines
* **PinHeaders.h** contains all the pin name definitions
* **TestCases.h** contains all the testcases.

The files copied from **Put these files in your sketch Folder** folder are:
* **AutomaticTesting.h**      DO NOT MODIFY THIS FILE
* **generateTestSets.xls**


In the sketch itself the following code is required:

```
#define AUTOTEST
#ifdef AUTOTEST
#include "AutomaticTesting.h"
void extendSerialOut(){
   //
   // add micros() output to Serial out
   // if more fields required separate them with a semicolon
   //
   Serial.print(micros());
   Serial.print(";");   
   Serial.print(myField);  
}
#endif
```
You can skip AUTOTEST but that allows me to switch of automatic testing with one statement

In the setup() you need to add the following statements:
```
#ifdef AUTOTEST
Serial.begin(115200);  
autotest.begin(extendSerialOut);
#endif`
```

**autotest.begin(extendSerialOut)** adds the ability to extend output. Serial.begin speaks for itself



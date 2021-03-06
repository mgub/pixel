// Notes for Teensy 3.1 (i.e., heuristics to get the CC3000 module working, based on experience):
// - Set to 48 MHz
// - Set clock to SPI_CLOCK_DIV8 (SPI_CLOCK_DIV2 also worked, but not too reliably)

// Adafruit Breakout Board:
// - Firmware V. : 1.24

#include <Wire.h>

// TODO: Master board I/O state for (1) requested state and (2) reported state (by master).
struct PinModel {
  int pin; // The pin number
  int type; // i.e., digital, analog, pwm, touch
  int mode; // i.e., input or output
  int value; // i.e., high or low
};
PinModel deviceReportedModel[24];
//for (int i = 0; i < 24; i++) {
//  deviceReportedModel[i].pin = i;
//  deviceReportedModel[i].type = 0; // i.e., Digital
//  deviceReportedModel[i].mode = 1; // i.e., Output
//  deviceReportedModel[i].value = 0;
//}

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "WebServer.h"

#define I2C_DEVICE_ADDRESS 2

// TODO: Implement list of changes to make to send to the Master (which executes gestural and the behavior code for the module)
// - TODO: Include status: "new", "sending", "sent", "confirmed" (after which, they're deleted)

// TODO: Implement web server and request handlers that, in response, update the queue of
//       changes to (1) make to Looper, and (2) to queue for sending to the other device 
//       over I2C upon request.

void setup () {
  Serial.begin(115200); // Start serial for output
  Serial.println("Slave device initializing...");
  
  // Setup Wi-Fi and web server
  setupWebServer();
  
  // Setup I2C communication for device-device communication
  setupDeviceCommunication();
}

void setupDeviceCommunication() {
  Wire.begin(I2C_DEVICE_ADDRESS); // Join I2C bus with the device's address
  Wire.onReceive(receiveEvent);   // Register event handler to receive data from the master I2C device
  Wire.onRequest(requestEvent);   // Event handler to respond to a request for data from the I2C master device
}

/**
 * The event loop (i.e., this function is called repeatedly when the board is on)
 */
void loop () {
    
  // Try to get a client which is connected.
  Adafruit_CC3000_ClientRef client = httpServer.available();
  
  if (client) {
    handleClientConnection (client);
  }
}

char i2cBuffer[32];
int i2cBufferSize = 0;
boolean hasMessage = false;

/**
 * function that executes whenever data is received from master
 * this function is registered as an event, see setup()
 */
void receiveEvent (int howMany) {
  
  while (Wire.available () > 0) { // loop through all but the last
    char c = Wire.read (); // receive byte as a character
    Serial.print (c); // print the character
    
    i2cBuffer[i2cBufferSize] = c; // Buffer the character
    i2cBufferSize++; // Increment the buffer size
  }
  i2cBuffer[i2cBufferSize] = NULL; // Terminate the string
  Serial.println();
  
  String split = String(i2cBuffer); // "hi this is a split test";
  String operation = getValue(split, ' ', 0);
  int pin = getValue(split, ' ', 1).toInt();
  int value = getValue(split, ' ', 2).toInt();
  
  Serial.print("PIN ");
  Serial.print(pin);
  Serial.print(" = ");
  Serial.print(value);
  deviceReportedModel[pin].value = value;
  // TODO: Update other state info for pin (or other state)
//  Serial.print(pin);
  Serial.println();
  
  i2cBufferSize = 0;
  
  
  // TODO: Parse the message
  // TODO: Handle initial processing for message
  // TODO: Add to incoming I2C message queue (to process)
  // TODO: (elsewhere) Process messages one by one, in order
}

/**
 * function that executes whenever data is requested by master
 * this function is registered as an event, see setup()
 */
void requestEvent () {
//  Wire.write ("13 w d o h "); // respond with message of 6 bytes as expected by master

//  Serial.print("Count: ");
//  Serial.println(behaviorNodeCount);
  
  char buf[4]; // "-2147483648\0"
//  Wire.write (itoa(behaviorNodeCount, buf, 10));
  
  if (behaviorNodeCount > 0) {

    // Send serialized behavior
    Wire.write (itoa(behaviorNodes[0].operation, buf, 10)); Wire.write (" ");
    Wire.write (itoa(behaviorNodes[0].pin, buf, 10));       Wire.write (" ");
    Wire.write (itoa(behaviorNodes[0].type, buf, 10));      Wire.write (" ");
    Wire.write (itoa(behaviorNodes[0].mode, buf, 10));      Wire.write (" ");
    Wire.write (itoa(behaviorNodes[0].value, buf, 10));     Wire.write (" ");
    
    // Remove the behavior from the processing queue once it's been sent over I2C
    removeBehaviorNode (0);
  }
  
  // Wire.write("3 14 digital high"); // Step 3: Ensures that stat of pin 14 is digital and that it is set to high, doing so as needed
  // Wire.write("4 delay 5"); // Step 4: Adds a delay of 5 seconds in the program
  // Wire.write("6 14 digital low"); // Step 6: Same as above, but sets low rather than high.
  
  // pin, operation, type, mode, value
  // "13 write digital output high"
  // "13 w d o h"
}

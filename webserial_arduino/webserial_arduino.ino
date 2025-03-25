#include <ArduinoJson.h>
#include <Servo.h>
Servo myservo;  // create servo object to control a servo

const int messageLength = 13; // this is not used at the moment - would be the message length for parsing JSON Data, used in recvJSON()
const int inputPin = A0;

//char json[] = "{\"angle\":255}";
char jsonMessage[messageLength];
boolean newData = false;
unsigned char receivedChar;

void setup() {
  pinMode(inputPin, INPUT); // set pin to input
  //pinMode(inputPin, INPUT_PULLUP); // set pin to input and turn on pullup resistors
  myservo.attach(6);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600); // initialize serial communications
}
 
void loop() {

  // read the input pin:
  int potentiometer = analogRead(inputPin);                  
  // remap the pot value to fit in 1 byte:
  int mappedPot = map(potentiometer, 0, 1023, 0, 180);   // Print values.

  // send values out serial port as a JSON string,
  // the resulting JSON lookins like this: {"angle":mappedPot}
  Serial.print("{\"angle\":");
  Serial.print(mappedPot);
  Serial.println("}");

  recvOneChar();
  updateServo();
                           
  // slight delay to stabilize the ADC:
  delay(1);                                            
}

void updateServo() {
    if (newData == true) {
        int receivedAngle = receivedChar - 0;
        myservo.write(receivedAngle);
        newData = false;
    }
}

void recvOneChar() {
    if (Serial.available() > 0) {
        receivedChar = Serial.read();
        newData = true;
    }
}



// Parsing JSON - still have to debug this one
void recvJSON() {

  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0) {
        rc = Serial.read();

        if (rc != endMarker) {
            jsonMessage[ndx] = rc;
            ndx++;
            if (ndx >= messageLength) {
                ndx = messageLength - 1;
            }
        }
        else {
            jsonMessage[ndx] = '\n'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }

    // Inside the brackets, 18 is the capacity of the memory pool in bytes.
    // Don't forget to change this value to match your JSON document.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<messageLength> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, jsonMessage);

    // Test if parsing succeeds.
    if (error) {
      //Serial.print(F("deserializeJson() failed: "));
      //Serial.println(error.f_str());
      return;
    }
    // Fetch values.
    //
    // Most of the time, you can rely on the implicit casts.
    // In other case, you can do doc["time"].as<long>();
    int receivedAngle = doc["angle"];
    myservo.write(receivedAngle);                  // sets the servo position according to the scaled value
    newData = false;
}
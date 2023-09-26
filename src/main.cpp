#include <Arduino.h>

int allSerialBraudrate = 115200;
HardwareSerial &eg25 = Serial1;          // Define Serial1 as EG25-G

String sendATCommand3(const String &command, const String &expectedResponse, unsigned long timeout)
{
  eg25.println(command);
  // Serial.println("Command sent: " + command); // Debug print

  unsigned long startTime = millis();

  while (millis() - startTime < timeout)
  {
    if (eg25.available())
    {
      String response = eg25.readStringUntil('\n');
      response.trim(); // Remove whitespace characters
      // Serial.println(response);
      if (response.length() > 0)
      { // Ignore empty lines
        // Serial.println("Response received: " + response); // Debug print

        if (response.indexOf(expectedResponse) != -1)
        {
          return response;
        }
        else if (response.indexOf("ERROR") != -1)
        {
          return "ERROR";
        }
      }
    }
  }

  return "TIMEOUT";
}
void setup() {
  // put your setup code here, to run once:
   Serial.begin(allSerialBraudrate);
   eg25.begin(allSerialBraudrate);
}

void loop() {
  if (eg25.available()) {
    char receivedChar = eg25.read();
    Serial.print(receivedChar);
  }
  if (Serial.available()) {
    char commandChar = Serial.read();
    eg25.print(commandChar);
  }
}


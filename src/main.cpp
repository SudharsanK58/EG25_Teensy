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
void checkSimCardStatus() {
  String command = "AT+QSIMSTAT?";
  String expectedResponsePrefix = "+QSIMSTAT:";
  unsigned long timeout = 5000; // 5 seconds timeout

  // Send the AT command to check SIM card status
  String response = sendATCommand3(command, expectedResponsePrefix, timeout);

  // Check if the response starts with the expected prefix
  if (response.startsWith(expectedResponsePrefix)) {
    // Extract the enable and inserted status values from the response
    response.remove(0, expectedResponsePrefix.length()); // Remove the prefix
    response.trim(); // Remove leading/trailing spaces

    // Split the response into two parts (enable and inserted status)
    int commaIndex = response.indexOf(',');
    if (commaIndex != -1) {
      String enableStatus = response.substring(0, commaIndex);
      String insertedStatus = response.substring(commaIndex + 1);

      // Check if the SIM card is present (insertedStatus == "1")
      if (insertedStatus.toInt() == 1) {
        // Print a message indicating the SIM card is present
        Serial.println("SIM card is present");
      } else {
        // Halt the program and print a message if the SIM card is not present
        Serial.println("Please insert the SIM card");
        while (true) {
          // Halt the program here
        }
      }
    }
  } else {
    // Handle unexpected response
    Serial.println("Unexpected response: " + response);
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.println("Sample EG25 program is started");
  Serial.begin(allSerialBraudrate);
  eg25.begin(allSerialBraudrate);
  delay(2000);
  checkSimCardStatus();
  delay(1000);
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


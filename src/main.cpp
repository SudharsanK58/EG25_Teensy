#include <Arduino.h>

int allSerialBraudrate = 115200;
String operatorName; // Global variable to store the operator name
String networkTypeString; // Global variable to store the network type as a string
HardwareSerial &eg25 = Serial1;          // Define Serial1 as EG25-G

String generateRandomString(size_t length)
{
  String randomString = "";
  char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  size_t charsetSize = sizeof(charset) - 1;

  for (size_t i = 0; i < length; i++)
  {
    uint8_t randomIndex = random(charsetSize);
    randomString += charset[randomIndex];
  }

  return randomString;
}
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
// Function to set Internet registration status to "1" with retries
void setInternetRegistration() {
  String command = "AT+CREG=1";
  String expectedResponse = "OK";
  unsigned long timeout = 5000; // 5 seconds timeout
  int maxRetries = 3; // Maximum number of retries

  for (int retryCount = 0; retryCount < maxRetries; retryCount++) {
    String response = sendATCommand3(command, expectedResponse, timeout);

    if (response == expectedResponse) {
      // Setting Internet registration successful
      Serial.println("Internet registration set to '1' successfully");
      return; // Exit the function on success
    } else {
      // Handle command failure or timeout
      Serial.println("Failed to set Internet registration to '1'");
    }
  }

  // If setting Internet registration fails after maxRetries attempts, you can handle it here.
  Serial.println("Could not set Internet registration to '1'");
  // Optionally add a delay before retrying
  // delay(1000); // 1-second delay before retry
}

String getDeviceModel() {
  String command = "AT+GMR";
  String expectedResponsePrefix = "EG";
  unsigned long timeout = 5000; // 5 seconds timeout

  // Send the AT command to get device model
  String response = sendATCommand3(command, expectedResponsePrefix, timeout);

  // Check if the response starts with the expected prefix
  if (response.startsWith(expectedResponsePrefix)) {
    // Extract the device model from the response
    response.trim(); // Remove leading/trailing spaces
    return response;
  } else {
    // Handle unexpected response
    return "Unknown"; // Return an "Unknown" model in case of unexpected response
  }
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

void activateAPN() {
  String apnCommand = "AT+CGDCONT=1,\"IP\",\"iot.com\"";
  String pdpCommand = "AT+CGACT=1,1";
  String expectedAPNResponse = "OK";
  String expectedPDPResponse = "OK";
  unsigned long timeout = 5000; // 5 seconds timeout
  int maxRetries = 10; // Maximum number of retries

  // Activate APN
  for (int retryCount = 0; retryCount < maxRetries; retryCount++) {
    String apnResponse = sendATCommand3(apnCommand, expectedAPNResponse, timeout);

    if (apnResponse == expectedAPNResponse) {
      // APN activation successful
      Serial.println("APN activated successfully");

      // Attempt to activate PDP context
      String pdpResponse = sendATCommand3(pdpCommand, expectedPDPResponse, timeout);

      if (pdpResponse == expectedPDPResponse) {
        // PDP context activation successful
        Serial.println("PDP context activated successfully");
        return;
      } else {
        // Handle PDP activation failure
        Serial.println("Can't activate PDP context");
        // Optionally add a delay before retrying PDP activation
        // delay(1000); // 1-second delay before retry
      }
    } else if (apnResponse == "TIMEOUT") {
      // Handle APN activation timeout, optionally add a delay between retries
      // delay(1000); // 1-second delay between retries
    } else if (apnResponse == "ERROR") {
      // Handle APN activation error, you can log an error message or take appropriate action
    }

    // If it's not successful, retry APN activation
  }

  // Halt the program if APN and PDP context activation failed after maxRetries attempts
  Serial.println("Can't activate the APN and PDP context");
  while (true) {
    // Halt the program here
  }
}

// Function to get the network the SIM card is connected to
void getConnectedNetwork() {
  String command = "AT+COPS?";
  String expectedResponsePrefix = "+COPS:";
  unsigned long timeout = 5000; // 5 seconds timeout

  // Send the AT command to get connected network information
  String response = sendATCommand3(command, expectedResponsePrefix, timeout);

  // Check if the response starts with the expected prefix
  if (response.startsWith(expectedResponsePrefix)) {
    // Extract the operator name and network type from the response
    response.remove(0, expectedResponsePrefix.length()); // Remove the prefix
    response.trim(); // Remove leading/trailing spaces

    // Find the first quote character
    int firstQuoteIndex = response.indexOf('\"');
    if (firstQuoteIndex != -1) {
      // Find the second quote character
      int secondQuoteIndex = response.indexOf('\"', firstQuoteIndex + 1);
      if (secondQuoteIndex != -1) {
        // Extract the operator name between the quotes
        operatorName = response.substring(firstQuoteIndex + 1, secondQuoteIndex);
        
        // Find the last number in the response
        int lastIndex = response.lastIndexOf(',');
        if (lastIndex != -1) {
          // Extract the network type as the last number
          String networkTypeStr = response.substring(lastIndex + 1);
          int networkType = networkTypeStr.toInt(); // Get the network type as an integer
          
          // Interpret the network type based on your requirements
          switch (networkType) {
            case 0:
              networkTypeString = "GSM";
              break;
            case 2:
              networkTypeString = "UTRAN";
              break;
            case 3:
              networkTypeString = "GSM W/EGPRS";
              break;
            case 4:
              networkTypeString = "UTRAN W/HSDPA";
              break;
            case 5:
              networkTypeString = "UTRAN W/HSUPA";
              break;
            case 6:
              networkTypeString = "UTRAN W/HSDPA and HSUPA";
              break;
            case 7:
              networkTypeString = "E-UTRAN";
              break;
            case 100:
              networkTypeString = "CDMA";
              break;
            default:
              networkTypeString = "Unknown";
              break;
          }
          return;
        }
      }
    }
  }

  // Handle unexpected response
  operatorName = "Unknown";
  networkTypeString = "Unknown";
  Serial.println("Could not retrieve connected network information");
}

// Function to activate the internet with retry and timeout
void activateInternet() {
  String internetCommand = "AT+CREG?";
  String expectedResponse1 = "+CREG: 1,1"; // Registered, home network
  String expectedResponse2 = "+CREG: 1,5"; // Registered, roaming
  unsigned long timeout = 5000; // 5 seconds timeout
  int maxRetries = 10; // Maximum number of retries

  // Check network registration status
  for (int retryCount = 0; retryCount < maxRetries; retryCount++) {
    String response = sendATCommand3(internetCommand, "+CREG:", timeout);

    if (response.startsWith(expectedResponse1)) {
      // Registered, home network
      Serial.println("Registered, home network");
      return;
    } else if (response.startsWith(expectedResponse2)) {
      // Registered, roaming
      Serial.println("Registered, roaming");
      return;
    } else if (response == "TIMEOUT") {
      // Handle timeout, optionally add a delay between retries if needed
      // delay(1000); // 1-second delay between retries
    } else {
      // Handle unexpected response or error
      Serial.println("Could not activate the internet");
    }
  }

  // Halt the program if internet activation failed after maxRetries attempts
  Serial.println("Could not activate the internet");
  while (true) {
    // Halt the program here
  }
}

void resetMQTT()
{
  String command = "AT+QMTDISC=0"; // AT command to reset MQTT
  String expectedResponse = "OK";  // Expected response
  unsigned long timeout = 6000;    // 6 seconds timeout

  String response = sendATCommand3(command, command, timeout); // Expect echo of the command

  if (response == command)
  { // Command echo received, now expect OK
    response = sendATCommand3("", expectedResponse, timeout);
  }

  if (response == expectedResponse)
  {
    Serial.println("MQTT reset successfully");
  }
  else
  {
    Serial.println("Failed to reset MQTT");
  }
}
void setMQTTMode()
{
  String command = "AT+QMTCFG=\"recv/mode\",0,0,1"; // AT command to set MQTT mode
  String expectedResponse = "OK";                   // Expected response
  unsigned long timeout = 6000;                     // 6 seconds timeout

  String response = sendATCommand3(command, command, timeout); // Expect echo of the command

  if (response == command)
  { // Command echo received, now expect OK
    response = sendATCommand3("", expectedResponse, timeout);
  }

  if (response == expectedResponse)
  {
    Serial.println("MQTT mode set successfully");
  }
  else
  {
    Serial.println("Failed to set MQTT mode");
  }
}
void openMQTT()
{
  String command = "AT+QMTOPEN=3,\"mqtt.zig-web.com\",1883"; // AT command to open MQTT connection
  String expectedResponse = "+QMTOPEN: 3,";                  // Expected response prefix
  unsigned long timeout = 6000;                              // 6 seconds timeout

  String response = sendATCommand3(command, command, timeout); // Expect echo of the command

  if (response == command)
  { // Command echo received, now expect +QMTOPEN: 3,
    response = sendATCommand3("", expectedResponse, timeout);
  }

  if (response.startsWith(expectedResponse))
  {
    // If the command is successfully acknowledged, read the MQTT connection status
    char mqttStatus = response.charAt(response.length() - 1); // Last character is MQTT status

    if (mqttStatus == '0')
    {
      Serial.println("MQTT connected successfully");
    }
    else
    {
      Serial.println("MQTT connection failed or no network");
    }
  }
  else
  {
    Serial.println("Failed to send AT command");
  }
}

void connectMQTT()
{
  String clientId = generateRandomString(7); // generate a 7-character random string
  Serial.println("GSM Mqtt client id : " + clientId);

  String command = "AT+QMTCONN=3,\"" + clientId + "\",\"\",\"\""; // AT command to connect to MQTT
  String expectedResponse = "+QMTCONN: 3,0,0";             // Expected response
  unsigned long timeout = 6000;                            // 6 seconds timeout

  String response = sendATCommand3(command, command, timeout); // Expect echo of the command

  if (response == command)
  { // Command echo received, now expect +QMTCONN: 3,0,0
    response = sendATCommand3("", expectedResponse, timeout);
  }

  if (response == expectedResponse)
  {
    Serial.println("MQTT connected to specific client ID successfully");
  }
  else
  {
    Serial.println("Failed to connect to MQTT with specific client ID");
  }
}

void subscribeMQTT(const String &topic)
{
  String command = "AT+QMTSUB=3,1,\"" + topic + "\",0"; // AT command to subscribe to MQTT topic
  String expectedResponse = "+QMTSUB: 3,1,0,0";         // Expected response
  unsigned long timeout = 6000;                         // 6 seconds timeout

  String response = sendATCommand3(command, command, timeout); // Expect echo of the command

  if (response == command)
  { // Command echo received, now expect +QMTSUB: 3,1,0,0
    response = sendATCommand3("", expectedResponse, timeout);
  }

  if (response == expectedResponse)
  {
    Serial.println("Successfully subscribed to topic " + topic);
  }
  else
  {
    Serial.println("Failed to subscribe to topic " + topic);
  }
}



void setup() {
  // put your setup code here, to run once:
  Serial.println("Sample EG25 program is started");
  delay(6000);
  String deviceModel = getDeviceModel();
  Serial.println("Device Model: " + deviceModel);
  Serial.begin(allSerialBraudrate);
  eg25.begin(allSerialBraudrate);
  delay(2000);
  checkSimCardStatus();
  delay(1000);
  activateAPN();
  delay(1000);
  setInternetRegistration();
  delay(200);
  activateInternet();
  delay(1000);
  getConnectedNetwork();
  delay(100);
  // Print the operator name and network type
  Serial.print("Operator Name: ");
  Serial.println(operatorName);
  Serial.print("Network Type: ");
  Serial.println(networkTypeString);
  resetMQTT();
  delay(500);
  setMQTTMode();
  delay(500);
  openMQTT();
  delay(500);
  connectMQTT();
  delay(500);
  subscribeMQTT("cool/nfc");
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


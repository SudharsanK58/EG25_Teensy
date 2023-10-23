#include <Arduino.h>
int allSerialBraudrate = 115200;
HardwareSerial &eg25 = Serial1;          // Define Serial1 as EG25-G
HardwareSerial &espSerial = Serial2;
String globalURL = "https://zig-app.com/Ticket/api/Hardware/Hardwaresettings/04:e9:e5:15:70:91"; // Replace with your desired URL

String jsonResponse = ""; // Declare a global String variable
String previousResponse = ""; // Store the previous response
bool jsonStarted = false; 
String extractedJSON = "";

const String primarySSID = "Zusan"; // Replace with your primary SSID
const String primaryPassword = "Wireless4U!"; // Replace with your primary Wi-Fi password
const String secondarySSID = "Zed"; // Replace with your secondary SSID
const String secondaryPassword = "Wireless4U!"; // Replace with your secondary Wi-Fi password
const unsigned long WifiTimeout = 30000; // 5 seconds


String sendWifiATCommands(const String &command, const String &expectedResponse, unsigned long timeout)
{
  espSerial.println(command);
  // Serial.println("Command sent: " + command); // Debug print

  unsigned long startTime = millis();

  while (millis() - startTime < timeout)  
  {
    if (espSerial.available())
    {
      String response = espSerial.readStringUntil('\n');
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
void resetAndDisconnect()
{
  unsigned long timeout = 5000; // 5 seconds

  // Send AT+RST
  String response = sendWifiATCommands("AT+RST", "ready", timeout);

  if (response == "ready") {
    Serial.println("ESP32 Successful (Ready)");
    // Proceed to disconnect
    response = sendWifiATCommands("AT+CWQAP", "OK", timeout);

    if (response == "OK") {
      Serial.println("WIFI reset Successful");
    } else {
      Serial.println("WIFI reset Failed");
    }
  } else {
    Serial.println("WIFI Init failed");
  }
}

void configureReconnect()
{
  unsigned long timeout = 5000; // 5 seconds
  String command = "AT+CWRECONNCFG=2,5";
  String response = sendWifiATCommands(command, "OK", timeout);
  
  if (response == "OK") {
    Serial.println("WIFI reconnect config Successful");
  } else {
    Serial.println("WIFI reconnect config Failed");
  }
}

void connectToWiFi()
{
  // Attempt to connect to the primary Wi-Fi network
  espSerial.print("AT+CWJAP=\"");
  espSerial.print(primarySSID);
  espSerial.print("\",\"");
  espSerial.print(primaryPassword);
  espSerial.println("\"");

  unsigned long startTime = millis();
  boolean espWifiConnected = false;
  boolean errorOccurred = false;
  String errorResponse;

  while (millis() - startTime < WifiTimeout) {
    if (espSerial.available()) {
      String response = espSerial.readStringUntil('\n');

      if (response.indexOf("WIFI CONNECTED") != -1) {
        espWifiConnected = true;
        break;
      } else if (response.indexOf("+CWJAP:") != -1) {
        // An error code is received, parse it
        int errorCode = response.substring(response.indexOf("+CWJAP:") + 7).toInt();
        String errorMessage;
        
        switch (errorCode) {
          case 1:
            errorMessage = "Connection timeout.";
            break;
          case 2:
            errorMessage = "Wrong password.";
            break;
          case 3:
            errorMessage = "Cannot find the target AP.";
            break;
          case 4:
            errorMessage = "Connection failed.";
            break;
          default:
            errorMessage = "Unknown error occurred.";
        }

        errorResponse = "Failed to connect to primary WIFI : " + errorMessage;
        errorOccurred = true;
        break;
      }
    }
  }

  if (espWifiConnected) {
    // "WIFI CONNECTED" received, now wait for "OK"
    startTime = millis();
    boolean okReceived = false;

    while (millis() - startTime < WifiTimeout) {
      if (espSerial.available()) {
        String response = espSerial.readStringUntil('\n');

        if (response.indexOf("OK") != -1) {
          okReceived = true;
          break;
        }
      }
    }

    if (okReceived) {
      Serial.println("Connected to Wi-Fi: Successful (Primary)");
    }
  } else if (errorOccurred) {
    Serial.println(errorResponse);
    // Attempt to connect to the secondary Wi-Fi network
    Serial.println("Reconnceting to secondary WIFI ");
    espSerial.print("AT+CWJAP=\"");
    espSerial.print(secondarySSID);
    espSerial.print("\",\"");
    espSerial.print(secondaryPassword);
    espSerial.println("\"");

    startTime = millis();
    espWifiConnected = false;
    errorOccurred = false;

    while (millis() - startTime < WifiTimeout) {
      if (espSerial.available()) {
        String response = espSerial.readStringUntil('\n');

        if (response.indexOf("WIFI CONNECTED") != -1) {
          espWifiConnected = true;
          break;
        } else if (response.indexOf("+CWJAP:") != -1) {
          // An error code is received, parse it
          int errorCode = response.substring(response.indexOf("+CWJAP:") + 7).toInt();
          String errorMessage;
          
          switch (errorCode) {
            case 1:
              errorMessage = "Connection timeout.";
              break;
            case 2:
              errorMessage = "Wrong password.";
              break;
            case 3:
              errorMessage = "Cannot find the target AP.";
              break;
            case 4:
              errorMessage = "Connection failed.";
              break;
            default:
              errorMessage = "Unknown error occurred.";
          }

          errorResponse = "Failed to connect to secondary WIFI " + errorMessage;
          errorOccurred = true;
          break;
        }
      }
    }

    if (espWifiConnected) {
      // "WIFI CONNECTED" received, now wait for "OK"
      startTime = millis();
      boolean okReceived = false;

      while (millis() - startTime < WifiTimeout) {
        if (espSerial.available()) {
          String response = espSerial.readStringUntil('\n');

          if (response.indexOf("OK") != -1) {
            okReceived = true;
            break;
          }
        }
      }

      if (okReceived) {
        Serial.println("Connected to Wi-Fi: Successful (Secondary)");
      } 
    } else if (errorOccurred) {
      Serial.println(errorResponse);
    } 
  }
}
void enableAutoConnect()
{
  String command = "AT+CWAUTOCONN=1";
  unsigned long timeout = 5000; // 5 seconds

  String response = sendWifiATCommands(command, "OK", timeout);
  
  if (response == "OK") {
    Serial.println("Autoconnect enabled: Successful");
  } else {
    Serial.println("Autoconnect enabling: Failed");
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.println("Sample EG25 program is started");
  Serial.begin(allSerialBraudrate);
  espSerial.begin(allSerialBraudrate);
  Serial.println("Please wait......");
  delay(7000);
  resetAndDisconnect();
  delay(2000);
  configureReconnect();
  delay(2000);
  connectToWiFi();
  delay(2000);
  enableAutoConnect();
  delay(2000);
}
void loop() {
   if (espSerial.available()) {
    char receivedChar = espSerial.read();
    Serial.print(receivedChar);
  }
  if (Serial.available()) {
    char commandChar = Serial.read();
    espSerial.print(commandChar);
  }
}


#include <Arduino.h>
int allSerialBraudrate = 115200;
HardwareSerial &eg25 = Serial1;          // Define Serial1 as EG25-G
String globalURL = "https://zig-app.com/Ticket/api/Hardware/Hardwaresettings"; // Replace with your desired URL

String jsonResponse = ""; // Declare a global String variable
String previousResponse = ""; // Store the previous response
bool jsonStarted = false; 
String extractedJSON = "";

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



void get4GApiReponse()
{
  // Send the AT+QHTTPSTOP command to stop any ongoing HTTP(S) request.
  String command = "AT+QHTTPSTOP";
  String expectedResponse = "OK";
  unsigned long timeout = 5000; // Adjust the timeout as needed.

  String response = sendATCommand3(command, expectedResponse, timeout);

  if (response == "OK")
  {
    Serial.println("HTTP(S) request stopped successfully.");
  }
  else if (response == "TIMEOUT")
  {
    Serial.println(command + "Timeout while waiting for response.");
  }
  else
  {
    Serial.println("Error: " + response);
  }

  // Send the AT+QHTTPCFG="contextid",1 command.
  command = "AT+QHTTPCFG=\"contextid\",1";
  expectedResponse = "OK";
  timeout = 5000; // Adjust the timeout as needed.

  response = sendATCommand3(command, expectedResponse, timeout);

  if (response == "OK")
  {
    Serial.println("Context ID configured successfully.");
  }
  else if (response == "TIMEOUT")
  {
    Serial.println(command + "Timeout while waiting for response.");
    return; 
  }
  else
  {
    Serial.println("Error: " + response);
    return; 
  }

  command = "AT+QHTTPCFG=\"responseheader\",1";
  expectedResponse = "OK";
  timeout = 5000; 

  response = sendATCommand3(command, expectedResponse, timeout);

  if (response == "OK")
  {
    Serial.println("Response header configured successfully.");
  }
  else if (response == "TIMEOUT")
  {
    Serial.println(command + "Timeout while waiting for response.");
    return; 
  }
  else
  {
    Serial.println("Error: " + response);
    return; 
  }


  command = "AT+QIACT=1";
  expectedResponse = "OK";
  timeout = 15000; 

  response = sendATCommand3(command, expectedResponse, timeout);

  if (response == "OK")
  {
    Serial.println("PDP context activated successfully.");
  }
  else if (response == "TIMEOUT")
  {
    Serial.println(command + "Timeout while waiting for response.");
  }
  else
  {
    Serial.println("Error: " + response);
  }
  
  String urlSize = String(globalURL.length());
  // Set the URL using the AT+QHTTPURL command.
  command = "AT+QHTTPURL=" + urlSize + ",80";
  expectedResponse = "CONNECT";
  timeout = 10000; // Adjust the timeout as needed.

  response = sendATCommand3(command, expectedResponse, timeout);

  if (response == "CONNECT")
  {
    Serial.println("AT+QHTTPURL request is ready to accept the URL.");
  }
  else if (response == "TIMEOUT")
  {
    Serial.println("Timeout while waiting for the CONNECT response.");
    return; 
  }
  else
  {
    Serial.println("Error: " + response);
    return; 
  }


  command = globalURL;
  expectedResponse = "OK";
  timeout = 10000; 

  response = sendATCommand3(command, expectedResponse, timeout);

  if (response == "OK")
  {
    Serial.println("URL set successfully.");
  }
  else if (response == "TIMEOUT")
  {
    Serial.println(command + "Timeout while waiting for response.");
    return; 
  }
  else
  {
    Serial.println("Error: " + response);
    return; 
  }

  command = "AT+QHTTPGET=20";
  expectedResponse = "+QHTTPGET: 0,200"; 
  timeout = 30000;

  response = sendATCommand3(command, expectedResponse, timeout);

  if (response == "+QHTTPGET: 0,200")
  {
    Serial.println("HTTP GET request sent, waiting for 20 seconds...");
  }
  else if (response == "TIMEOUT")
  {
    Serial.println(command + "Timeout while waiting for response.");
  }
  else
  {
    Serial.println("Error: " + response);
  }


 
  command = "AT+QHTTPREAD=80";
  timeout = 15000; 
  eg25.println(command);
  jsonResponse = "";
  while (true)
  {
    if (eg25.available())
    {
      String responseLine = eg25.readStringUntil('\n');

      if (responseLine.startsWith("{"))  
      {
        Serial.println("HTTP Response:");
        jsonStarted = true;
      }

      if (jsonStarted)
      {
        jsonResponse += responseLine;
        if (responseLine.indexOf("}") != -1)
        {
          extractedJSON = jsonResponse;
          jsonResponse = ""; 
          jsonStarted = false; 
          break; 
        }
      }
      if (responseLine == "OK" || responseLine == "+QHTTPREAD: 0")
      {
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.println("Sample EG25 program is started");
  Serial.begin(allSerialBraudrate);
  eg25.begin(allSerialBraudrate);
  Serial.println("Please wait......");
  delay(12000);
  get4GApiReponse();
  Serial.println(extractedJSON);
}
void loop() {
  //  if (eg25.available()) {
  //   char receivedChar = eg25.read();
  //   Serial.print(receivedChar);
  // }
  // if (Serial.available()) {
  //   char commandChar = Serial.read();
  //   eg25.print(commandChar);
  // }
}


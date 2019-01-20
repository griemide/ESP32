/*
 * SKETCH:
 *  af104-tbr.ino version 19.1.20 (main module)  
 * 
 *  (c) 2019,  Michael Gries
 *  Creation:  2019-01-19 (copied from MATLAB / ThinkSpeak example for ESP32)
 *  Modified:  2019-01-20 (fixed ISSUE on boards tested) 
 *  
 * MODULES:
 *   none
 * 
 * PREREQUISITES:
 *   1. requires ThingSpeak channel for status report (new channel af104-tbr created. 682562)
 *   2. requires ThingSpeak TalkBack App (new App TalkBack_af104-tbr created)
 *  
 * SPECS:
 *   none
 * 
 * LINKS:
 *   see https://de.mathworks.com/help/thingspeak/control-a-light-with-talkback-and-esp32.html
 *   see GET https://api.thingspeak.com/talkbacks/30428/commands.json?api_key=xxxxxxxxxxxxxxxx
 *   
 * LIBRARYS:
 *   WiFi.h (core package); SPI.h not required as given in original MATLAB example
 *   
 * ARCHIVE:
 *   !!! ATTTENTION !!! (mask credential before archiving)
 *   see https://github.com/griemide/ESP32/tree/master/projects/af104-tbr
 *   
 * BOARDS TESTED:
 *   see https://github.com/griemide/ESP32/tree/master/Boards
 *   1. WEMOS mini 32 (CP2104) with board configuration WEMOS LOLIN32
 *   2. WEMOS D1 R32  (CH340G) with board configuration WEMOS LOLIN32
 *   
 * ISSUES:
 *   LED_BUILTIN=5 not usable according boards tested (workaround: see #ifdef ...)
 *   
 */

/*
FetchCommandFromTalkBack

Description: 
Checks a TalkBack queue every 60 seconds and set the state of the build in LED according
to the latest command fetched. Turn the LED on and off by using the commands TURN_ON and TURN_OFF.
The TalkBack documentation can be found at: https://www.mathworks.com/help/thingspeak/talkback-app.html
             
Hardware: 
ESP32 based boards

Notes:
- Requires installation of EPS32 core. 
  See https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md for details. 
- Select the target hardware from the Tools ->Board menu (and corresponding COM port)

Copyright 2018, The MathWorks, Inc
*/


#ifdef ARDUINO_LOLIN32
  #define SERIAL_BAUDRATE 115200
  //#include <SPI.h>
  #include <WiFi.h>
  #define LED_BUILTIN 2
#endif

#define POLLING_RATE 60000 // milliseconds

char ssid[] = "HP LaserJet P1505n";   // your network SSID (name) 
char pass[] = "xxxxxxxxxxxxxxxxxx";   // your network password

WiFiClient  client;

unsigned long myTalkBackID = 30428; // using TalkBack app TalkBack_af104-tbr
const char * myTalkbackKey = "xxxxxxxxxxxxxxxx";

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // set up LED
  Serial.begin(SERIAL_BAUDRATE); // Initialize serial (board dependent ESP8266 vs. ESP32)
  Serial.print("LED_BUILTIN: "); Serial.println(LED_BUILTIN);
}

void loop() {

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(String(ssid));
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
    digitalWrite(LED_BUILTIN, HIGH);  
  }

  // create the TalksBack URI
  String tbURI = String("/talkbacks/") + String(myTalkBackID) + String("/commands/execute");
  
  // Create the message body for the POST out of the values
  String postMessage =  String("apikey=") + String(myTalkbackKey);                      
                       
   // make a String for any commands that might be in the queue
  String newCommand = String();

  // make the POST to ThingSpeak
  int x = httpPOST(tbURI, postMessage, newCommand);
  client.stop();
  
  // Check the result
  if(x == 200){
    Serial.println("checking queue..."); 
    // check for a command returned from Talkback
    if(newCommand.length() != 0){

      Serial.print("  Latest command from queue: ");
      Serial.println(newCommand);
      
      if(newCommand == "TURN_ON"){
        digitalWrite(LED_BUILTIN, HIGH);  
      }

      if(newCommand == "TURN_OFF"){
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
    else{
      Serial.println("... nothing new !");  
    }
    
  }
  else{
    Serial.println("Problem checking queue. HTTP error code " + String(x));
  }
  
  delay(POLLING_RATE); // Wait defined milliseconds to check queue again
}

// general function to POST to ThingSpeak
int httpPOST(String uri, String postMessage, String &response){

  bool connectSuccess = false;
  connectSuccess = client.connect("api.thingspeak.com",80);

  if(!connectSuccess){
      return -301;   
  }
  
  postMessage += "&headers=false";
  
  String Headers =  String("POST ") + uri + String(" HTTP/1.1\r\n") +
                    String("Host: api.thingspeak.com\r\n") +
                    String("Content-Type: application/x-www-form-urlencoded\r\n") +
                    String("Connection: close\r\n") +
                    String("Content-Length: ") + String(postMessage.length()) +
                    String("\r\n\r\n");

  client.print(Headers);
  client.print(postMessage);

  long startWaitForResponseAt = millis();
  while(client.available() == 0 && millis() - startWaitForResponseAt < 5000){
      delay(100);
  }

  if(client.available() == 0){       
    return -304; // Didn't get server response in time
  }

  if(!client.find(const_cast<char *>("HTTP/1.1"))){
      return -303; // Couldn't parse response (didn't find HTTP/1.1)
  }
  
  int status = client.parseInt();
  if(status != 200){
    return status;
  }

  if(!client.find(const_cast<char *>("\n\r\n"))){
    return -303;
  }

  String tempString = String(client.readString());
  response = tempString;
  
  return status;
    
}

/*EOF*/

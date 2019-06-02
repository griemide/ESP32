/* SKETCH:
 *   ESP-DASH.ino version 19.6.2 (main module)  
 * 
 *   (c) 2019,  Michael Gries
 *   Creation:  2019-05-30 (granted from https://github.com/ayushsharma82/ESP-DASH)
 *   Modified:  2019-06-01 (ElegantOTA handling added) 
 *   Modified:  2019-06-02 (ESP32 hall and temperature sensor added)
 *  
 * MODULES:
 *   none
 * 
 * PREREQUISITES:
 *   1. requires libraries below
 *   2. modify AsyncElegantOTA.h by uncomment <Hash.h>
 *  
 * SPECS:
 *   see https://github.com/ayushsharma82/ESP-DASH/wiki/Code-Reference
 * 
 * LINKS:
 *   see http://192.168.0.59 (see used IP number shown on serial port after reset)
 *   see https://github.com/ayushsharma82/AsyncElegantOTA (OTA feature added
 *   see http://192.168.0.59/update (local OTA webpage
 *   
 * LIBRARYS:
 *   WiFi.h (core package); 
 *   
 * ARCHIVE:
 *   !!! ATTTENTION !!! (mask credential before archiving on cloud server)
 *   see https://github.com/griemide/ESP32/tree/master/project
 *   
 * BOARDS TESTED:
 *   see https://github.com/griemide/ESP32/tree/master/Boards
 *   1. TTGO T5 V2.3 2.13 e-ink board with configuration 'TTGO LoRa32-OLED V1' on Port 10
 *   
 * ISSUES:
 *   Hash.h library issue within ESP8266 and ESP32 core   
 *   
 */

/*
* ESP-DASH V2
* Made by Ayush Sharma
* Github URL: https://github.com/ayushsharma82/ESP-DASH
* Support Me: https://www.patreon.com/asrocks5
*
* - Version Changelog - 
* V1.0.0 - 11 Nov. 2017 - Library was Born
* V2.0.0 - 25 Jan. 2019 - Wohoo! A breakthrough in performance and capabilities!
*/

#include <Arduino.h>
#include <WiFi.h>
//#include <Hash.h>  
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include <AsyncElegantOTA.h>  // includes <Hash.h> nut not available and or required

AsyncWebServer server(80);

const char* ssid = "HP LaserJet P1505n"; // Your WiFi SSID
const char* password = ""; // Your WiFi Password


void sliderChanged(const char* id, int value){
   Serial.println("Slider Changed - "+String(id)+" "+String(value));
   ESPDash.updateGaugeChart("gauge1", value);
}

int indicator = 2; // LED port for buttonClicked()


void buttonClicked(const char* id){
   Serial.println("Button Clicked - "+String(id));
   digitalWrite(indicator, HIGH);
   delay(100);
   digitalWrite(indicator, LOW);
}


// Line Chart Data
int x_axis_size = 7;
String x_axis[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}; 
int y_axis_size = 7;
int y_axis[7] = {2, 5, 10, 12, 18, 8, 5};

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read(); // °F
int     temp_c; // °C

void setup() {
    Serial.begin(115200);
    pinMode(indicator, OUTPUT);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    ESPDash.init(server);   // Initiate ESPDash and attach your Async webserver instance
    
    // Attach Slider Change Events
    ESPDash.attachSliderChanged(sliderChanged);

    ESPDash.addSliderCard("slider1", "Gauge Slider", 3);
    ESPDash.addGaugeChart("gauge1", "Gauge 1");
    ESPDash.addLineChart("chart1", "Example Line Chart", x_axis, x_axis_size, "Power KwH", y_axis, y_axis_size);
    ESPDash.addStatusCard("status0", "Status chart", 0);
    ESPDash.addGaugeChart("chart2", "Gauge random value", 30);
    ESPDash.addNumberCard("num1", "Hall sensor (internal)", hallRead());
    ESPDash.addTemperatureCard("temp1", "Temp. sensor (internal)", 0, temp_c);
    ESPDash.addHumidityCard("hum1", "Humidity Card", 98);
    ESPDash.addButtonCard("btn1", "Blink Button");
    ESPDash.attachButtonClick(buttonClicked);


    Serial.println("OTA feature activated. Call <IPaddress>/update");
    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
}

void loop() {
    AsyncElegantOTA.loop();
        // Fill Data with random Values
    for(int i=0; i < y_axis_size; i++){
        y_axis[i] = random(2, 20);
    }

    // Convert raw temperature in F to Celsius degrees
    temp_c = (temprature_sens_read() - 32) / 1.8;
    ESPDash.updateLineChart("chart1", x_axis, x_axis_size, y_axis, y_axis_size);
    ESPDash.updateGaugeChart("chart2", 50);
    delay(1000);
    ESPDash.updateGaugeChart("chart2", 80);
    delay(1000);
    ESPDash.updateGaugeChart("chart2", 10);
    delay(1000);
    ESPDash.updateNumberCard("num1", hallRead());
    ESPDash.updateTemperatureCard("temp1", temp_c);
    ESPDash.updateHumidityCard("hum1", random(0, 100));
    ESPDash.updateStatusCard("status0", 1);
}

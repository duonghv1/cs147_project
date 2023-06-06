#include <string>
#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>

#include <Adafruit_AHTX0.h>
#include "light.h"

#define CALIBRATION_ON false
#define LIGHT_SENSOR 33
#define MIC_SENSOR 15

#define DEBUG_MODE false


Adafruit_AHTX0 aht;

char ssid[50] = "Gone with the Wind"; //"UCInet Mobile Access"; // your network SSID (name)
char pass[50] = "zotzotzot"; //{0}; // your network password (use for WPA, or use as key for WEP)

String public_IP = "54.177.115.132";//"18.219.240.227";
const int port = 5000;
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

const int uploadFreq = 20*1000; //20 sec per update to the server
const int samplingFreq = 5*1000; //5 sec => for each sensor
unsigned long upload_timer;

String var;
sensors_event_t humidity, temp;
uint16_t light_read;
double light_val;
int sound_val = 0;

void AHT_setup(){
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  // Serial.println("AHT10 or AHT20 found");
}

void wifi_setup() {
  Serial.begin(9600);
  delay(1000);
  // Retrieve SSID/PASSWD from flash before anything else
  //nvs_access();

  // We start by connecting to a WiFi network
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");

  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
}

void setup(){
  wifi_setup();

  AHT_setup();
  if (CALIBRATION_ON){
    light_calibration(LIGHT_SENSOR);
  }

  //microphone setup
  pinMode(MIC_SENSOR, INPUT);
  upload_timer = millis() + uploadFreq;
}

void loop() {
  // Read Temperature and Humidity Data
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

  // Read Light Data
  light_read = analogRead(LIGHT_SENSOR);
  light_val = map(light_read)*100;

  // Read Sound Data
  sound_val += digitalRead(MIC_SENSOR);
  
  // PRINT RESULTS
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  Serial.print("Light Level: "); Serial.print(light_val, 2); Serial.println("%");
  Serial.print("Sound Exists: "); Serial.println(sound_val);

  // Upload data to the server
  // Question: 
  //    Is MIC connected to the same board as other sensors?
  //       if yes => Temp & Light sensor only need to sample at the same rate as uploading rate
  //       if no => the ESP32 connected with Temp&Light can implement sleeping mode; and the data collecting format(var=?) needs to be changed
  //        ==> we can add a name to each parameter such as "temp=None,light=None,mic=3"
  if (!DEBUG_MODE){
    if (millis() > upload_timer){
      int err = 0;
      WiFiClient c;
      HttpClient http(c);

      var = (String)"/?var=" + (String)temp.temperature + "," + (String)humidity.relative_humidity;
      var += "," + (String)light_val;
      var += "," + (String)sound_val;

      err = http.get(public_IP.c_str(), port, var.c_str());
      sound_val = 0;
    
      if (err == 0) {
        // Serial.println("\nstartedRequest ok");
        err = http.responseStatusCode();
        if (err >= 0) {
          // Serial.print("Got status code: ");
          // Serial.println(err);

          err = http.skipResponseHeaders();
          if (err >= 0) {
            int bodyLen = http.contentLength();
            // Serial.print("Content length is: ");
            // Serial.println(bodyLen);
            // Serial.println();
            // Serial.println("Body returned follows:");

            // Now we've got to the body, so we can print it out
            unsigned long timeoutStart = millis();
            char c;
            // Whilst we haven't timed out & haven't reached the end of the body
            while ((http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout)) {
              if (http.available()) {
                c = http.read();
                // Print out this character
                // Serial.print(c);
                
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
              } else {
                // We haven't got any data, so let's pause to allow some to arrive
                delay(kNetworkDelay);
              }
            }
          } else {
            Serial.print("Failed to skip response headers: ");
            Serial.println(err);
          }
        } else {
          Serial.print("Getting response failed: ");
          Serial.println(err);
        }
      } else {
        Serial.print("Connect failed: ");
        Serial.println(err);
      }
      http.stop();
    }
    // End of Upload
  }
  else{
    sound_val = 0;
  }
  Serial.println();
  delay(samplingFreq);
}
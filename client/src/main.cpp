#include <string>
#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <Adafruit_AHTX0.h>
#include "light.h"


#define CALIBRATION_ON false
#define LIGHT_SENSOR 33


Adafruit_AHTX0 aht;

char ssid[50] = "UCInet Mobile Access";//"Gone with the Wind"; // your network SSID (name)
char pass[50] = {0}; //"zotzotzot"; // your network password (use for WPA, or use as key for WEP)


// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;
// static int LIGHT_SENSOR = 33;

String var;
sensors_event_t humidity, temp;
uint16_t light_read;
double light_val;
String public_IP = "54.177.115.132";//"18.219.240.227";

void nvs_access() {
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
    err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  // Open
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
  switch (err) {
    case ESP_OK:
      Serial.printf("Done\n");
      //Serial.printf("SSID = %s\n", ssid);
      //Serial.printf("PASSWD = %s\n", pass);
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      Serial.printf("The value is not initialized yet!\n");
      break;
    default:
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }

  // Close
  nvs_close(my_handle);
}

void sensor_configuration(){
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  // Serial.println("AHT10 or AHT20 found");
}

void setup() {
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

  sensor_configuration();
  if (CALIBRATION_ON){
    light_calibration(LIGHT_SENSOR);
  }
}

void loop() {
  // Read Temperature and Humidity Data
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

  // Read Light Data
  light_read = analogRead(LIGHT_SENSOR);
  light_val = map(light_read);

  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  Serial.print("Light Level: "); Serial.print(light_val*100, 3); Serial.println("%");

  // Print to the Webpage
  int err = 0;
  WiFiClient c;
  HttpClient http(c);

  var = (String)"/?var=" + (String)temp.temperature + "," + (String)humidity.relative_humidity;
  var += "," + (String)light_val;

  err = http.get(public_IP.c_str(), 5000, var.c_str());
  // err = http.get("127.0.0.1", 5000, var.c_str());
  
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
  
  Serial.println();
  // And just stop, now that we've tried a download
  delay(10000);
}

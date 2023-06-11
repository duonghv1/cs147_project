#include <string>
#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include <TFT_eSPI.h>
#include <Adafruit_AHTX0.h>

#include "light.h"

#define LIGHT_SENSOR 32
#define BUTTON_PIN 33

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

#define DEBUG_MODE false
#define CALIBRATION_ON false

// Temperature & Humidity configuration
Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;

bool onMode = true;

// Light (photosensor) configuration
uint16_t light_read;
double light_val;

// Wi-Fi configuration
char ssid[50] = "Gone with the Wind"; //"UCInet Mobile Access"; // your network SSID (name)
char pass[50] = "zotzotzot"; //{0}; // your network password (use for WPA, or use as key for WEP)

String public_IP = "54.177.115.132";//"18.219.240.227";
const int port = 5000;

// const int uploadFreq = 20*1000; //20 sec per update to the server
// const int samplingFreq = 5*1000; //5 sec => for each sensor
// unsigned long upload_timer;
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  30
RTC_DATA_ATTR int bootCount= 0;

String queryString;

// TFT Configuration
TFT_eSPI tft = TFT_eSPI();
void print_wakeup_reason();
void print(String s);
void tft_setup();
void AHT_setup();
void wifi_setup();
void IRAM_ATTR turnOff();
void determineOnOff();
void showLogo();

void setup(){
  Serial.begin(9600);
  delay(1000);

  tft_setup();

  // Button & Sleep Mode setup
  //esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
  delay(100);

  // determineOnOff();
  wifi_setup();
  //delay(1000);

  AHT_setup();
  if (CALIBRATION_ON){
    light_calibration(LIGHT_SENSOR);
  }
  else{
    default_calibration();
  }

  pinMode(BUTTON_PIN, INPUT);
  // attachInterrupt(BUTTON_PIN, turnOff, HIGH); 

  print("Ready...!");
  tft.fillScreen(TFT_BLACK);
  digitalWrite(38, LOW); // turn off the back light of lcd

  // Upload & Sleep Mode setup
  // Set timer to 30 seconds
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(100);
}

void loop() {
  determineOnOff();
  // Read Temperature and Humidity Data
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

  // Read Light Data
  light_read = analogRead(LIGHT_SENSOR);
  light_val = map(light_read);

  // PRINT RESULTS
  Serial.println("*************One Upload*****************");
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  Serial.print("Light Level: "); Serial.print(light_val, 2); Serial.println("%");

  if (!DEBUG_MODE){
    int err = 0;
    WiFiClient c;
    HttpClient http(c);

    queryString = (String)"/?var1=" + (String)temp.temperature + "," + (String)humidity.relative_humidity;
    queryString += "," + (String)light_val;
    // print("temp="+(String)temp.temperature);
    // print("humidity="+(String)humidity.relative_humidity);
    // print("light="+(String)light_val);

    err = http.get(public_IP.c_str(), port, queryString.c_str());
  
    if (err == 0) {
      // Serial.println("\nstartedRequest ok");
      err = http.responseStatusCode();
      if (err >= 0) {
        Serial.print("Got status code: "); Serial.println(err);
      } else {
        Serial.print("Getting response failed: "); Serial.println(err);
      }
    } else {
      Serial.print("Connect failed: "); Serial.println(err);
    }
    http.stop();
    // End of Upload
  }

  delay(500);
  esp_light_sleep_start();
}

// Below handler is not used
void IRAM_ATTR turnOff() {
  Serial.println("In turnOff()");
  print("Power off...");
  onMode = false;
  delay(1000);
  esp_light_sleep_start();
  delay(100);
  Serial.println("Never got printed");
}

void determineOnOff(){
  esp_sleep_wakeup_cause_t wake_up_source;
  wake_up_source = esp_sleep_get_wakeup_cause();
  if (wake_up_source == ESP_SLEEP_WAKEUP_EXT0){
    if (onMode){
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);

      print("Power off...");
      delay(1000);
      tft.fillScreen(TFT_BLACK);
      digitalWrite(38, LOW); // turn off the back light of lcd

      onMode = false;
      esp_light_sleep_start();
      
      // Where it wakes up
      onMode=true;
      delay(100);
      showLogo();
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      delay(100);
    }
  }
}

void print_wakeup_reason(){
   esp_sleep_wakeup_cause_t wake_up_source;

   wake_up_source = esp_sleep_get_wakeup_cause();

   switch(wake_up_source){
      case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wake-up from external signal with RTC_IO"); break;
      case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wake-up from external signal with RTC_CNTL"); break;
      case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wake up caused by a timer"); break;
      case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wake up caused by a touchpad"); break;
      default : Serial.printf("Wake up not caused by Deep Sleep: %d\n",wake_up_source); break;
   }
}

void print(String s){
  tft.fillScreen(TFT_PURPLE);
  tft.setTextColor(TFT_WHITE, TFT_PURPLE);

  tft.setTextDatum(MC_DATUM);
  tft.drawString(s, tft.width()/2, tft.height()/2);
  delay(1000);
}

void showLogo(){
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Anteater", tft.width()/2, 30);
  tft.drawString("Sleep", tft.width()/2, tft.height()/2);
  tft.drawString("Awareness", tft.width()/2, tft.height()-30);

  delay(2000);
  tft.setTextSize(2);
}

void tft_setup(){
  // OLED Display Configuration
  tft.init();
  tft.setRotation(1);
  tft.setTextSize(3);
  tft.fillScreen(TFT_PURPLE);
  tft.setTextColor(TFT_WHITE, TFT_PURPLE);
  // tft.setTextSize(3);

  showLogo();
}

void AHT_setup(){
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  // Serial.println("AHT10 or AHT20 found");
}

void wifi_setup() {
  // We start by connecting to a WiFi network
  print("Connecting to Wi-Fi");
  Serial.print("\nConnecting to "); Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  Serial.println("MAC address: "); Serial.println(WiFi.macAddress());
  Serial.println();
}

#include <string>
#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include <TFT_eSPI.h>

#include "SparkFunLSM6DSO.h"
#include "Wire.h"

#define MIC_SENSOR 15
#define GYROINT_PIN 32
#define BUTTON_PIN 33

#define DEBUG_MODE false  //false: upload value to the server
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

const int threshold = 130;
const int lower_bound_threshold = 90;

// Accelerometer & Gyroscope Configuration
LSM6DSO myIMU;
float xavg, yavg, zavg, x, y, z, total;
bool has_motion = false;
int motion_val = 0;

// Microphone Configuration
int sound_val = 0;
bool has_sound = false;

// Wi-Fi Configuration
char ssid[50] = "UCInet Mobile Access"; //"Gone with the Wind";  // your network SSID (name)
char pass[50] = {0}; //"zotzotzot"; // your network password (use for WPA, or use as key for WEP)

String public_IP = "54.177.115.132";//"18.219.240.227";
const int port = 5000;
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

const int uploadFreq = 20*1000; //20 sec per update to the server
// const int samplingFreq = 50; //5 sec => for each sensor
const int soundDetectFreq = 100; //Ref: duration of a finger click/snap is 50 ms ~ 150 ms
unsigned long upload_timer;
unsigned long sound_timer;

String queryString;

// TFT Configuration
TFT_eSPI tft = TFT_eSPI();

void print(String s){
  tft.fillScreen(TFT_PURPLE);
  tft.setTextColor(TFT_WHITE, TFT_PURPLE);

  tft.setTextDatum(MC_DATUM);
  tft.drawString(s, tft.width()/2, tft.height()/2);
}

void tft_setup(){
  // OLED Display Configuration
  tft.init();
  tft.setRotation(1);
  tft.setTextSize(3);
  tft.fillScreen(TFT_PURPLE);
  tft.setTextColor(TFT_WHITE, TFT_PURPLE);
  // tft.setTextSize(3);

  tft.setTextDatum(MC_DATUM);
  tft.drawString("Anteater", tft.width()/2, 30);
  tft.drawString("Sleep", tft.width()/2, tft.height()/2);
  tft.drawString("Awareness", tft.width()/2, tft.height()-30);

  delay(2000);
  tft.setTextSize(2);
}

void gyroCalibrate(){
  print("Start Calibration");
  float sum, sum1, sum2 = 0;
  float xval[100] = {};
  float yval[100] = {};
  float zval[100] = {};

  //x-axis calibration
  for (int i = 0; i < 100; i++) {
      // xval[i] = myIMU.readFloatAccelX();
      xval[i] = myIMU.readFloatGyroX();
      sum = xval[i] + sum;

      yval[i] = myIMU.readFloatGyroY();
      sum1 = yval[i] + sum1;

      zval[i] = myIMU.readFloatGyroZ();
      sum2 = zval[i] + sum2;
      delay(20);
  }

  xavg = sum / 100.0;
  yavg = sum1 / 100.0;
  zavg = sum2 / 100.0;

  Serial.println("Calibration values:");
  Serial.print("xavg="); Serial.print(xavg);
  Serial.print(", yavg="); Serial.print(yavg);
  Serial.print(", zavg="); Serial.println(zavg);
  delay(1000);
}

void wifi_setup() {
  print("Connecting to Wi-Fi");
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void IMU_congifuration(){
  Wire.begin();
  if( myIMU.begin() )
    Serial.println("Ready.");
  else { 
    Serial.println("Could not connect to IMU.");
    Serial.println("Freezing");
    while(1);
  }
  if( !myIMU.initialize(HARD_INT_SETTINGS) ){
    Serial.println("Error initialize myIMU!");
  }
}

void setup(){
  Serial.begin(9600);
  delay(1000);

  tft_setup();
  wifi_setup();

  //gyroscope setup
  pinMode(GYROINT_PIN,INPUT);
  IMU_congifuration();
  gyroCalibrate();

  //microphone setup
  pinMode(MIC_SENSOR, INPUT);
  upload_timer = millis() + uploadFreq;
  Serial.println();

  //Button setup & Sleep Mode setup
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  pinMode(BUTTON_PIN, INPUT);
  sound_timer = millis();

  print("Ready for sleep!");
  delay(2000);

  tft.fillScreen(TFT_BLACK);
  digitalWrite(38, LOW); // turn off the back light of lcd
}

void loop() {
  // Read Sound Data
  // Unsolved: The Microphone is not very Accurate
  if (digitalRead(MIC_SENSOR) == LOW){
    if (millis() > sound_timer){
      sound_val++;
      // has_sound = true;
      Serial.println("Sound detected");
      sound_timer = millis() + soundDetectFreq;
    }
  }
  else{
    has_sound = false;
  }

  if( digitalRead(GYROINT_PIN) == HIGH ){
    x = myIMU.readFloatGyroX() - xavg;
    y = myIMU.readFloatGyroY() - yavg;
    z = myIMU.readFloatGyroZ() - zavg;

    total = sqrt((x*x + y*y + z*z));
    // Serial.print("The total is ");Serial.println(total);

    if (total > threshold && !has_motion){
      motion_val++;
      has_motion = true;
      Serial.print("Motion detected when "); Serial.println(total);
    }
    else if (total < lower_bound_threshold && has_motion){
      has_motion = false;
    }
  }
  // PRINT RESULTS
  // Serial.print("Motion Frequency: "); Serial.println(motion_val);
  // Serial.print("Noise Frequency: "); Serial.println(sound_val);

  if (millis() > upload_timer){
    if (!DEBUG_MODE){
      int err = 0;
      WiFiClient c;
      HttpClient http(c);

      queryString = (String)"/?var2=" + (String)sound_val + "," + (String)motion_val;
      Serial.println(queryString);

      err = http.get(public_IP.c_str(), port, queryString.c_str());
    
      if (err == 0) {
        // Serial.println("\nstartedRequest ok");
        err = http.responseStatusCode();
        if (err >= 0) {
          Serial.print("Got status code: ");
          Serial.println(err);
        } else {
          Serial.print("Getting response failed: ");
          Serial.println(err);
        }
      } else {
        Serial.print("Connect failed: ");
        Serial.println(err);
      }
      http.stop();
      // End of Upload
    }
    Serial.println("*************One Upload*****************");
    Serial.print("Motion Frequency: "); Serial.println(motion_val);
    Serial.print("Noise Frequency: "); Serial.println(sound_val);

    sound_val=0;
    motion_val=0;
    // Restart timer
    upload_timer = millis() + uploadFreq;
  }
  
  // Determine whether to sleep
  // Todo: Use interrupt to enter DEEP SLEEP
  if (digitalRead(BUTTON_PIN) == HIGH){
    Serial.println("Button Pressed");
    Serial.println("Going to sleep");
    delay(1000); // Has to be long enough in case the user hasn't release the button for long and restart the program again
    esp_deep_sleep_start();
  }

  // Serial.println();
  //delay(samplingFreq);
}
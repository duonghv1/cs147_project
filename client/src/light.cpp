#include <Arduino.h>
#include <algorithm>
#include "light.h"

int upper_bound = -1;
int lower_bound = 4096;
int val;

double map(int read_val){
  // range(read_val) = (0, 4095)
  double range = double(upper_bound - lower_bound);
  double percent = (double)(read_val - lower_bound)/range;
  // percent range [0, 100]
  return percent*100;
}

void default_calibration(){
  upper_bound = 4095;
  lower_bound = 0;
}

void light_calibration(int LIGHT_SENSOR){
  // Part 1: Calibration (10s)
  Serial.println();
  Serial.print("Start Calibration.");
  int two_second = 1;
  
  while (two_second <= 10){
    val = analogRead(LIGHT_SENSOR);
    upper_bound = max(val, upper_bound);
    lower_bound = min(val, lower_bound);
    delay(500);

    val = analogRead(LIGHT_SENSOR);
    upper_bound = max(val, upper_bound);
    lower_bound = min(val, lower_bound);
    delay(500);
    two_second ++;
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Light max="); Serial.println(upper_bound);
  Serial.print("Light min="); Serial.println(lower_bound);
  Serial.println();
}
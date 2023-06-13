# CS147 Project - Anteater Sleep Awareness
Anteater Sleep Awareness is the project developed by Yiru Tang and Duong Vu for UC Irvine's CS147 course (IoT Software and Systems). This was a project based class where the goal was to develop a meaningful IoT device to solve real life problems.

The device collect sound, light, temperature, humidity data from users' sleeping environment and also monitor users' movement during sleep. It transfer data to the cloud via Wi-Fi and users can view real-time data with metrics on the online dashboard.

## Overview

#### Tasks completed

- [x] One ESP32 board collects temperature/humidity, light data from users' sleep environment.
- [x] One ESP32 board(wearable) collect sound and motion data from users.
- [x] On-off button and the sleep mode are implemented for each piece of device, respectively.
- [x] Web App hosted on AWS EC2 instance, and receives data from two boards and integrate them for the frontend.
- [x] Web App dashboards incorporate real-time charts to display different metrics and their running averages.
- [x] A summary of metrics’ averages at the top of the page.

#### Future Work

- [ ] Add a user login interface with Firebase.
- [ ] Store user data into database.
- [ ] Enable users to view their history of sleeping data.
- [ ] Include "sleep quality rating" feature with a calendar to check-in everyday.
- [ ] Implement algorithm to analyze users' sleeping data and give devices to improve their sleeping quality accordingly.

### Tech Stacks

<img src="https://github.com/duonghv1/cs147_project/blob/master/view/CS147FinalProject.drawio.png" width=500><br>

#### **Hardware - LILYGO-TTGO ESP32:**
With two ESP32 boards, we connect one to the light sensor and humidity/temperature sensor, which can be placed anywhere in the bedroom. The other board will be collecting data from the accelerometer and the microphone and will be wearable for users as they sleep. Sensors transmit data to ESP32 via the I2C serial protocol. Two boards will then send data separately to the backend via Wi-Fi in real time. 

#### **Back-end - [Flask Python](https://flask.palletsprojects.com/en/2.0.x/installation/):**
The Python Flask backend handles necessary endpoints for data push from sensors, combines sensor data, and passes data to the frontend. The backend caches and computes running averages of sleep metrics, which is used by the frontend to display. 

#### **Front-end - HTML/CSS:**
For our website hosted on EC2 instance, we will create features such as dashboards showing all data as real-time charts, a sleep rating screen, settings panels to change the ideal level of different metrics. When a user clicks on a metric, the app will show the graph as a pop-up window and the user can choose to download it as an image. In each chart, we are showing both the real time sensor data and the running average to display the trend of that metric based on different choices of time range. The user can choose a specific time point to view their sleeping environment data. 

#### **Cloud Service - AWS:**
We host the web application using Amazon Web Services, EC2 instance. It allows 25GB free storage which is enough for our project use. By doing this, the application can be accessed by a public IP address.


## Website Walkthough

<img src="https://github.com/duonghv1/cs147_project/blob/master/view/webAppDemo.gif" width=500><br>

## Final Thoughts

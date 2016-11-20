/**************************************************************
 * Blynk is a platform with iOS and Android apps to control
 * Arduino, Raspberry Pi and the likes over the Internet.
 * You can easily build graphic interfaces for all your
 * projects by simply dragging and dropping widgets.
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social networks:            http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 *
 * Blynk library is licensed under MIT license
 * This example code is in public domain.
 *
 **************************************************************
 * This example shows how to use ordinary Arduino Serial
 * to connect your project to Blynk.
 * Feel free to apply it to any other example. It's simple!
 *
 * 1. Optional, but recommended.
 *    Connect additional USB-serial adapter to see the prints.
 *
 * 2. Edit auth token and upload this sketch.
 *
 * 3. Run the script (script located in "scripts" folder of library root,
 *    e.g. 'blynk-library/scripts') for redirecting traffic to server:
 *
 *      for Windows:
 *                     1. Open cmd.exe
 *                     2. write : (your way to blynk-ser.bat folder) example: "cd C:\blynk-library-0.3.1\blynk-library-0.3.1\scripts"
 *                     3. write : "blynk-ser.bat -c COM4" (where COM4 is port with your Arduino)
 *                     4. And press "Enter" , press "Enter" and press "Enter"
 *
 *      for Linux and OSX:
 *
 *            cd /Users/Andrew/Documents/Arduino/libraries/Blynk/scripts        
 *            ./blynk-ser.sh (may need to run with sudo)
 *
 *    You can specify port, baud rate, and server endpoint like this:
 *      ./blynk-ser.sh -c <serial port> -b <baud rate> -s <server address> -p <server port>
 *
 *    For instance :
 *      ./blynk-ser.sh -c /dev/ttyACM0 -b 9600 -s cloud.blynk.cc -p 8442
 *
 *    Run blynk-ser.sh -h for more information
 *
 *    Be sure to select the right serial port (there may be multiple).
 *
 *    Attention!
 *        Arduino IDE may complain with "programmer is not responding".
 *        You need to terminate script before uploading new sketch.
 *
 * 4. Start blynking! :)
 *
 **************************************************************/

// You could use a spare Hardware Serial on boards that have it (like Mega)
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <MCP3008.h>
#include "alarm.h"
#include <Wire.h>


// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "d77d64bde2f04a39973277db7f734fa3";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "CarrollHome";
char pass[] = "Amazinglove1222";

// define pin connections
#define CS_PIN 0
#define CLOCK_PIN 14
#define MOSI_PIN 13
#define MISO_PIN 12

// put pins inside MCP3008 constructor
MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);

SimpleTimer timer;
WidgetLED led2(2);
WidgetLED led3(3);
int lightPrev = 0;
int motionPrev = 0;
int enbNotify = 0;

void setup()
{
  Blynk.begin(auth, ssid, pass);
  Wire.pins(4, 5);
  Wire.setClock(400000);
  Wire.begin();
  // Setup a function to be called every second
  timer.setInterval(1000L, getSample);
  
}

void getSample()
{
  int tempSense = adc.readADC(2);
  float tempVolt = (tempSense/1024.0)*3.3;
  float tempC = (tempVolt - 0.5) * 100;
  float tempF = tempC*(9.0/5.0) + 32;
  //Serial.println(tempSense);
  Blynk.virtualWrite(V1,tempF);
  delay(200);
  
  int lightSense = adc.readADC(3);
  float lightSenseC = 0-(lightSense-1023); // invert reading and scale from 0-100
  lightSenseC = lightSenseC/1023.0*100.0;
  int lightSenseInt = lightSenseC;
  float lightSenseFloat = lightSenseInt/100.0;
  Blynk.virtualWrite(V5,lightSenseFloat);
  Blynk.virtualWrite(V7,lightSenseInt);
  //Serial.println(lightSense);
  if (lightSenseInt > 54) {
    led2.on();
    if (lightPrev == 0) {
      if (enbNotify == 1) {
        Blynk.notify("Lights Turned On!");
      }
    }
    lightPrev = 1;
  } else {
    led2.off();
    if (lightPrev == 1) {
      if (enbNotify == 1) {
        Blynk.notify("Lights Turned Off!");
      }
    }
    lightPrev = 0;
  }
  delay(200);
  
  int motionSense = adc.readADC(1);
  //Serial.println(motionSense);
  if (motionSense > 500) {
    led3.on();
    Blynk.virtualWrite(V6,90);
    if (motionPrev == 0) {
      if (enbNotify == 1) {
        playAlarm();
        Blynk.notify("Motion Detected!");
      }
    }
    motionPrev = 1;
  } else {
    led3.off();
    Blynk.virtualWrite(V6,10);
    motionPrev = 0;
  }
  delay(200);
}

BLYNK_WRITE(V4)
{
  enbNotify = param.asInt();
}

void playAlarm()
{
  
  for (int i = 0; i < 1250; i++) {
    Wire.beginTransmission(0x62);
    for (int j = 0; j < 100; j++) {
      Wire.write(uint8_t(alarmSamps[j] >> 8));
      Wire.write(uint8_t(alarmSamps[j] & 0x00FF));
    }
    Wire.endTransmission();
  }
}

void loop()
{
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
}


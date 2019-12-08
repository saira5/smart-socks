/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "DHT.h"


#define DHTPIN 2
#define DHTTYPE DHT11
#define REPORTING_PERIOD_MS     1000

int outputpin= A0;

String HardwareID="A0900";
int i=0;
float hBeat=0;
float oxy=0;
float h=0;
float bodyTemp=0; 

DHT dht(DHTPIN, DHTTYPE);
  
// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}



void sendData(float hb,float oxygen,float Humidity,float childTemp){
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
 
   HTTPClient http;    //Declare object of class HTTPClient

 //String ADCData="data1";
 //String station="data2";
 //String dhtSensorValue=String(h);
  //Post Data

  Serial.println("AKJSDHAKSD:");
  Serial.println(hb);
   String postData = "hardware_id=" + String(HardwareID) + "&pulse_rate=" + String(hb) + "&blood_oxygen=" + String(oxygen)+ "&moisture=" + String(Humidity) + "&body_temperature=" + String(childTemp) ;
  
   http.begin("http://192.168.43.250:3000/home/uploadHardwareData");      //Specify request destination
   http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header
 
 //  int httpCode = http.POST(postData);   //Send the request
    http.POST(postData);   //Send the request
 //  String payload = http.getString();                  //Get the response payload
 
   //Serial.println(httpCode);   //Print HTTP return code
   //Serial.println(payload);    //Print request response payload
 
   http.end();  //Close connection
 
 }else{
 
    Serial.println("Error in WiFi connection");   
 
 }
 


}

void setup()
{
    Serial.begin(115200);

    Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
     dht.begin();
 WiFi.begin("AndroidA", "12345678");   //WiFi connection
  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
 
    delay(500);
    Serial.println("Waiting for connection");
 
  }


    
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

      
 

    // The default current for the IR LED is 50mA and it could be changed
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
    // Make sure to call update as fast as possible
    pox.update();
    float tempH=0;
    float tempBeat=0;
    float tempOxy=0;
    
    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        tempBeat=pox.getHeartRate();
        Serial.print(tempBeat);
        Serial.print("bpm / SpO2:");
        tempOxy=pox.getSpO2();
        Serial.print(tempOxy);
        Serial.println("%");


            int analogValue = analogRead(outputpin);
            float millivolts = (analogValue/1024.0) * 3300; //3300 is the voltage provided by NodeMCU
            float celsius = millivolts/10;
       //     Serial.print("in DegreeC=   ");
       //     Serial.println(celsius);
            
            //---------- Here is the calculation for Fahrenheit ----------//
            
            float fahrenheit = ((celsius * 9)/5 + 32);
            Serial.print("  Farenheit=   ");
            Serial.println(fahrenheit);

          
        tsLastReport = millis();
       if(isnan(tempH) || isnan(hBeat) || isnan(tempOxy) || tempOxy==0 || tempBeat==0 || fahrenheit<88 || fahrenheit>109){
      
          i=i-1;
              
        }else{
          
          tempH= dht.readHumidity();
          h=tempH+h;

          hBeat=tempBeat+hBeat;
          oxy=tempOxy+oxy;

          bodyTemp=bodyTemp+fahrenheit;
        
        }
       Serial.println(i);
       i=i+1;
   
    }

    

if(i>=30){

    Serial.print("TST VAL :");
    Serial.print(oxy);
  
    h=h/30;
    hBeat=hBeat/30;
    oxy=oxy/30;
    bodyTemp=bodyTemp/30;
    
    Serial.print("HUDMIDITY :");
    Serial.print(h);
    Serial.print("      HeartBEAT :");
    Serial.print(hBeat);
    Serial.print("      SpO2 :");
    Serial.println(oxy);
    Serial.print("      BODYTEMP :");
    Serial.print(bodyTemp);
    
  
  
  //Serial.println(String(hbeat)+" "+ String(oxy));
  sendData(hBeat,oxy,h,bodyTemp);
  h=0;
  oxy=0;
  hBeat=0;
  i=0;
  
  }
    
}


#include <TinyGPS++.h>

#include <Arduino_JSON.h>
#include "Arduino.h"

#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESP8266WebServer.h"
#include <SoftwareSerial.h>
#include <math.h>
#define LED D4 // Led in NodeMCU at pin GPIO16 (D0).

boolean simMode=true;

boolean isWithinRange = false;
boolean isOnDirection = false;
double distanceToStop = 10000.0, directionDiff;
double LonLatArr[4];
double receivedLon = 79.0, receivedLat = 6.000, receivedDirection = 0.0;
//double gpsLon = 79.94522849529328, gpsLat = 6.8186241151775535, gpsSpeed = 3.6, gpsDirection = 10.000;
double gpsLon = 0, gpsLat = 0, gpsSpeed = 0, gpsDirection = 0;

boolean stopFlag = false;
bool receivedStatus = false;
const char *ssid = "VWS21";         // replace with your SSID
const char *password = "Vikum31415"; // and Password
ESP8266WebServer server(80);

TinyGPSPlus gps;
//d1
SoftwareSerial SoftSerial(5, 4);
void setup()
{
  Serial.begin(9600);
  SoftSerial.begin(9600);
  Serial.println(WiFi.macAddress());

  WiFi.begin(ssid, password); //Connect to the WiFi network

  while (WiFi.status() != WL_CONNECTED)
  { //Wait for connection

    delay(500);
    Serial.println(WiFi.macAddress());

    Serial.println("Waiting to connect...");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //Print the local IP

  server.on("/", HTTP_POST, handleBody); //Associate the handler function to the path
  server.on("/gps", HTTP_POST, handleBodyPut); //Associate the handler function to the path

  server.begin(); //Start the server
  Serial.println("Server listening");
  pinMode(LED, OUTPUT); // set the digital pin as output.

}

void loop()
{

  server.handleClient(); //Handling of incoming requests

  if (receivedStatus)
  {
    if (!simMode) {
      while (SoftSerial.available() > 0)
      {
        if (gps.encode(SoftSerial.read()))
        {
          if (gps.speed.isValid())
          {
            gpsSpeed = gps.speed.kmph();
          }
          if (gps.course.isValid())
          {
            gpsDirection = gps.course.deg();

          }
          if (gps.location.isValid())
          {
            gpsLon = gps.location.lng();
            gpsLat = gps.location.lat();
            //          Serial.println(gpsLon,7);
            //          Serial.println(gpsLat,7);

          }
        }
      }
    }
    distanceToStop = TinyGPSPlus::distanceBetween(
                       gpsLat,
                       gpsLon,
                       receivedLat,
                       receivedLon);
    if (!within10Meters())
    {
      withinDistance();
      withinRadius();

    }
    if (isWithinRange && isOnDirection)
    {
      stopFlag = true;
      Serial.println("stop");
      digitalWrite(LED, HIGH);// turn the LED off.(Note that LOW is the voltage level but actually
      //the LED is on; this is because it is acive low on the ESP8266.
      delay(500);          // wait for 1 second.
      digitalWrite(LED, LOW); // turn the LED on.
      delay(500);         // wait for 1 second.
    }
    else {
      stopFlag = false;
    }

  }

  else
  {
    isWithinRange = false;
    isOnDirection = false;
  }
}
boolean within10Meters () {
  if (distanceToStop < 10.0) {
    Serial.println("within 10 meters");
    isWithinRange = true;
    isOnDirection = true;
    return true;
  }
  return false;
}
boolean withinRadius() {
  directionDiff = fabs(receivedDirection - gpsDirection);
  //30 degree search radius was set arbitraly since there are no standards for road directions)
  if (directionDiff < 15.0 || (directionDiff < 195.0 && directionDiff > 165.0) || directionDiff > 345.0)
  {
    Serial.println("within radius");

    isOnDirection = true;
    return true;
  }
  isOnDirection = false;
  return false;
}
boolean withinDistance() {
  if (distanceToStop < 150.0)
  {
    Serial.println("within distance");
    Serial.println("direction");
    Serial.print(gpsDirection, 7);
    isWithinRange = true;
    return true;
  }
  isWithinRange = false;
  return false;
}
void handleBody()
{ // If a POST request is made to URI /login
  String postBody = server.arg("plain");
  JSONVar myObject = JSON.parse(postBody);
  if (JSON.typeof(myObject) == "undefined")
  {
    Serial.println("Parsing input failed!");
  }
  JSONVar keys = myObject.keys();

  for (int i = 0; i < keys.length(); i++)
  {
    JSONVar value = myObject[keys[i]];
    LonLatArr[i] = double(value);
  }
  receivedStatus = LonLatArr[0] == 1 ? true : false;
  if (receivedStatus)
  {
    receivedLon = LonLatArr[1];
    receivedLat = LonLatArr[2];
    receivedDirection = LonLatArr[3];
  }
  else
  {
    receivedLon = 0.0;
    receivedLat = 0.0;
    receivedDirection = 0.0;
  }

  Serial.println("received Lat ");
  Serial.println(receivedLat, 7);
  Serial.println("received Lon ");
  Serial.println(receivedLon, 7);
  Serial.println("received status ");
  Serial.println(receivedStatus);
  Serial.println("received direction ");
  Serial.println(receivedDirection, 7);

  Serial.println("gps Lat ");
  Serial.println(gpsLat, 7);
  Serial.println("gps Lon ");
  Serial.println(gpsLon, 7);
  Serial.println("gps status ");
  Serial.println(gpsDirection, 7);

  // if (server.hasArg("plain") == false)
  // { //Check if body received

  //   server.send(200, "text/plain", "Body not received");
  //   return;
  // }
  String message = "Body received:\n";
  message += server.arg("plain");
  message += "\n";

  server.send(200, "text/plain", message);
  Serial.println(message);
}
void handleBodyPut()
{ // If a POST request is made to URI /login
  String postBody = server.arg("plain");
  JSONVar myObject = JSON.parse(postBody);
  if (JSON.typeof(myObject) == "undefined")
  {
    Serial.println("Parsing input failed!");
  }
  JSONVar keys = myObject.keys();

  for (int i = 0; i < keys.length(); i++)
  {
    JSONVar value = myObject[keys[i]];
    LonLatArr[i] = double(value);
  }


  gpsLon = LonLatArr[0];
  gpsLat = LonLatArr[1];
  gpsDirection = LonLatArr[2];


  Serial.println("gps Lat ");
  Serial.println(gpsLat, 7);
  Serial.println("gps Lon ");
  Serial.println(gpsLon, 7);
  Serial.println("gps direction ");
  Serial.println(gpsDirection, 7);

  // if (server.hasArg("plain") == false)
  // { //Check if body received

  //   server.send(200, "text/plain", "Body not received");
  //   return;
  // }
  String message = "GPS received:\n";
  message += server.arg("plain");
  message += "\n";

  server.send(200, "text/plain", message);
  Serial.println(message);
}

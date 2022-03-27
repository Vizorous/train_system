
#include <TinyGPS++.h>

#include <Arduino_JSON.h>
#include "Arduino.h"

#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <math.h>

boolean isWithinRange = false;
boolean isOnDirection = false;
double distanceToStop = 10000.0, directionDiff;
double LonLatArr[4];
double recievedLon = 79.0, recievedLat = 6.000, recievedDirection = 0.0;
double gpsLon = 79.94522849529328, gpsLat = 6.8186241151775535, gpsSpeed = 3.6, gpsDirection = 10.000;

bool recievedStatus = false;
const char *ssid = "VWS21";         // replace with your SSID
const char *password = "Vikum31415"; // and Password
ESP8266WebServer server(80);

TinyGPSPlus gps;
//d2 
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

  server.begin(); //Start the server
  Serial.println("Server listening");
}

void loop()
{

  server.handleClient(); //Handling of incoming requests

  if (recievedStatus)
  {
    distanceToStop = TinyGPSPlus::distanceBetween(
                       gpsLat,
                       gpsLon,
                       recievedLat,
                       recievedLon);
    if (distanceToStop < 10.0)
    {
      isWithinRange = true;
      isOnDirection = true;
    }
    else{
      
    directionDiff = fabs(recievedDirection - gpsDirection);
    if (directionDiff < 15.0 || (directionDiff < 195.0 && directionDiff > 165.0) || directionDiff > 345.0)
    {
      isOnDirection = false;
    }
    else
    {
      isOnDirection = true;
    }

    if (distanceToStop < 150.0)
    {
      isWithinRange = true;
    }
    else
    {

      isWithinRange = false;
    }

    }
    if (isWithinRange && isOnDirection)
    {
      Serial.println("stop");
    }
  }

  else
  {
    isWithinRange = false;
    isOnDirection = false;
  }
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
  recievedStatus = LonLatArr[0] == 1 ? true : false;
  if (recievedStatus)
  {

    recievedLon = LonLatArr[1];
    recievedLat = LonLatArr[2];
    recievedDirection = LonLatArr[3];
  }
  else
  {
    recievedLon = 0.0;
    recievedLat = 0.0;
    recievedDirection = 0.0;
  }

  Serial.println("recieved Lat ");
  Serial.println(recievedLon, 7);
  Serial.println("recieved Lon ");
  Serial.println(recievedLon, 7);
  Serial.println("recieved status ");
  Serial.println(recievedStatus);
  Serial.println("recieved direction ");
  Serial.println(recievedDirection, 7);

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

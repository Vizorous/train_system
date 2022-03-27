#include <Arduino_JSON.h>

#include <TinyGPS++.h>
#include <TimeLib.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

// #define time_offset 19800 // define a clock offset of 19800 seconds (5.5 hours) ==> UTC + 5.30

//wifi details
const char *serverName = "http://192.168.1.8:3000/train";
const char *ssid = "VW SLT";
const char *password = "Vikum31415";

//range detecter bool
boolean isWithinRange = false;

//route id init
int routeId = 1000, stopId = 1000, nextRouteId, nextStopId;

//waypoint init

double waypointLat = 6.8143542, waypointLon = 79.9712693, nextWaypointLat, nextWaypointLon;

// date and time
String gpsTime, gpsDate;

//gps values
double gpsLon, gpsLat, gpsSpeed, gpsDirection, distanceToWaypointM;

//server operating in http
ESP8266WebServer postServer(3010);

//tinygps init
TinyGPSPlus gps;

// SoftSerial (RX pin, TX pin)
SoftwareSerial SoftSerial(5, 4);

void setup()
{
  Serial.begin(9600);             // initialise serial communication at 9600 bps
  SoftSerial.begin(9600);         // initialize software serial at 9600 bps
  Serial.println();               // print empty line
  Serial.print("Connecting to "); // print text in Serial Monitor
  Serial.println(ssid);           // print text in Serial Monitor
  WiFi.begin(ssid, password);     // connect to Wi-Fi network with SSID and password
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("Server started");
  Serial.println(WiFi.localIP());

  // server.begin();
  postServer.on("/", HTTP_POST, handleRequest);
  postServer.on("/reset", HTTP_POST, handleReset);
  postServer.begin();
}

void loop()
{

  postServer.handleClient();
  while (SoftSerial.available() > 0)
  {
        char val=SoftSerial.read();
        Serial.print(val);
    if (gps.encode(SoftSerial.read()))
    {

      if (gps.location.isValid())
      {
        gpsLon = gps.location.lng();
        gpsLat = gps.location.lat();
        //        Serial.println(gpsLon, 7);
        //        Serial.println(gpsLat, 7);

        distanceToWaypointM =
          TinyGPSPlus::distanceBetween(
            gpsLat,
            gpsLon,
            waypointLat,
            waypointLon);

        if (distanceToWaypointM < (double)1500.0 && !isWithinRange)
        {
          Serial.println(gpsLon, 7);
          Serial.println(gpsLat, 7);
          Serial.println(waypointLon, 7);
          Serial.println(waypointLat, 7);
          Serial.println("inside the 1.5km radius");
          WiFiClient wifiClient;
          HTTPClient http;

          http.begin(wifiClient, serverName);
          http.addHeader("Content-Type", "application/json");
          String postString = String(String("{\"route_id\":") + routeId + ",\"stop_id\":" + stopId + ",\"activation_type\":\"train\",\"status\":" + 1 + "}");
          int httpResponseCode = http.POST(postString);
          String serverResponse = http.getString();
          http.end();
          if (httpResponseCode != 200)
          {
            Serial.println("Error Occured!");
            delay(10000);

            return;
          }
          isWithinRange = true;

          Serial.println("HTTP Response code: ");
          Serial.print(httpResponseCode);

          JSONVar respJson = JSON.parse(serverResponse);

          if (JSON.typeof(respJson) == "undefined")
          {
            Serial.println("Parsing input failed!");
            delay(10000);

            return;
          }

          // Serial.print("JSON object = ");
          // Serial.println(respJson);

          JSONVar keys = respJson.keys();

          nextWaypointLon = double(respJson[keys[0]]);
          nextWaypointLat = double(respJson[keys[1]]);
          nextRouteId = int(respJson[keys[2]]);
          nextStopId = int(respJson[keys[3]]);

          Serial.print("next Waypoint Lon: ");
          Serial.println(nextWaypointLon, 7);
          Serial.print("next Waypoint Lat: ");
          Serial.println(nextWaypointLat, 7);
          Serial.print("next Route Id: ");
          Serial.println(nextRouteId);
          Serial.print("next Stop Id: ");
          Serial.println(nextStopId);
        }
        if (distanceToWaypointM > (double)1500.0 && isWithinRange)
        {
          Serial.println("outside the 1.5km range with isWithinRange set to false");
          WiFiClient wifiClient;
          HTTPClient http;

          http.begin(wifiClient, serverName);

          http.addHeader("Content-Type", "application/json");
          String postString = String(String("{\"route_id\":") + routeId + ",\"stop_id\":" + stopId + ",\"activation_type\":\"train\",\"status\":" + 0 + "}");
          int httpResponseCode = http.POST(postString);
          String serverResponse = http.getString();
          http.end();
          if (httpResponseCode != 200)
          {
            Serial.println("Error Occured!");
            delay(10000);

            return;
          }
          isWithinRange = false;

          routeId = nextRouteId;
          stopId = nextStopId;
          waypointLat = nextWaypointLat;
          waypointLon = nextWaypointLon;


          Serial.print("next Waypoint Lon: ");
          Serial.println(waypointLon, 7);
          Serial.print("next Waypoint Lat: ");
          Serial.println(waypointLat, 7);
          Serial.print("next Route Id: ");
          Serial.println(routeId);
          Serial.print("next Stop Id: ");
          Serial.println(stopId);
          Serial.println("HTTP Response code: ");
          Serial.print(httpResponseCode);
        }
      }

    }
  }
}

void handleReset() {
  waypointLat = 6.8143542, waypointLon = 79.9712693;
  routeId = 1000, stopId = 1000;
  isWithinRange = false;
  Serial.println("reset");
  postServer.send(200, "text/plain", "reset");

  //  ESP.restart();
  return;

}




void handleRequest()
{
  String body = postServer.arg("plain");
  JSONVar reqJson = JSON.parse(body);
  if (JSON.typeof(reqJson) == "undefined")
  {
    postServer.send(500, "text/plain", "Error Occured!");
    Serial.println("Parsing input failed!");
    return;
  }
  JSONVar keys = reqJson.keys();
  double adjustWaypointArr[2];

  for (int i = 0; i < keys.length(); i++)
  {
    JSONVar value = reqJson[keys[i]];
    adjustWaypointArr[i] = double(value);
  }

  waypointLon = adjustWaypointArr[0];
  waypointLat = adjustWaypointArr[1];
  String message = "recieved";
  message += postServer.arg("plain");
  message += "\n";

  postServer.send(200, "text/plain", message);
  Serial.println(message);
}

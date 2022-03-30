#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const int buttonPin = D2;     
const char* ssid = "VWS21";
const char* password = "Vikum31415";
const char* serverName = "http://34.93.185.72/node/train";

int routeId =1000;
int stopId =1000;

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

int buttonState = 0;

void setup() {
  
  pinMode(buttonPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT); 
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
}

void loop() {

  buttonState = digitalRead(buttonPin);
  Serial.print(buttonState);
  if (buttonState == HIGH) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }

  if ((millis() - lastTime) > timerDelay) {
 
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");
      String postString = String(String("{\"route_id\":") + routeId + ",\"stop_id\":" + stopId + ",\"activation_type\":\"relay\",\"status\":" + buttonState + "}");
      int httpResponseCode = http.POST(postString);
      Serial.print(postString);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

}

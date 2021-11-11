#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <aREST.h>
#include "config.h"

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

const int teaSwitchPin = 14;
const int meetingSwitchPin = 12;
bool teaSwitchLatch = false;

WiFiServer server(LISTEN_PORT);
aREST rest = aREST();

// Variables to be exposed to the API
int inMeeting = 0;

void setup() {
  Serial.begin(115200);

  connectToWiFi();
  restHandlerSetup();

  pinMode(teaSwitchPin, INPUT);
  pinMode(meetingSwitchPin, INPUT);
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void restHandlerSetup() {
  rest.variable("inMeeting", &inMeeting);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("000001");
  rest.set_name("esp8266");

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  handleRestCalls();
  checkMeetingSwitch();
  checkTeaSwitch();
}

void handleRestCalls() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (!client.available()) {
    delay(1);
  }
  rest.handle(client);
}

void checkMeetingSwitch() {
  inMeeting = digitalRead(meetingSwitchPin);
}

void checkTeaSwitch() {
  if (HIGH == digitalRead(teaSwitchPin)) {
    if (teaSwitchLatch) {
      callVoiceMonkey("testing-monkey");
      teaSwitchLatch = false;
    }
  } else {
    teaSwitchLatch = true;
  }
}

void callVoiceMonkey(String monkeyId) {
  Serial.println("callAlexaRoutine");
  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, "http://api.voicemonkey.io/trigger?access_token=" + String(ACCESS_TOKEN) + "&secret_token=" + String(SECRET_TOKEN) + "&monkey=" + monkeyId);

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }

}

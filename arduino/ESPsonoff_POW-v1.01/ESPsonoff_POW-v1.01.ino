/*

  Copyright (c) 2017 @KmanOz
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ==============================================================================
  Changes in v1.01
  
    - Relay state now stored in EEPROM and will power up with last relay state
  ==============================================================================

  **** USE THIS Firmware for: Sonof POW ****

*/

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include "power.h"

#define BUTTON          0                                    // (Don't Change for Sonoff POW)
#define RELAY           12                                   // (Don't Change for Sonoff POW)
#define LED             15                                   // (Don't Change for Sonoff POW)
 
#define MQTT_CLIENT     "Sonoff_Living_Room_v1.0pow"         // mqtt client_id (Must be unique for each Sonoff)
#define MQTT_SERVER     "192.168.0.100"                      // mqtt server
#define MQTT_PORT       1883                                 // mqtt port
#define MQTT_TOPIC      "home/sonoff/living_room/1"          // mqtt topic (Must be unique for each Sonoff)
#define MQTT_USER       "user"                               // mqtt user
#define MQTT_PASS       "pass"                               // mqtt password

#define WIFI_SSID       "homewifi"                           // wifi ssid
#define WIFI_PASS       "homepass"                           // wifi password

#define VERSION    "\n\n----------------  Sonoff POW Powerpoint v1.01  -----------------"

bool debug = false;                                          // If 'true' enables serial output at console for debug messages
bool rememberRelayState = true;                              // If 'true' remembers the state of the relay before power loss.
bool requestRestart = false;                                 // (Do not Change)
bool sendStatus = false;                                     // (Do not Change)

char message_buff[60];                                       // (Do not Change)

int kUpdFreq = 1;                                            // Update frequency in Mintes to check mqtt connection
int kPwrUpdFreq = 15;                                        // Update frequency in Seconds to publish power usage
int kRetries = 10;                                           // WiFi retry count. Increase if not connecting to router.
int lastRelayState;                                          // (Do not Change)

unsigned long TTasks1;                                       // (Do not Change)
unsigned long TTasks2;                                       // (Do not Change)
unsigned long count = 0;                                     // (Do not Change)

extern "C" { 
  #include "user_interface.h" 
}

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient, MQTT_SERVER, MQTT_PORT);
ESP8266PowerClass power_read;
Ticker btn_timer;

void callback(const MQTT::Publish& pub) {
  if (pub.payload_string() == "on") {
    digitalWrite(RELAY, HIGH);
  }
  else if (pub.payload_string() == "off") {
    digitalWrite(RELAY, LOW);
  }
  else if (pub.payload_string() == "reset") {
    requestRestart = true;
  }
  sendStatus = true;
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUTTON, INPUT);
  digitalWrite(LED, LOW);
  digitalWrite(RELAY, LOW);
  power_read.enableMeasurePower();
  power_read.selectMeasureCurrentOrVoltage(VOLTAGE);
  power_read.startMeasure();
  EEPROM.begin(8);
  lastRelayState = EEPROM.read(0);
  if (rememberRelayState && lastRelayState == 1) {
     digitalWrite(RELAY, HIGH);
  }
  btn_timer.attach(0.05, button);
  mqttClient.set_callback(callback);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  delay(500);
  if (debug) {
   Serial.begin(115200);
   blinkLED(LED, 25, 120); 
   Serial.println(VERSION);
   Serial.print("\nESP ChipID: ");
   Serial.print(ESP.getChipId(), HEX);
   Serial.print("\nConnecting to "); Serial.print(WIFI_SSID); Serial.print(" Wifi"); 
  }
  while ((WiFi.status() != WL_CONNECTED) && kRetries --) {
    delay(500);
    if (debug) {
      Serial.print(" .");
    }
  }
  if (WiFi.status() == WL_CONNECTED) {  
    if (debug) {
      Serial.println(" DONE");
      Serial.print("IP Address is: "); Serial.println(WiFi.localIP());
      Serial.print("Connecting to ");Serial.print(MQTT_SERVER);Serial.print(" Broker . .");
    }
    delay(500);
    while (!mqttClient.connect(MQTT::Connect(MQTT_CLIENT).set_keepalive(120).set_auth(MQTT_USER, MQTT_PASS)) && kRetries --) {
      if (debug) {
        Serial.print(" .");
      }
      delay(1000);
    }
    if(mqttClient.connected()) {
      if (debug) {
        Serial.println(" DONE");
        Serial.println("\n----------------------------  Logs  -----------------------------");
        Serial.println();
      }
      mqttClient.subscribe(MQTT_TOPIC);
      blinkLED(LED, 40, 8);
      digitalWrite(LED, HIGH);
    }
    else {
      if (debug) {
        Serial.println(" FAILED!");
        Serial.println("\n----------------------------------------------------------------");
        Serial.println();
      }
    }
  }
  else {
    if (debug) {
      Serial.println(" WiFi FAILED!");
      Serial.println("\n----------------------------------------------------------------");
      Serial.println();
    }
  }
}

void loop() { 
  mqttClient.loop();
  yield();
  timedTasks1();
  yield();
  timedTasks2();
  yield();
  checkStatus();
  yield();
}

void blinkLED(int pin, int duration, int n) {             
  for(int i=0; i<n; i++)  {  
    digitalWrite(pin, HIGH);        
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
  }
}

void button() {
  if (!digitalRead(BUTTON)) {
    count++;
  } 
  else {
    if (count > 1 && count <= 40) {   
      digitalWrite(RELAY, !digitalRead(RELAY));
      sendStatus = true;
    } 
    else if (count >40){
      if (debug) {
        Serial.println("\n\nSonoff Rebooting . . . . . . . . Please Wait"); 
      }
      requestRestart = true;
    } 
    count=0;
  }
}

void checkConnection() {
  if (WiFi.status() == WL_CONNECTED)  {
    if (mqttClient.connected()) {
      if (debug) {
        Serial.println("mqtt broker connection . . . . . . . . . . . . . . . . . . . OK");
      }
    } 
    else {
      if (debug) {
        Serial.println("mqtt broker connection . . . . . . . . . . . . . . . . . . . LOST");
      }
      requestRestart = true;
    }
  }
  else { 
    if (debug) {
      Serial.println("WiFi connection . . . . . . . . . . . . . . . . . . . LOST");
    }
    requestRestart = true;
  }
}

void checkStatus() {
  if (sendStatus) {
    if(digitalRead(RELAY) == LOW)  {
      if (rememberRelayState) {
        EEPROM.write(0, 0);
      }
      mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/stat", "off").set_retain().set_qos(1));
      if (debug) {
        Serial.println("Relay . . . . . . . . . . . . . . . . . . . . . . . . . . . . OFF");
      }
    } else {
      if (rememberRelayState) {
        EEPROM.write(0, 1);
      }
      mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/stat", "on").set_retain().set_qos(1));
      if (debug) {
        Serial.println("Relay . . . . . . . . . . . . . . . . . . . . . . . . . . . . ON");
      }
    }
    if (rememberRelayState) {
      EEPROM.commit();
    }
    sendStatus = false;
  }
  if (requestRestart) {
    blinkLED(LED, 400, 4);
    ESP.restart();
  }
}

void getPower() {
  double power = power_read.getPower();
  yield();
  double voltage = power_read.getVoltage();
  yield();
  if (debug) {
    Serial.print("Reading Power . . . . . . . . . . . . . . . . . . . . . . . . ");
  }
  if (isnan(power_read.getPower() || power_read.getVoltage())) {
    mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/debug","\"Power Read Error\"").set_retain().set_qos(1));
    if (debug) {
      Serial.println("ERROR");
    }
    return;
  }
  String pubString = "{\"Power\": "+String(power)+", "+"\"Voltage\": "+String(voltage) + "}";
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/power", message_buff).set_retain().set_qos(1));
}

void timedTasks1() {
  if ((millis() > TTasks1 + (kUpdFreq*60000)) || (millis() < TTasks1)) { 
    TTasks1 = millis();
    checkConnection();
  }
}

void timedTasks2() {
  if ((millis() > TTasks2 + (kPwrUpdFreq*1000)) || (millis() < TTasks2)) { 
    TTasks2 = millis();
    getPower();
  }
}

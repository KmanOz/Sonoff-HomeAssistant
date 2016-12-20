#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define BUTTON          0                                    // (Don't Change for Sonoff)
#define RELAY           12                                   // (Don't Change for Sonoff)
#define LED             13                                   // (Don't Change for Sonoff)
#define DHTPIN          14                                   // (Don't Change for Sonoff)

#define DHTTYPE         DHT22                                // (Don't Change for Sonoff TH10/16)

#define MQTT_CLIENT     "Sonoff_Living_Room_v2.0t"           // mqtt client_id (Must be unique for each Sonoff)
#define MQTT_SERVER     "192.168.0.100"                      // mqtt server
#define MQTT_PORT       1883                                 // mqtt port
#define MQTT_TOPIC      "sonoff"                              // mqtt topic (Must be unique for each Sonoff)
#define MQTT_USER       "user"                               // mqtt user
#define MQTT_PASS       "pass"                               // mqtt password

#define WIFI_SSID       "homewifi"                           // wifi ssid
#define WIFI_PASS       "homepass"                           // wifi password

#define VERSION    "\n\n-----------------  Sonoff TH Powerpoint v1.0t+ -----------------"

DHT dht(DHTPIN, DHTTYPE, 11);                                // (Don't Change for Sonoff)

extern "C" { 
  #include "user_interface.h" 
}

bool sendStatus = false;
bool requestRestart = false;
bool tempReport = false;

int kUpdFreq = 1;
int kRetries = 30;
int mTimer = 0;

String str;

unsigned long TTasks;
unsigned long count = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient, MQTT_SERVER, MQTT_PORT);
Ticker btn_timer;

void callback(const MQTT::Publish& pub) {
  //lcd.print("Temp:"+payload.substring(payload.indexOf('temp', 135)+3,payload.indexOf('pres',150)-5)+" C");
  Serial.println("MQTT Payload: " + pub.payload_string());
  if (pub.payload_string() == "stat") {
  }
  else if (pub.payload_string() == "on") {
    digitalWrite(RELAY, HIGH);
    mTimer = 0;
    sendStatus = true;
  }
  else if (pub.payload_string() == "off") {
    digitalWrite(RELAY, LOW);
    mTimer = 0;
    sendStatus = true;
  }
  else if (pub.payload_string() == "reset") {
    requestRestart = true;
  }
  else if (pub.payload_string() == "temp") {
    tempReport = true;
  }
  else { 
    str = pub.payload_string();
    int i = atoi(str.c_str());
    if ((i > 0) && (i < 999)) {
      mTimer = i;
      digitalWrite(RELAY, HIGH);
      sendStatus = true;
    }
  }
  sendStatus = true;
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUTTON, INPUT);

  digitalWrite(LED, HIGH);
  digitalWrite(RELAY, LOW);
  mTimer = 0;
  
  btn_timer.attach(0.05, button);
  
  mqttClient.set_callback(callback);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.begin(115200);
  Serial.println(VERSION);
  Serial.print("\nESP ChipID: ");
  Serial.print(ESP.getChipId(), HEX);
  Serial.print("\nConnecting to "); Serial.print(WIFI_SSID); Serial.print(" Wifi"); 
  while ((WiFi.status() != WL_CONNECTED) && kRetries --) {
    delay(500);
    Serial.print(" .");
  }
  if (WiFi.status() == WL_CONNECTED) {  
    Serial.println(" DONE");
    Serial.print("IP Address is: "); Serial.println(WiFi.localIP());
    Serial.print("Connecting to ");Serial.print(MQTT_SERVER);Serial.print(" Broker . .");
    delay(500);
    while (!mqttClient.connect(MQTT::Connect(MQTT_CLIENT).set_keepalive(90).set_auth(MQTT_USER, MQTT_PASS)) && kRetries --) {
      Serial.print(" .");
      delay(1000);
    }
    if(mqttClient.connected()) {
      Serial.println(" DONE");
      Serial.println("\n----------------------------  Logs  ----------------------------");
      Serial.println();
      mqttClient.subscribe(MQTT_TOPIC);
      blinkLED(LED, 40, 8);
      digitalWrite(LED, LOW);
    }
    else {
      Serial.println(" FAILED!");
      Serial.println("\n----------------------------------------------------------------");
      Serial.println();
    }
  }
  else {
    Serial.println(" WiFi FAILED!");
    Serial.println("\n----------------------------------------------------------------");
    Serial.println();
  }
  getTemp();
}

void loop() { 
  mqttClient.loop();
  timedTasks();
  checkStatus();
  if (tempReport) {
    getTemp();
  }
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
      mTimer = 0;
      sendStatus = true;
    } 
    else if (count >40){
      Serial.println("\n\nSonoff Rebooting . . . . . . . . Please Wait"); 
      requestRestart = true;
    } 
    count=0;
  }
}

void checkConnection() {
  if (WiFi.status() == WL_CONNECTED)  {
    if (mqttClient.connected()) {
      Serial.println("mqtt broker connection . . . . . . . . . . OK");
    } 
    else {
      Serial.println("mqtt broker connection . . . . . . . . . . LOST");
      requestRestart = true;
    }
  }
  else { 
    Serial.println("WiFi connection . . . . . . . . . . LOST");
    requestRestart = true;
  }
}

void checkStatus() {
  if (sendStatus) {
    if(digitalRead(RELAY) == LOW)  {
      mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/stat", "off").set_retain().set_qos(1));
      Serial.println("Relay . . . . . . . . . . . . . . . . . . OFF");
    } else {
      mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/stat", "on").set_retain().set_qos(1));
      Serial.println("Relay . . . . . . . . . . . . . . . . . . ON");
    }
    mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/timer", String(mTimer)).set_retain().set_qos(1));
    sendStatus = false;
  }
  if (requestRestart) {
    blinkLED(LED, 400, 4);
    ESP.restart();
  }
}

void getTemp() {
  Serial.print("DHT read . . . . . . . . . . . . . . . . . ");
  float dhtH, dhtT;
  char message_buff[60];
  dhtH = dht.readHumidity();
  dhtT = dht.readTemperature();
  if(digitalRead(LED) == LOW)  {
    blinkLED(LED, 100, 1);
  } else {
    blinkLED(LED, 100, 1);
    digitalWrite(LED, HIGH);
  }
  if (isnan(dhtH) || isnan(dhtT)) {
    mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/debug","\"DHT Read Error\"").set_retain().set_qos(1));
    Serial.println("ERROR");
    tempReport = false;
    return;
  }
  String pubString = "{\"Temp\": "+String(dhtT)+", "+"\"Humidity\": "+String(dhtH) + "}";
  pubString.toCharArray(message_buff, pubString.length()+1);
  mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/temp", message_buff).set_retain().set_qos(1));
  Serial.println("OK");
  tempReport = false;
}

void timedTasks() {
  if ((millis() > TTasks + (kUpdFreq*60000)) || (millis() < TTasks)) { 
    TTasks = millis();
    checkConnection();
    tempReport = true;

    if (mTimer > 0) {
      mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/timer", String(mTimer)).set_retain().set_qos(1));
      Serial.println("TIMER " + String(mTimer));
      
      if (mTimer == 1) {
          digitalWrite(RELAY, LOW);
          sendStatus = true;
        }
      mTimer--;
    }
  }
}

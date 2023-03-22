/*
  MQTT Client  Light sensor sender/receiver
  This sketch demonstrates an MQTT client that connects to a broker, subscribes to a topic,
  and both listens for messages on that topic and sends messages to it, a random number between 0 and 255.
  When the client receives a message, it parses it, and PWMs the built-in LED.
  Uses a TCS34725 light sensor to read lux and color temperature
  This sketch uses https://public.cloud.shiftr.io as the MQTT broker, but others will work as well.
  See https://tigoe.github.io/mqtt-examples/#broker-client-settings for connection details. 
Libraries used:
  * http://librarymanager/All#WiFiNINA or
  * http://librarymanager/All#WiFi101 
  * http://librarymanager/All#ArduinoMqttClient
  * http://librarymanager/All#Adafruit_TCS34725 (for the sensor)
  the arduino_secrets.h file:
  #define SECRET_SSID ""    // network name
  #define SECRET_PASS ""    // network password
  #define SECRET_MQTT_USER "public" // broker username
  #define SECRET_MQTT_PASS "public" // broker password
  created 11 June 2020
  updated 4 Jan 2023
  by Tom Igoe
*/

#include <SPI.h>
#include <WiFiNINA.h>  // use this for Nano 33 IoT, MKR1010, Uno WiFi
#include <ArduinoMqttClient.h>
#include <Wire.h>
#include "arduino_secrets.h"

// initialize WiFi connection as SSL:
WiFiSSLClient wifi;
MqttClient mqttClient(wifi);

// details for MQTT client:
// char broker[] = "test.mosquitto.org";
char broker[] = "public.cloud.shiftr.io";
// char broker[] = "ws://10.23.10.135";
int port = 8883;
char topic[] = "conndev/apoorva";
String clientID = "indoor-navigation";

// // last time the client sent a message, in ms:
// long lastTimeSent = 0;
// // message sending interval:
// int interval = 6 * 1000;

// last RSSi or BSSID recorded:
// int lastRSSI = 0;
String lastBSSID = "";

String macAddr;

void setup() {
  // initialize serial:
  Serial.begin(9600);
  // wait for serial monitor to open:
  if (!Serial) delay(3000);

  pinMode(LED_BUILTIN, OUTPUT);
  // get MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  // put it in a string:
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) macAddr += "0";
    macAddr += String(mac[i], HEX);
  }
  Serial.println();

  // set the credentials for the MQTT client:
  mqttClient.setId(clientID);
  // if needed, login to the broker with a username and password:
  mqttClient.setUsernamePassword(SECRET_MQTT_USER, SECRET_MQTT_PASS);
}

void loop() {
  //if you disconnected from the network, reconnect:
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    connectToNetwork();
    // skip the rest of the loop until you are connected:
    return;
  }

  digitalWrite(LED_BUILTIN, HIGH);
  // if not connected to the broker, try to connect:
  if (!mqttClient.connected()) {
    Serial.println("attempting to connect to broker");
    connectToBroker();
  }
  // poll for new messages from the broker:
  mqttClient.poll();

  // int signalStregth = WiFi.RSSI();
  String router = "";
  byte bssid[6];
  WiFi.BSSID(bssid);
    
  for (int i = 5; i > -1; i--) {
    router += String(bssid[i], HEX);
    router += String(":");
  }
  // router[strlen(router)-1] = '\0';
  // router = router[0:-1];

  // once every interval, send a message:
  // if (millis() - lastTimeSent > interval) {
  // if (lastRSSI - signalStregth >= 4  || lastBSSID != router) {
  if (lastBSSID != router) {    
    
    // String message = "RSSI";
    // message.replace("RSSI", String(signalStregth));
    // mqttClient.beginMessage(topic);
    // // print the body of the message:
    // mqttClient.print(message);
    // // send the message:
    // mqttClient.endMessage();
    
    // Serial.print("published a message: ");
    // Serial.println(message);
    // client.println(message);
    
    
    String message = "BSSID";
    message.replace("BSSID", router);
    mqttClient.beginMessage(topic);
    // print the body of the message:
    mqttClient.print(findPosition(message));
    // send the message:
    mqttClient.endMessage();
    Serial.print("published a message: ");
    Serial.println(findPosition(message));

    // timestamp this message:
    // lastTimeSent = millis();
    // lastRSSI=signalStregth;
    lastBSSID=router;
  }
}

boolean connectToBroker() {
  // if the MQTT client is not connected:
  if (!mqttClient.connect(broker, port)) {
    // print out the error message:
    Serial.print("MOTT connection failed. Error no: ");
    Serial.println(mqttClient.connectError());
    // return that you're not connected:
    return false;
  }

  // set the message receive callback:
  mqttClient.onMessage(onMqttMessage);
  // subscribe to a topic:
  Serial.print("Subscribing to topic: ");
  Serial.println(topic);
  mqttClient.subscribe(topic);
  // mqttClient.subscribe(topic2);

  // once you're connected, you
  // return that you're connected:
  return true;
}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.println("Received a message with topic ");
  Serial.print(mqttClient.messageTopic());
  Serial.print(", length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  String incoming = "";
  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    incoming += (char)mqttClient.read();
  }

  // print the incoming message:
  Serial.println(incoming);
}


void connectToNetwork() {
  // try to connect to the network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to: " + String(SECRET_SSID));
    //Connect to WPA / WPA2 network:
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    delay(2000);
  }
  // print IP address once connected:
  Serial.print("Connected. My IP address: ");
  Serial.println(WiFi.localIP());
}

String findPosition(String macAddress){
  if(macAddress == "6c:8b:d3:f5:85:25:"){
    return "north-side open studio";
  }

  else if(macAddress == "dc:8c:37:2f:12:65:"){
    return "classrooms corridoor";
  }

  else if(macAddress == "dc:8c:37:1e:97:85:"){
    return "room 410";
  }

  else if(macAddress == "dc:8c:37:23:f9:5:"){
    return "room 411";
  }

  else if(macAddress == "dc:8c:37:1e:83:5:"){
    return "room 412";
  }

  else if(macAddress == "dc:8c:37:1e:a1:5:"){
    return "room 413";
  }

  else if(macAddress == "6c:8b:d3:be:84:65:"){
    return "ITP entrace";
  }

  else if(macAddress == "dc:8c:37:b:b2:25:"){
    return "hallway behind the shop";
  }

  else if(macAddress == "dc:8c:37:1e:e1:e5:"){
    return "hallway by emergency exit";
  }

  else if(macAddress == "dc:8c:37:1e:9c:45:"){
    return "south-side hallway";
  }

  else if(macAddress == "dc:8c:37:3d:45:5:"){
    return "around the shop";
  }

  else if(macAddress == "dc:8c:37:2f:73:65:"){
    return "south-side open studio";
  }
}
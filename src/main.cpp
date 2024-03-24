/*
*  ESP32 WiFi Connect To an Existing Network
*  Full Tutorial @ https://deepbluembedded.com/esp32-wifi-library-examples-tutorial-arduino/
*/
 
#include <WiFi.h>
#include <PubSubClient.h>

#define DOOR_SENSOR_PIN  19
#define MSG_BUFFER_SIZE 50

const char* ssid = "Locutus";
const char* password = "secret-passw";
const char* mqtt_server = "192.168.1.79";
const char* topic = "sensors/door/meter-cupboard";
const char* status_topic = "sensors/status/door/meter-cupboard";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int value = 0;
int doorState;                   // store the state of the door (open/closed)
int previousState = LOW;         // hold the previous state

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(length);
  Serial.print("] ");
  Serial.println(status_topic);
  if (strcmp(topic, status_topic) == 0 && length == 0) {
    Serial.println("Received the proper message.");
    if (previousState == HIGH) {
        snprintf (msg, MSG_BUFFER_SIZE, "{ \"status\" : \"open\" }");
    } else {
        snprintf (msg, MSG_BUFFER_SIZE, "{ \"status\" : \"closed\" }");
    }
    client.publish(status_topic, msg);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "DoorSensor-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      snprintf (msg, MSG_BUFFER_SIZE, "init");
      client.publish(topic, msg);
      // ... and resubscribe
      client.subscribe(status_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(){

    pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
    Serial.begin(9600);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

}
 
void loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  doorState = digitalRead(DOOR_SENSOR_PIN); // read state
  
  if (doorState == HIGH && previousState != HIGH) {
    previousState = doorState;
    Serial.println("The door is open");
    snprintf (msg, MSG_BUFFER_SIZE, "open");
    client.publish(topic, msg);
  }

  if (doorState == LOW && previousState != LOW) {
    previousState = doorState;
    Serial.println("The door is closed");
    snprintf (msg, MSG_BUFFER_SIZE, "closed");
    client.publish(topic, msg);
  }

}
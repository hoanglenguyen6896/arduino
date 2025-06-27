
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MSG_BUFFER_SIZE  (32)

const char* ssid = "ENOGY T4.1";
const char* password = "12345678";
const char* mqtt_server = "broker.mqtt-dashboard.com";

uint8_t turnLR = 0;
boolean isOpen = true, isClose = true;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];

uint8_t i = 0;
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
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == 'o' && (char)payload[1] == 'p' && (char)payload[2] == 'e' && (char)payload[3] == 'n')
  {
    if (digitalRead(13) == 0)
    {
      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
    }
    else if (digitalRead(12) == 0)
    {
      if (digitalRead(13) == 1)
      {
        turnLR = 1;
      }
      else
      {
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
      }
    }
  }
  else if ((char)payload[0] == 'c' && (char)payload[1] == 'l' && (char)payload[2] == 'o' && (char)payload[3] == 's' && (char)payload[4] == 'e')
  {
    if (digitalRead(12) == 0)
    {
      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
    }
    else if (digitalRead(13) == 0)
    {
      if (digitalRead(12) == 1)
      {
        turnLR = 2;
      }
      else
      {
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
      }
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("MyTestTopic", "hello world");
      // ... and resubscribe
      client.subscribe("MyLedTest");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  pinMode(12, INPUT_PULLUP);    //left switch IB = H, IA = L
  pinMode(13, INPUT_PULLUP);    //right switch OB = L, IA = H

  pinMode(4, OUTPUT);   //IB
  pinMode(5, OUTPUT);   //IA

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if(digitalRead(13) == 0) 
  {
    isOpen = true;
    isClose = false;
  }
  else if(digitalRead(12) == 0) 
  {
    isClose = true;
    isOpen = false;
  }

  if(digitalRead(13) == 1 && digitalRead(12) == 1)
  {
    if(isClose == true)
    {
      digitalWrite(4, HIGH);
      digitalWrite(5, LOW);
    }
    else if(isOpen == true)
    {
      digitalWrite(4, LOW);
      digitalWrite(5, HIGH);
    }
  }
  else if(digitalRead(13) == 1 && turnLR == 1)
  {
    digitalWrite(4, HIGH);
    digitalWrite(5, LOW);
  }
  else if (digitalRead(12) == 1 && turnLR == 2)
  {
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
  }
  else
  {
    digitalWrite(4,LOW);
    digitalWrite(5,LOW);
    turnLR = 0;
  }
  
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    Serial.println(i++);
    client.publish("MyTestTopic", "Hello World");
  }
}

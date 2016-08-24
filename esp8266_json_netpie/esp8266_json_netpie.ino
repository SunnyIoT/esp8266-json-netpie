#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <MicroGear.h>

/* Setting WIFI */
const char* ssid     = "MAXNET";
const char* password = "macrol3ios";

/* Setting NETPIE */
#define APPID   "WebApp"
#define KEY     "kuZyWfJrig1mb7E"
#define SECRET  "ZNMyde0RNtPR04SUz2kLS0woM"
#define ALIAS   "esp8266-sensor-json" // Change to serial number device
WiFiClient client;
int timer = 0, state_connect = 0;
MicroGear microgear(client);

/* Setting DHT22 */
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

/* Setting JSON */
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

/* Function NETPIE */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
    Serial.print("Incoming message --> ");
    msg[msglen] = '\0';
    Serial.println((char *)msg);
}
void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.print("Found new member --> ");
    for (int i=0; i<msglen; i++)
        Serial.print((char)msg[i]);
    Serial.println();
}
void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.print("Lost member --> ");
    for (int i=0; i<msglen; i++)
        Serial.print((char)msg[i]);
    Serial.println();
}
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.println("Connected to NETPIE...");
    microgear.setAlias(ALIAS);
}

void setup() {
  microgear.on(MESSAGE,onMsghandler);
  microgear.on(PRESENT,onFoundgear);
  microgear.on(ABSENT,onLostgear);
  microgear.on(CONNECTED,onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");
  if (WiFi.begin(ssid, password)) {
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
      }
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  microgear.init(KEY,SECRET,ALIAS);
  microgear.connect(APPID);

  dht.begin();
}

void loop() {
//  delay(2000);

  if (microgear.connected()) {
        if (state_connect == 0) {
          Serial.println("connected");
          state_connect = 1;
        }
        microgear.loop();

        if (timer >= 1000) {
            int humid = dht.readHumidity();
            int temp = dht.readTemperature();

            if (isnan(humid) || isnan(temp)) {
              Serial.println("Failed to read from DHT sensor!");
//              return;
            } else {
              root["temperature"] = temp;
              root["humidity"] = humid;
              Serial.println();
              char buffer[256];
              root.printTo(buffer, sizeof(buffer));
              Serial.println(buffer);
              Serial.println();
              microgear.chat(ALIAS,buffer);
              Serial.println("Publish.");
            }

            timer = 0;
        } else {
          timer += 100;
        }
    } else {
        Serial.println("connection lost, reconnect...");
        if (timer >= 5000) {
            microgear.connect(APPID);
            timer = 0;
            state_connect = 0;
        } else {
          timer += 100;
        }
    }
    delay(100);
}

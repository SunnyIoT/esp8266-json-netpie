#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <MicroGear.h>

/* Setting WIFI */
const char* ssid            = "MAXNET";
const char* password        = "macrol3ios";

/* Setting NETPIE */
#define APPID               "WebApp"
#define KEY                 "kuZyWfJrig1mb7E"
#define SECRET              "ZNMyde0RNtPR04SUz2kLS0woM"
char ALIAS[15]              = "14365118"; // Change your serial number device
char html_alias[15]         = "html_";

String netpie_incoming_text = "\0";
int secure_incoming_text = 0;

EspClass Esp;

WiFiClient client;
int timer = 0, state_connect = 0, state_change_alias = 0;
MicroGear microgear(client);

/* Setting DHT22 */
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

/* Setting JSON */
StaticJsonBuffer<200> jsonExportBuffer;
JsonObject& json_export = jsonExportBuffer.createObject();

StaticJsonBuffer<200> jsonIncomingBuffer;
JsonObject& json_incoming = jsonIncomingBuffer.createObject();

/* Function NETPIE */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
    Serial.print("Incoming message --> ");
    msg[msglen] = '\0';
    netpie_incoming_text = (char*)msg;
    Serial.println(netpie_incoming_text);
    JsonObject& json_incoming = jsonIncomingBuffer.parseObject(netpie_incoming_text);

    if (!json_incoming.success()) {
      Serial.println("parseObject() failed");
    } else {
  
      /* Compare value */
      if (json_incoming["set_temp"] != NULL) {
        Serial.println("temp : 23 done.");
        secure_incoming_text = 1; // state secure data
      }

      if (json_incoming["set_height"] != NULL) {

        // insert your function
        
        secure_incoming_text = 1; // state secure data
      }

      if (json_incoming["set_light"] != NULL) {

        // insert your function
        
        secure_incoming_text = 1; // state secure data
      }

      if (json_incoming["set_on_water"] != NULL) {

        // insert your function
        
        secure_incoming_text = 1; // state secure data
      }

      if (json_incoming["set_off_water"] != NULL) {

        // insert your function
        
        secure_incoming_text = 1; // state secure data
      }

      if (secure_incoming_text == 1) {
        microgear.chat(html_alias, "receiver success");
        secure_incoming_text = 0;
      }
      
      jsonIncomingBuffer = StaticJsonBuffer<200>(); // Clear jsonBuffer
      //Serial.println(Esp.getFreeHeap());
      
    }
    
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
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP());

  microgear.init(KEY,SECRET,ALIAS);
  microgear.connect(APPID);

  if (state_change_alias == 0) {
    strcat(html_alias, ALIAS);
    state_change_alias = 1;
  }
  
  dht.begin();
}

void loop() {
  
  if (microgear.connected()) {
        if (state_connect == 0) {
          Serial.println("Microgear Connected");
          state_connect = 1;
        }
        microgear.loop();

        if (timer >= 1000) {
            int humid = dht.readHumidity();
            int temp = dht.readTemperature();
            
            if (isnan(humid) || isnan(temp)) {
              Serial.println("Failed to read from DHT sensor!");
            } else {
              json_export["Temperature"] = temp;
              json_export["Humidity"] = humid;
              Serial.println();
              char buffer[256];
              json_export.printTo(buffer, sizeof(buffer));
              Serial.print("Publish : ");
              Serial.println(buffer);
              
              microgear.chat(html_alias,buffer);

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

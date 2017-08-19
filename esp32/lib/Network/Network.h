#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

class Network
{
  public:
    Network(void (*sleep)());
    void wifi_init();
    void wifi_connect();
    void wifi_disconnect();
    void mqtt_init();
    void mqtt_connect();
    void mqtt_send(int reading, MQTT_CALLBACK_SIGNATURE);
    void mqtt_loop();
    void mqtt_disconnect();
    String get_reply_id();

private:
    const char* ssid = "FASTWEB-1-D2700B";
    const char* password = "978C3B413C";
    const char* mqtt_server = "mqtt.chris.gunawardena.id.au";
    const long mqtt_reply_timeout_ms = 5 * 1000;
    const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 180;    
    const char* request_template = "{\"uri\":\"https://water-9dbfa.firebaseio.com/users/chris/readings.json\",\"method\":\"POST\",\"json\":true,\"body\":{\"level\":123456,\"timestamp\":{\".sv\":\"timestamp\"}},\"reply_id\":\"/request/reply/123456\"}";
    String reply_id;
    WiFiClient wiFiClient;
    PubSubClient pubSubClient;
    unsigned long request_start_time;
    void (*_sleep)();
};

Network::Network(void (*sleep)()) {
    _sleep = sleep;

}

String Network::get_reply_id() {
    return reply_id;
}

void Network::wifi_connect() {
    Serial.println("Connecting to WiFi");
    int wifi_rety_count = 10;
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println(".");
      if(--wifi_rety_count < 0) {
        Serial.println("ERROR: No wifi connection in " + String(wifi_rety_count) + " tries.");
        (*_sleep)();
      }
      delay(1000);
    }
    Serial.println("WiFi connected!");
}

void Network::mqtt_connect() {
    Serial.println("Connecting to MQTT");

    int mqtt_rety_count = 10;
    pubSubClient.setClient(wiFiClient);
    pubSubClient.setServer(mqtt_server, 1883);

    while (!pubSubClient.connected()) {
        String clientId = "esp32client" + String(random(0xffff), HEX);
        reply_id = "/request/reply/" + clientId;
        // if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
        if (pubSubClient.connect(clientId.c_str())) {
          Serial.println("MQTT connected");
        } else {
          Serial.print(".");
          // Serial.print(pubSubClient.state());
          delay(1000);
          if(--mqtt_rety_count <= 0) {
            Serial.println("ERROR: No MQTT connection in " + String(mqtt_rety_count) + " tries.");
            (*_sleep)();
          }  
        }
    }
}
void Network::mqtt_loop() {
    pubSubClient.loop();
}

void Network::mqtt_disconnect() {
    if (pubSubClient.connected())
        pubSubClient.disconnect();
}

void Network::wifi_disconnect() {
    if (WiFi.status() == WL_CONNECTED) 
        WiFi.disconnect();
}


void Network::mqtt_send(int reading, MQTT_CALLBACK_SIGNATURE) {
    pubSubClient.setCallback(callback);    
    DynamicJsonBuffer jsonBuffer(bufferSize);    
    JsonObject& root = jsonBuffer.parseObject(request_template);
    root["reply_id"] = reply_id;
    pubSubClient.subscribe(reply_id.c_str());
    root["body"]["level"] = reading;
    String request;
    root.printTo(request);
    Serial.println(request.c_str());
    pubSubClient.publish("/request", request.c_str());
    request_start_time = millis();
}


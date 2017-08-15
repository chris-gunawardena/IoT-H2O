#include <Arduino.h>
#include <WiFi.h>
#include <esp_log.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "FASTWEB-1-D2700B";
const char* password = "978C3B413C";
const char* mqtt_server = "mqtt.chris.gunawardena.id.au";
RTC_DATA_ATTR int bootCount = 1;
RTC_DATA_ATTR int last_reading = 0;
const int a_in_pin = 34;
const int led_pin = 5;
int sleep_time_seconds = 1 * 60;
const long mqtt_timeout_ms = 5 * 1000;
const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 180;
DynamicJsonBuffer jsonBuffer(bufferSize);
const char* request_template = "{\"uri\":\"https://water-9dbfa.firebaseio.com/users/chris/readings.json\",\"method\":\"POST\",\"json\":true,\"body\":{\"level\":123456,\"timestamp\":{\".sv\":\"timestamp\"}},\"reply_id\":\"/request/reply/123456\"}";
String reply_id;
WiFiClient wiFiClient;
PubSubClient pubSubClient(wiFiClient);
unsigned long request_start_time;

void sleep() {
  esp_deep_sleep_enable_timer_wakeup(sleep_time_seconds * 1000000);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();  
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, reply_id.c_str()) == 0) {
    digitalWrite(led_pin, LOW);
    sleep();
  }
}

void setup(){
  //esp_log_level_set("*", ESP_LOG_VERBOSE);
  Serial.begin(115200);
  delay(5000);
  randomSeed(micros());

  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);

  // measure 
  int reading = analogRead(a_in_pin);
  Serial.println("Reading: " + String(reading));

  if (reading != last_reading && reading != 0) {
    // WiFi
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) {
      delay(5000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");

    // MQTT
    pubSubClient.setServer(mqtt_server, 1883);
    pubSubClient.setCallback(mqtt_callback);

    while (!pubSubClient.connected()) {
      String clientId = "esp32client" + String(random(0xffff), HEX);
      reply_id = "/request/reply/" + clientId;
      // if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
      if (pubSubClient.connect(clientId.c_str())) {
        Serial.println("mqtt connected");
        JsonObject& root = jsonBuffer.parseObject(request_template);
        root["reply_id"] = reply_id;
        pubSubClient.subscribe(reply_id.c_str());
        root["body"]["level"] = reading;
        String request;
        root.printTo(request);
        Serial.println(request.c_str());
        pubSubClient.publish("/request", request.c_str());
        request_start_time = millis();
        last_reading = reading;
      } else {
        Serial.print("mqtt failed, rc=");
        Serial.print(pubSubClient.state());
        Serial.println("mqtt try again in 5 seconds");
        delay(5000);
      }
    }
  }
}

void loop() {
  pubSubClient.loop();
  if ((millis() - request_start_time) > mqtt_timeout_ms) {
    sleep();
  }
}


// void log_memory() {
//   UBaseType_t uxHighWaterMark2 = uxTaskGetStackHighWaterMark( NULL );
//   Serial.print( "Memory: " );
//   Serial.print(uxHighWaterMark2 );
//   Serial.print( " ");
//   Serial.println( esp_get_free_heap_size() );
// }
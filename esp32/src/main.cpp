#include <Arduino.h>
#include <WiFi.h>
#include <esp_log.h>
#include <PubSubClient.h>

const char* ssid = "FASTWEB-1-D2700B";
const char* password = "978C3B413C";
const char* mqtt_server = "192.168.1.199";
RTC_DATA_ATTR int bootCount = 1;
RTC_DATA_ATTR int last_reading = 0;
const int a_in_pin = 34;
const int led_pin = 5;
int sleep_time_seconds = 1 * 60;
String reply_id;

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, reply_id.c_str()) == 0) {
    digitalWrite(led_pin, LOW);
    esp_deep_sleep_enable_timer_wakeup(sleep_time_seconds * 1000000);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
    esp_deep_sleep_start();  
  }
}

void setup(){
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  Serial.begin(115200);
  delay(5000);
  randomSeed(micros());

  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);

  Serial.println("Boot number: " + String(bootCount++));

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
    WiFiClient wiFiClient;
    PubSubClient pubSubClient(wiFiClient);
    pubSubClient.setServer(mqtt_server, 1883);
    pubSubClient.setCallback(mqtt_callback);

    while (!pubSubClient.connected()) {
      String clientId = "esp32client" + String(random(0xffff), HEX);
      reply_id = "/request/reply/" + clientId;
      // if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
      if (pubSubClient.connect(clientId.c_str())) {
        Serial.println("mqtt connected");
        pubSubClient.subscribe(reply_id.c_str());
        pubSubClient.publish("/request", "{\"uri\":\"https://water-9dbfa.firebaseio.com/users/chris/readings.json\",\"method\":\"POST\",\"json\":true,\"body\":{\"level\":123456,\"timestamp\":{\".sv\":\"timestamp\"}},\"reply_id\":\"/request/reply/xoxo\"}");
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

void loop(){ }


// void log_memory() {
//   UBaseType_t uxHighWaterMark2 = uxTaskGetStackHighWaterMark( NULL );
//   Serial.print( "Memory: " );
//   Serial.print(uxHighWaterMark2 );
//   Serial.print( " ");
//   Serial.println( esp_get_free_heap_size() );
// }
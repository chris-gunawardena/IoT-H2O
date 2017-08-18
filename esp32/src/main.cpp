#include <Arduino.h>
#include <esp_log.h>
#include <Measurement.h>
#include <Network.h>

RTC_DATA_ATTR int last_reading = 0;
const int a_in_pin = 34;
const int led_pin = 5;
int sleep_time_seconds = 1 * 60;
void sleep();
Network network(sleep);
String reply_id;


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, network.get_reply_id().c_str()) == 0) {
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
  Messurement messurement(a_in_pin);
  int reading = messurement.get_level();
  Serial.println("Reading: " + String(reading));

  if (reading == last_reading || reading == 0) {
    sleep();
  } else {
    network.wifi_connect();
    network.mqtt_connect();
    network.mqtt_send(reading, mqtt_callback);
  }
}

void loop() {
  // Serial.println(analogRead(a_in_pin));  
  network.mqtt_loop();
}

void sleep() {
  digitalWrite(led_pin, LOW);
  esp_deep_sleep_enable_timer_wakeup(sleep_time_seconds * 1000000);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  // esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  // esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  // esp_deep_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();  
}

// void log_memory() {
//   UBaseType_t uxHighWaterMark2 = uxTaskGetStackHighWaterMark( NULL );
//   Serial.print( "Memory: " );
//   Serial.print(uxHighWaterMark2 );
//   Serial.print( " ");
//   Serial.println( esp_get_free_heap_size() );
// }

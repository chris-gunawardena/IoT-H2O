#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
 
const char* ssid = "FASTWEB-1-D2700B";
const char* password = "978C3B413C";
const char* firebase_ca =     "-----BEGIN CERTIFICATE-----\n" \
    "MIIEKDCCAxCgAwIBAgIQAQAhJYiw+lmnd+8Fe2Yn3zANBgkqhkiG9w0BAQsFADBC\n" \
    "MQswCQYDVQQGEwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMS\n" \
    "R2VvVHJ1c3QgR2xvYmFsIENBMB4XDTE3MDUyMjExMzIzN1oXDTE4MTIzMTIzNTk1\n" \
    "OVowSTELMAkGA1UEBhMCVVMxEzARBgNVBAoTCkdvb2dsZSBJbmMxJTAjBgNVBAMT\n" \
    "HEdvb2dsZSBJbnRlcm5ldCBBdXRob3JpdHkgRzIwggEiMA0GCSqGSIb3DQEBAQUA\n" \
    "A4IBDwAwggEKAoIBAQCcKgR3XNhQkToGo4Lg2FBIvIk/8RlwGohGfuCPxfGJziHu\n" \
    "Wv5hDbcyRImgdAtTT1WkzoJile7rWV/G4QWAEsRelD+8W0g49FP3JOb7kekVxM/0\n" \
    "Uw30SvyfVN59vqBrb4fA0FAfKDADQNoIc1Fsf/86PKc3Bo69SxEE630k3ub5/DFx\n" \
    "+5TVYPMuSq9C0svqxGoassxT3RVLix/IGWEfzZ2oPmMrhDVpZYTIGcVGIvhTlb7j\n" \
    "gEoQxirsupcgEcc5mRAEoPBhepUljE5SdeK27QjKFPzOImqzTs9GA5eXA37Asd57\n" \
    "r0Uzz7o+cbfe9CUlwg01iZ2d+w4ReYkeN8WvjnJpAgMBAAGjggERMIIBDTAfBgNV\n" \
    "HSMEGDAWgBTAephojYn7qwVkDBF9qn1luMrMTjAdBgNVHQ4EFgQUSt0GFhu89mi1\n" \
    "dvWBtrtiGrpagS8wDgYDVR0PAQH/BAQDAgEGMC4GCCsGAQUFBwEBBCIwIDAeBggr\n" \
    "BgEFBQcwAYYSaHR0cDovL2cuc3ltY2QuY29tMBIGA1UdEwEB/wQIMAYBAf8CAQAw\n" \
    "NQYDVR0fBC4wLDAqoCigJoYkaHR0cDovL2cuc3ltY2IuY29tL2NybHMvZ3RnbG9i\n" \
    "YWwuY3JsMCEGA1UdIAQaMBgwDAYKKwYBBAHWeQIFATAIBgZngQwBAgIwHQYDVR0l\n" \
    "BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMA0GCSqGSIb3DQEBCwUAA4IBAQDKSeWs\n" \
    "12Rkd1u+cfrP9B4jx5ppY1Rf60zWGSgjZGaOHMeHgGRfBIsmr5jfCnC8vBk97nsz\n" \
    "qX+99AXUcLsFJnnqmseYuQcZZTTMPOk/xQH6bwx+23pwXEz+LQDwyr4tjrSogPsB\n" \
    "E4jLnD/lu3fKOmc2887VJwJyQ6C9bgLxRwVxPgFZ6RGeGvOED4Cmong1L7bHon8X\n" \
    "fOGLVq7uZ4hRJzBgpWJSwzfVO+qFKgE4h6LPcK2kesnE58rF2rwjMvL+GMJ74N87\n" \
    "L9TQEOaWTPtEtyFkDbkAlDASJodYmDkFOA/MgkgMCkdm7r+0X8T/cKjhf4t5K7hl\n" \
    "MqO5tzHpCvX2HzLc\n" \
    "-----END CERTIFICATE-----\n";;

RTC_DATA_ATTR int bootCount = 1;
RTC_DATA_ATTR int last_reading = 0;
const int a_in_pin = 34;
HTTPClient http;

void setup(){
  Serial.begin(115200);
  delay(5000);
  bootCount++;
  Serial.println("Boot number: " + String(bootCount));

  // measure 
  int reading = analogRead(a_in_pin);
  Serial.println("Reading: " + String(reading));
  if (reading != last_reading && reading != 0) {
    last_reading = reading;

    // WiFi
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");

    // POST  
    http.setReuse(true);
    http.begin("https://water-9dbfa.firebaseio.com/users/chris/readings.json", firebase_ca);  //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json");             //Specify content-type header
    int httpResponseCode = http.POST("{ \"level\": " + String(reading) + ", \"timestamp\": {\".sv\": \"timestamp\"} }");   
    Serial.println("httpResponseCode: " + http.errorToString(httpResponseCode));
    http.end();
  }

  // sleep
  esp_deep_sleep_enable_timer_wakeup(10 * 1000000);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();
}

void loop(){
  // int reading = analogRead(a_in_pin);
  // Serial.println("Reading: " + String(reading));

  // if (reading != last_reading && reading != 0) {
  //   last_reading = reading;

  //   http.begin("https://water-9dbfa.firebaseio.com/users/chris/settings.json", firebase_ca);
  //   http.addHeader("Content-Type", "application/json");
  //   int httpResponseCode = http.PUT("{ \"reading\": " + String(reading) + ", \"timestamp\": {\".sv\": \"timestamp\"} }");   
  //   Serial.println("httpResponseCode: " + http.errorToString(httpResponseCode));
  // }
  // delay(100);
}
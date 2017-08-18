#include "Arduino.h"

class Messurement
{
  public:
    Messurement(int pin);
    int get_level();
  private:
    static const int num_readings = 10;
    static const int allowed_variance = 5;         // % wait for variance
    static const int delay_between_readings = 10; //ms
    static const int reading_timeout = 10 * 1000;
    int _pin;
    int readings[num_readings];      // the readings from the analog input
    int read_index;              // the index of the current reading
    int total = 0;                  // the running total
    int average = 1;                // the average
    void (*_sleep)();
};

Messurement::Messurement(int pin)
{ _pin = pin;
  for(int i=0; i<num_readings; i++) {
    readings[i] = 0;
  }
}

int Messurement::get_level() {
  const long request_start_time = millis();
  int variance;
  do{
    total = total - readings[read_index];
    readings[read_index] = analogRead(_pin);
    total = total + readings[read_index];
    variance = abs(100 - (readings[read_index] * 100 / average));
    Serial.println("Reading " + String(readings[read_index] + " variance" + String(variance) + "/" + String(allowed_variance)));
    read_index++;
    if (read_index >= num_readings) {
      read_index = 0;
    }
    average = total / num_readings;
    delay(delay_between_readings);
  } while(variance > allowed_variance && (millis() - request_start_time) < reading_timeout);
  return average;
}

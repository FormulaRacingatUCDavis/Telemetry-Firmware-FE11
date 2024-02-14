#include "esp_now.h"
#include <WiFi.h>

unsigned long timer;
unsigned long currTime;
long loopTime = 5000;  // in microseconds

void setup() {
  // Initialize Serial Connection to PC
  Serial.begin(38400);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  timer = micros();
}

void loop() {
  // Nothing here since action is only taken when data is received
}

void timeSync(unsigned long deltaT) {
  currTime = micros();
  long timeToDelay = deltaT - (currTime - timer);
  if (timeToDelay > 5000) {
    delay(timeToDelay / 1000);
    delayMicroseconds(timeToDelay % 1000);
  } else if (timeToDelay > 0) {
    delayMicroseconds(timeToDelay);
  } else {
      // timeToDelay is negative so we start immediately
  }
  timer = currTime + timeToDelay;
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  timeSync(loopTime);
  Serial.write(incomingData, len);
}

#include "esp_now.h"
#include <WiFi.h>

unsigned long timer;
unsigned long currTime;
long loopTime = 2000;  // in microseconds

// for testing
void printPacket(const uint8_t* packet) {
  // values that shouldn't be sent over
  uint8_t valid[2] = {0x0f, 0x0f};
  uint16_t id = 50;
  uint16_t vals[4];
  uint32_t tval = 10;

  memcpy(valid, packet, 2);
  memcpy(&id, packet+2, 2);
  memcpy(vals, packet+4, 8);
  memcpy(&tval, packet+12, 4);

  for (int i = 0; i < 2; i++) {
    Serial.print("valid[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(valid[i]);
  }
  Serial.print("id = ");
  Serial.println(id);
  for (int i = 0; i < 4; i++) {
    Serial.print("vals[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(vals[i]);
  }
  Serial.print("time = ");
  Serial.println(tval);
}

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

  // Send to graphing
  // Serial.write(incomingData, len);
  
  // Print to serial moniter
  printPacket(incomingData);
}

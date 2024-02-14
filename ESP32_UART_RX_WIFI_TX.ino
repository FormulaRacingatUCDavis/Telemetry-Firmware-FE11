#include <esp_now.h>
#include <WiFi.h>
#include "Packet.h"
#define RXD2 16
#define TXD2 17

esp_now_peer_info_t peerInfo;

// REPLACE WITH YOUR RECEIVER MAC Address
// 24:0A:C4:61:50:C8 transceiver
// 24:6F:28:7A:93:44 old test esp32
// 94:3C:C6:34:6E:68 new test esp32
uint8_t broadcastAddress[] = {0x94, 0x3C, 0xC6, 0x34, 0x6E, 0x68};

// buffer for receiving and sending packets
uint8_t data[Packet::length];

// for testing
uint16_t id = 50;
uint16_t vals[4];
uint32_t tval = 10;

void setup() {
  Serial.begin(115200);  // for debugging
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  // for UART

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of transmitted packets
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  if (Serial2.available() >= Packet::length) { 
    Serial2.readBytes(data, Packet::length);
    esp_err_t result = esp_now_send(broadcastAddress, data, Packet::length);
    
    Serial.println(result == ESP_OK ? "Sent with success" : "Error sending the data");

    memcpy(&id, data, 2);
    memcpy(vals, data+2, 8);
    memcpy(&tval, data+10, 4);

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
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Delivery Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
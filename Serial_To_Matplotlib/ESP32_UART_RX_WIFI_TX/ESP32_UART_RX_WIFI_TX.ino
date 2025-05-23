#include <esp_now.h>
#include <WiFi.h>
#include "Packet.h"
#define RXD2 17
#define TXD2 16

// can include the class directly without Packet.h
// class Packet {
//  public:
//   static const size_t length = 16;
//   char* validation;
//   int data_id;
//   short* data;
//   unsigned long time;
//   Packet(char* vd, int id, short* d, unsigned long t) : validation(vd), data_id(id), data(d), time(t) {};
// };

esp_now_peer_info_t peerInfo;

// REPLACE WITH YOUR RECEIVER MAC Address
// 24:0A:C4:61:50:C8 transceiver
// 24:6F:28:7A:93:44 test esp32 labeled '2'
// 94:3C:C6:34:6E:68 test esp32 unlabeled
uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0x7A, 0x93, 0x44};
// uint8_t broadcastAddress[] = {0x24, 0x0A, 0xC4, 0x61, 0x50, 0xC8};

// buffer for receiving and sending packets
uint8_t data[Packet::length];

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
    Serial.println("available");
    Serial2.readBytes(data, 1);
    if (data[0] == 0x00) {
      Serial2.readBytes(data+1, 1);
      if (data[1] == 0xff) {
        Serial2.readBytes(data+2, Packet::length - 2);
        esp_err_t result = esp_now_send(broadcastAddress, data, Packet::length);
    
        Serial.println(result == ESP_OK ? "Sent with success" : "Error sending the data");
        // printPacket(data);
      }
    }
  }
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Delivery Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
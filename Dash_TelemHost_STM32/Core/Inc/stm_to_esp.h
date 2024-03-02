#ifndef SRC_STM_TO_ESP_H_
#define SRC_STM_TO_ESP_H_

#define PACKET_LENGTH 14

typedef struct Packet {
  uint16_t data_id;
  uint16_t* data;
  uint32_t time;
} Packet;

// Function prototypes
void timer_init(void);
void read_send_loop(void);
void reset(void);
void send(Packet* p);

#endif /* SRC_STM_TO_ESP_H_ */

#ifndef SRC_STM_TO_ESP_H_
#define SRC_STM_TO_ESP_H_

#include "main.h"

#define PACKET_LENGTH 16

typedef struct __attribute__((__packed__)) Packet {
	uint8_t validation[2];
	uint16_t data_id;
	uint16_t data[4];
	uint32_t time;
} Packet;

// Function prototypes
void timer_init(void);
void read_send_loop(void);
void reset(void);

#endif /* SRC_STM_TO_ESP_H_ */

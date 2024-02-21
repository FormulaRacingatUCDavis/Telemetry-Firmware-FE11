/*
 * packet.h
 *
 *  Created on: Dec 2, 2023
 *      Author: nickr
 */

#ifndef SRC_PACKET_H_
#define SRC_PACKET_H_

#define PACKET_LENGTH 14

typedef struct Packet {
  uint16_t data_id;
  uint16_t* data;
  uint32_t time;
} Packet;

#endif /* SRC_PACKET_H_ */

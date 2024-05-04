#include <stdint.h>
#include <stdio.h>
#include "ugui.h"
#include "telem.h"
#include "can_manager.h"

const uint8_t packet_validation[2] = {0x00, 0xff};
uint32_t send_time;
uint32_t prev_time = 0;
char msg[100];

extern UART_HandleTypeDef huart7;

void telem_send(void) {
//	sprintf(msg, "telem_id = %u\n", telem_id);
//	UG_PutString(5, 250, msg);
//	sprintf(msg, "wheel_updated[1] = %u\n", wheel_updated[1]);
//	sprintf(msg, "rr = %u\n", rear_right_wheel_speed);
//	sprintf(msg, "rl = %u\n", rear_left_wheel_speed);
//	sprintf(msg, "it = %d\n", inlet_temp);
//	sprintf(msg, "ot = %d\n", outlet_temp);
//	if (telem_id == 1 || (telem_id == 0 && wheel_updated[0] && wheel_updated[1])) {
		// enough data received to send a packet
		send_time = HAL_GetTick();
		Packet p = {{packet_validation[0], packet_validation[1]},
					telem_id, {0, 0, 0, 0}, send_time};
		switch(telem_id) {
			case 0:
				p.data[0] = front_right_wheel_speed;
				p.data[1] = front_left_wheel_speed;
				p.data[2] = rear_right_wheel_speed;
				p.data[3] = rear_left_wheel_speed;
				break;
			case 1:
				p.data[0] = inlet_temp;
				p.data[1] = outlet_temp;
				p.data[2] = inlet_pres;
				p.data[3] = outlet_pres;
				break;
			default:
				return;
		}
//		sprintf(msg, "st = %lu\n", send_time);
//		UG_PutString(10, 250, msg);
//		sprintf(msg, "pt = %lu\n", prev_time);
//		if (send_time - prev_time > 20) {
			HAL_UART_Transmit(&huart7, (uint8_t*)&p, PACKET_LENGTH, 1000);
			telem_reset();
			HAL_Delay(20);
//		}
//	}
}

void telem_reset() {
	if (telem_id == 0) {
		// change if/when we have front wheel sensors
		wheel_updated[0] = 1;
		wheel_updated[1] = 0;
		telem_id = 1;
	} else {
		telem_id = 0;
	}
	prev_time = send_time;
}

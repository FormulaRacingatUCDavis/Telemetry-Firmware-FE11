#include <stdint.h>
#include "stm32f7xx_hal.h"
#include "stm_to_esp.h"

// Global variables
int canId;
uint16_t id = -1;
uint16_t data[2][4];
char wheelUpdated[4] = {0, 0, 0, 0};
uint32_t time;
uint32_t startTime;

// Temp for simulation
int canIdVals[] = {0x470, 0x471, 0x472, 0x473, 0x475};
int canIdIndex = 0;

// UART
extern UART_HandleTypeDef huart7;

void timer_init(void) {
	SysTick_Config(SystemCoreClock / 1000);
	startTime = HAL_GetTick();
}

void read_send_loop(void) {
	// currently simulating can messages, replace with real can access code
	for(uint16_t fakeData = 0; fakeData < 50; fakeData++)
	{
		canId = canIdVals[canIdIndex++ % 5];

		if (canId >= 0x470 && canId <= 0x473) {
			// wheel speed data
		    id = 0;
		    // could include math here to convert sensor data to speed
		    data[id][canId - 0x470] = fakeData;
		    wheelUpdated[canId - 0x470] = 1;
		} else if (canId == 0x475) {
		    // steering data
		    id = 1;
		    data[id][0] = fakeData;
		}

		if (id == 1 || (id == 0 && wheelUpdated[0] && wheelUpdated[1] && wheelUpdated[2] && wheelUpdated[3])) {
			// enough data received to send a packet
		    time = HAL_GetTick() - startTime;
		    Packet p = {id, data[id], time};
		    send(&p);
		    reset();
		    // delay 50 ms, can change
		    HAL_Delay(50);
		}
	}
}

void reset() {
  if (id == 0) {
    wheelUpdated[0] = 0;
    wheelUpdated[1] = 0;
    wheelUpdated[2] = 0;
    wheelUpdated[3] = 0;
  }
  id = -1;
}

void send(Packet* p) {
  char* byteId = (char*)(&(p->data_id));
  char* byteData = (char*)(p->data);
  char* byteTime = (char*)(&(p->time));
  uint8_t buf[PACKET_LENGTH];
  buf[0] = byteId[0];
  buf[1] = byteId[1];
  buf[2] = byteData[0];
  buf[3] = byteData[1];
  buf[4] = byteData[2];
  buf[5] = byteData[3];
  buf[6] = byteData[4];
  buf[7] = byteData[5];
  buf[8] = byteData[6];
  buf[9] = byteData[7];
  buf[10] = byteTime[0];
  buf[11] = byteTime[1];
  buf[12] = byteTime[2];
  buf[13] = byteTime[3];
  HAL_UART_Transmit(&huart7, buf, PACKET_LENGTH, 1000);
}

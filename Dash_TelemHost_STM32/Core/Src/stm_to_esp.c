#include <stdint.h>
#include "stm_to_esp.h"

// Global variables
int canId;
uint8_t valid[2] = {0x00, 0xff};
uint16_t id = -1;
uint16_t data[2][4];
uint8_t wheelUpdated[4] = {0, 0, 0, 0};
uint32_t time;
uint32_t startTime;

// Temp for simulation
int canIdVals[] = {0x470, 0x471, 0x472, 0x473, 0x400};
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
		} else if (canId == 0x400) {
		    // temp data
		    id = 1;
		    data[id][0] = fakeData;
		    data[id][1] = fakeData;
		}

		if (id == 1 || (id == 0 && wheelUpdated[0] && wheelUpdated[1]
								&& wheelUpdated[2] && wheelUpdated[3])) {
			// enough data received to send a packet
		    time = HAL_GetTick() - startTime;
		    Packet p = {{valid[0], valid[1]}, id,
		    			{data[id][0], data[id][1], data[id][2], data[id][3]}, time};
		    HAL_UART_Transmit(&huart7, (uint8_t*)&p, PACKET_LENGTH, 1000);
		    reset();
		    // delay 20 ms, can't send too fast
		    HAL_Delay(20);
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

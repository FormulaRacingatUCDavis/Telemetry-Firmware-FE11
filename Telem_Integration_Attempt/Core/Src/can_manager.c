#include "can_manager.h"

#include "serial_print.h"
#include "traction_control.h"

volatile uint8_t mc_lockout;
volatile uint8_t mc_enabled;
volatile uint16_t capacitor_volt = 0;
volatile uint8_t shutdown_flags = 0b00111000;  //start with shutdown flags OK
volatile uint8_t estop_flags = 0;
volatile uint8_t switches = 0xC0;   //start with switches on to stay in startup state
volatile uint8_t PACK_TEMP;
volatile uint8_t mc_fault;
volatile uint8_t soc;
volatile uint16_t bms_status;
volatile uint8_t mc_fault_clear_success = 0;
volatile uint16_t pack_voltage;

volatile uint16_t front_right_wheel_speed = 0;
volatile uint16_t front_left_wheel_speed = 0;
volatile uint16_t rear_right_wheel_speed = 0;
volatile uint16_t rear_left_wheel_speed = 0;
volatile uint8_t wheel_updated[2] = {1,0};
volatile int16_t inlet_temp = 0;
volatile int16_t outlet_temp = 0;
volatile int16_t inlet_pres = 0;
volatile int16_t outlet_pres = 0;
volatile uint16_t telem_id = 0;

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);
	save_can_rx_data(RxHeader, RxData);
}


/************ CAN RX ************/

void save_can_rx_data(CAN_RxHeaderTypeDef RxHeader, uint8_t RxData[]) {
    // gets message and updates values
	switch (RxHeader.StdId) {
		case BMS_STATUS_MSG:
			PACK_TEMP = RxData[0];
			soc = RxData[1];
			bms_status = (RxData[2] << 8);
			bms_status += RxData[3];
			pack_voltage = (RxData[4] << 8);
			pack_voltage += RxData[5];
			temp_attenuate();
			break;
		case MC_VOLTAGE_INFO:
			capacitor_volt = (RxData[0] << 8); // upper bits
			capacitor_volt += RxData[1]; // lower bits
			break;
		case MC_INTERNAL_STATES:
			mc_lockout = RxData[6] & 0b1000000;
			mc_enabled = RxData[6] & 0b1;
			break;
		case PEI_CURRENT_SHUTDOWN:
			shutdown_flags = RxData[2];
			break;
		case MC_FAULT_CODES:
			for (uint8_t i = 0; i < 8; i++) {
				if (RxData[i] > 0) {
					mc_fault = 1;
					break;
				}
				else {
					mc_fault = 0;
				}
			}
		case MC_PARAM_RESPONSE:
			if (RxData[0] == 0x20 && RxData[2] == 1) {
				mc_fault_clear_success = 1;
			}
			break;
/* 		  case WHEEL_SPEED_FRONT:
			front_right_wheel_speed = (RxData[0] << 8);
			front_right_wheel_speed += RxData[1];
			front_left_wheel_speed = (RxData[2] << 8);
			front_left_wheel_speed += RxData[3];
			wheel_updated[0] = 1;
			telem_id = 0;
			break;
*/
		case WHEEL_SPEED_REAR:
			rear_right_wheel_speed = (RxData[0] << 8);
			rear_right_wheel_speed += RxData[1];
			rear_left_wheel_speed = (RxData[2] << 8);
			rear_left_wheel_speed += RxData[3];
			wheel_updated[1] = 1;
			telem_id = 0;
			break;
		case COOLING_LOOP:
			inlet_temp = (RxData[0] << 8);
			inlet_temp += RxData[1];
			outlet_temp = (RxData[2] << 8);
			outlet_temp += RxData[3];
			inlet_pres = (RxData[4] << 8);
			inlet_pres += RxData[5];
			outlet_pres = (RxData[6] << 8);
			outlet_pres += RxData[7];
			telem_id = 1;
		default:
			// no valid input received
			break;
	}

}


/************ CAN TX ************/

CAN_TxHeaderTypeDef   TxHeader;
uint32_t              TxMailbox;

//  transmit state
void can_tx_vcu_state(CAN_HandleTypeDef *hcan){
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.StdId = VEHICLE_STATE;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = 6;
	uint8_t data_tx_state[6] = {
        0,
        hv_requested(),
        throttle1.percent,
        throttle2.percent,
		brake.percent,
        one_byte_state(),

    };

    if (HAL_CAN_AddTxMessage(hcan, &TxHeader, data_tx_state, &TxMailbox) != HAL_OK)
	{
	  print("CAN Tx failed\r\n");
	}
}


// transmit torque request
void can_tx_torque_request(CAN_HandleTypeDef *hcan){
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.StdId = TORQUE_REQUEST;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = 8;

    uint16_t throttle_msg_byte = 0;
    if (state == DRIVE) {
    	uint16_t throttle_req = requested_throttle();
    	if (throttle_req  < 50) {
    		throttle_req = 0;
    	}
        throttle_msg_byte = throttle_req - TC_torque_adjustment;
    }

    uint8_t byte5 = 0b010;   //speed mode | discharge_enable | inverter enable

    if (state == DRIVE) {
    	byte5 |= (hv_requested() & 0x01);  //set inverter enable bit
    }


    uint8_t data_tx_torque[8] = {
        (uint8_t)(throttle_msg_byte & 0xff), // 0 - torque command lower (Nm*10)
        (uint8_t)(throttle_msg_byte >> 8) & 0xFF, // 1 - torque command upper (Nm*10)
        0, // 2 - speed command lower (not applicable)
        0, // 3 - speed command upper (not applicable)
        1, // 4 - direction (1 = forward, 0 = backward)
        byte5, // 5 - speed mode | discharge_enable | inverter enable
        0, // 6 - torque limit lower (if 0, default EEPROM value used)
        0 // 7 - torque limit upper (if 0, default EEPROM value used)
    };

    if (HAL_CAN_AddTxMessage(hcan, &TxHeader, data_tx_torque, &TxMailbox) != HAL_OK)
	{
	  print("CAN Tx failed\r\n");
	}
}


void can_tx_disable_MC(CAN_HandleTypeDef *hcan) {
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.StdId = TORQUE_REQUEST;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = 8;

	uint8_t data_tx_torque[8] = {0,0,0,0,0,0,0,0};

	if (HAL_CAN_AddTxMessage(hcan, &TxHeader, data_tx_torque, &TxMailbox) != HAL_OK)
	{
	  print("CAN Tx failed\r\n");
	}
}

void can_clear_MC_fault(CAN_HandleTypeDef *hcan) {
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.StdId = MC_PARAM_COMMAND;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = 8;

	const uint16_t param_addr = 20;
	uint8_t data_tx_param_command[8] = {
			param_addr & 0xFF, // address lower (little endian)
			param_addr << 8, // address upper
			1, // r/w: 1 = write
			0, // reserved
			0, // data
			0, // data
			0, // reserved
			0 // reserved
	};

	if (HAL_CAN_AddTxMessage(hcan, &TxHeader, data_tx_param_command, &TxMailbox) != HAL_OK)
	{
	  print("CAN Tx failed\r\n");
	}
}

/*
 * sensors.c
 *
 *  Created on: Feb 20, 2024
 *      Author: cogus
 */
#include "sensors.h"
#include "can_manager.h"
#include "frucd_display.h"
#include <stdlib.h>

CALIBRATED_SENSOR_t throttle1;
CALIBRATED_SENSOR_t throttle2;
CALIBRATED_SENSOR_t brake;
uint32_t torque_percentage = 100;

//extern void Error_Handler();

/************ Timer ************/
unsigned int discrepancy_timer_ms = 0;


void init_sensors(){
    throttle1.min = 0x7FFF;
    throttle1.max = 0;
    throttle1.range = 1;
    throttle2.min = 0x7FFF;
    throttle2.max = 0;
    throttle2.range = 1;
    brake.min = 0x7FFF;
    brake.max = 0;
    brake.range = 1;
}

// APPS
uint8_t THROTTLE_MULTIPLIER = 100;
const uint8_t THROTTLE_MAP[8] = { 95, 71, 59, 47, 35, 23, 11, 5 };

void select_adc_channel(ADC_HandleTypeDef *hadc, ADC_CHAN channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    switch (channel)
    {
        case APPS1:
            sConfig.Channel = ADC_CHANNEL_10;
			sConfig.Rank = 1;

			if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
			{
//				Error_Handler();
			}
			break;

        case APPS2:
			sConfig.Channel = ADC_CHANNEL_8;
			sConfig.Rank = 1;
			if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
			{
//				Error_Handler();
			}
			break;
        case BSE:
			sConfig.Channel = ADC_CHANNEL_15;
			sConfig.Rank = 1;
			if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
			{
//				Error_Handler();
			}
			break;
        case KNOB1:
			sConfig.Channel = ADC_CHANNEL_13;
			sConfig.Rank = 1;
			if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
			{
//				Error_Handler();
			}
			break;
        case KNOB2:
			sConfig.Channel = ADC_CHANNEL_12;
			sConfig.Rank = 1;
			if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
			{
//				Error_Handler();
			}
			break;
        default:
            break;
    }
}

uint32_t get_adc_conversion(ADC_HandleTypeDef *hadc, ADC_CHAN channel) {

	select_adc_channel(hadc, channel);

	uint32_t conversion;

	HAL_ADC_Start(hadc);

	// Wait for the conversion to complete
	HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);

	// Get the ADC value
	conversion = HAL_ADC_GetValue(hadc);

	return conversion;
}


// Update sensors

void run_calibration() {
    update_minmax(&throttle1);
    update_minmax(&throttle2);
    update_minmax(&brake);
}

void update_sensor_vals(ADC_HandleTypeDef *hadc1, ADC_HandleTypeDef *hadc3) {
    throttle1.raw = get_adc_conversion(hadc1, APPS1);
    update_percent(&throttle1);
    throttle2.raw = get_adc_conversion(hadc3, APPS2);
    update_percent(&throttle2);
    brake.raw = get_adc_conversion(hadc3, BSE);
    update_percent(&brake);
    torque_percentage = get_adc_conversion(hadc1, KNOB2) * 100 / 4095;

//    char * str;
//    	  sprintf(str, "min1: %ld, max1: %ld, min2: %ld, max2: %ld, minb: %ld, maxb: %ld", throttle1.min, throttle1.max, throttle2.min, throttle2.max, brake.min, brake.max);
//    	  UG_PutColorString(2, 3, str, C_GREEN, C_BLACK);

    /*
     * T.4.2.5 in FSAE 2022 rulebook
     * If an implausibility occurs between the values of the APPSs and
     * persists for more than 100 msec, the power to the Motor(s) must
     * be immediately stopped completely.
     *
     * It is not necessary to Open the Shutdown Circuit, the motor
     * controller(s) stopping the power to the Motor(s) is sufficient.
     */
    if (has_discrepancy()) {
        discrepancy_timer_ms += TMR1_PERIOD_MS;
        if (discrepancy_timer_ms > MAX_DISCREPANCY_MS && state == DRIVE) {
            report_fault(SENSOR_DISCREPANCY);
        }
    } else {
        discrepancy_timer_ms = 0;
    }
}

uint16_t requested_throttle(){
    temp_attenuate();


    uint32_t throttle = ((uint32_t)throttle2.percent * MAX_TORQUE) / 100;  //upscale for MC code

    throttle = (throttle * THROTTLE_MULTIPLIER) / 100;       //attenuate for temperature
    throttle = throttle * torque_percentage / 100;

    if (throttle >= 5.0) {			//case 1: if the pedal is actually being pressed return on a 1:1 scale
    	return (uint16_t)throttle;
    } else {						//case 2: if we don't know if it's being pressed or just car shaking
    	return (uint16_t)throttle / 2;	//return on 1:1/2 scale
    }
	return 1;
}

void temp_attenuate() {
    int t = PACK_TEMP - 50;
    if (t < 0) {
        THROTTLE_MULTIPLIER = 100;
    } else if (t < 8) {
        THROTTLE_MULTIPLIER = THROTTLE_MAP[t];
    } else if (t >= 8) {
        THROTTLE_MULTIPLIER = THROTTLE_MAP[7];
    }
}

bool sensors_calibrated(){
    if(throttle2.range < APPS1_MIN_RANGE) return 0;
    if(brake.range < BRAKE_MIN_RANGE) return 0;

    return 1;
}

bool braking(){
    return brake.raw > BRAKE_LIGHT_THRESHOLD;
}

bool brake_mashed(){
    return brake.raw > RTD_BRAKE_THRESHOLD;
}

// check differential between the throttle sensors
// returns true only if the sensor discrepancy is > 10%
// Note: after verifying there's no discrepancy, can use either sensor(1 or 2) for remaining checks
bool has_discrepancy() {
    if(abs((int)throttle1.percent - (int)throttle2.percent) > 10) return 1;  //percentage discrepancy

    return (throttle1.raw < APPS_OPEN_THRESH)
        || (throttle1.raw > APPS_SHORT_THRESH)
        || (throttle2.raw < APPS_OPEN_THRESH)
        || (throttle2.raw > APPS_SHORT_THRESH);   //wiring fault
	return false;

}

// check for soft BSPD
// see EV.5.7 of FSAE 2022 rulebook
bool brake_implausible() {
    if (error == BRAKE_IMPLAUSIBLE) {
        // once brake implausibility detected,
        // can only revert to normal if throttle unapplied
        return !(throttle1.percent <= APPS1_BSPD_RESET);
    }

    // if both brake and throttle applied, brake implausible
    //return (temp_brake > 0 && temp_throttle > throttle_range * 0.25);
    return (brake.raw >= BRAKE_BSPD_THRESHOLD && throttle1.percent > APPS1_BSPD_THRESHOLD);
	return false;
}

void update_percent(CALIBRATED_SENSOR_t* sensor){
    uint32_t raw = (uint32_t)clamp(sensor->raw, sensor->min, sensor->max);
    sensor->percent = (uint16_t)((100*(raw-sensor->min))/((sensor->range)));
}

void update_minmax(CALIBRATED_SENSOR_t* sensor){
    if (sensor->raw > sensor->max) sensor->max = sensor->raw;
    else if (sensor->raw < sensor->min) sensor->min = sensor->raw;
    if(sensor->max > sensor->min) sensor->range = sensor->max - sensor->min;
}

void add_apps_deadzone(){
	add_deadzone(&throttle1, 5);
	add_deadzone(&throttle2, 5);
	add_deadzone(&brake, 10);
}

void add_deadzone(CALIBRATED_SENSOR_t* sensor, uint16_t deadzone_percentage){
	uint16_t deadzone = sensor->range * deadzone_percentage / 100;

	// catch funky cases that would end up with a negative or 0 range
	if(deadzone >= sensor->range) return;

	sensor->min += deadzone;
	sensor->range -= deadzone;
}

uint16_t clamp(uint16_t in, uint16_t min, uint16_t max){
    if(in > max) return max;
    if(in < min) return min;
    return in;
}



/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "inttypes.h"
#include "can_manager.h"
#include "sensors.h"
#include "fsm.h"
#include "traction_control.h"
#include "frucd_display.h"
#include "sd_card.h"
#include "telem.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc3;

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart7;
UART_HandleTypeDef huart3;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */

// Keeps track of timer waiting for pre-charging
volatile unsigned int precharge_timer_ms = 0;
volatile uint8_t init_fault_cleared = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN2_Init(void);
static void MX_SDMMC1_SD_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC3_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
static void MX_UART4_Init(void);
static void MX_UART7_Init(void);
static void MX_FMC_Init(void);
static void MX_CAN1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/************ Switches ************/

/*
 * global variable for driver switch and button status
 *
 * format: 0000 0[TC][Hv][Dr]
 * bit is high if corresponding switch is on, low otherwise
 *
 * use in conditions:
 *  - TC = switches & 0b100
 *  - Hv = switches & 0b10
 *  - Dr = switches & 0b1
 */

uint8_t traction_control_enable() {
	return HAL_GPIO_ReadPin(GPIOG, BUTTON_1_Pin);
}

uint8_t display_debug_enabled() {
	return HAL_GPIO_ReadPin(GPIOG, BUTTON_2_Pin);
}

uint8_t hv_switch() {
	return !HAL_GPIO_ReadPin(GPIOG, HV_REQUEST_Pin);
}

uint8_t drive_switch() {
	return !HAL_GPIO_ReadPin(GPIOG, DRIVE_REQUEST_Pin);
}


uint8_t shutdown_closed() {
    if (estop_flags) return 0;
    return (shutdown_flags & 0b00111000) == 0b00111000;
}

// TEST

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  // Check which version of the timer triggered this callback and toggle LED
  if (htim == &htim7)
  {
	  if (state == PRECHARGING) {
		  precharge_timer_ms += TMR1_PERIOD_MS;
		  if (precharge_timer_ms > PRECHARGE_TIMEOUT_MS) {
			  report_fault(CONSERVATIVE_TIMER_MAXED);
		  }
	  } else {
		  precharge_timer_ms = 0;
	  }
  }
}

uint8_t splashscreen[] = {255, 255, 175, 66, 255, 241, 86, 255, 131, 66, 255, 241, 86, 255, 131, 66, 255, 203, 0, 164, 86, 255, 131, 66, 236, 73, 211, 1, 164, 67, 255, 150, 66, 232, 79, 209, 0, 165, 67, 255, 150, 66, 230, 83, 206, 1, 165, 67, 255, 150, 66, 229, 85, 204, 1, 166, 67, 255, 150, 66, 227, 70, 139, 70, 201, 2, 166, 67, 255, 150, 66, 226, 69, 143, 69, 199, 2, 167, 67, 255, 150, 66, 225, 68, 146, 68, 198, 3, 167, 67, 255, 150, 66, 225, 67, 149, 67, 197, 2, 168, 67, 255, 150, 66, 224, 67, 150, 68, 195, 2, 169, 67, 255, 150, 66, 224, 67, 151, 67, 194, 3, 169, 67, 159, 73, 149, 69, 140, 71, 135, 72, 139, 67, 141, 67, 136, 66, 141, 73, 199, 67, 153, 67, 192, 3, 170, 67, 157, 77, 139, 67, 130, 70, 132, 66, 130, 74, 132, 75, 138, 67, 141, 67, 136, 66, 138, 77, 198, 66, 154, 67, 191, 4, 170, 67, 155, 80, 139, 66, 128, 72, 132, 66, 129, 76, 130, 77, 137, 67, 141, 67, 136, 66, 136, 80, 196, 67, 155, 66, 190, 4, 171, 84, 137, 69, 135, 68, 138, 72, 136, 71, 133, 73, 134, 68, 136, 67, 141, 67, 136, 66, 136, 69, 134, 68, 195, 66, 156, 67, 188, 5, 171, 85, 135, 68, 138, 68, 137, 69, 139, 69, 136, 71, 136, 67, 136, 67, 141, 67, 136, 66, 137, 65, 138, 67, 195, 66, 157, 66, 188, 4, 172, 85, 134, 68, 141, 67, 136, 68, 140, 68, 138, 69, 138, 66, 136, 67, 141, 67, 136, 66, 151, 67, 194, 66, 157, 66, 187, 5, 172, 85, 134, 67, 142, 67, 136, 67, 141, 68, 138, 68, 139, 67, 135, 67, 141, 67, 136, 66, 151, 67, 193, 67, 157, 66, 186, 5, 173, 84, 134, 67, 144, 67, 135, 67, 141, 67, 139, 68, 139, 67, 135, 67, 141, 67, 136, 66, 152, 66, 193, 67, 157, 66, 185, 6, 173, 67, 151, 67, 144, 67, 135, 67, 141, 67, 139, 67, 140, 67, 135, 67, 141, 67, 136, 66, 152, 66, 193, 66, 158, 66, 184, 6, 174, 67, 151, 66, 146, 66, 135, 66, 142, 66, 141, 66, 140, 67, 135, 67, 141, 67, 136, 66, 152, 66, 193, 66, 158, 66, 183, 7, 174, 67, 150, 67, 146, 67, 134, 66, 142, 66, 141, 66, 141, 66, 135, 67, 141, 67, 136, 66, 143, 75, 193, 66, 158, 66, 182, 7, 175, 67, 150, 67, 146, 67, 134, 66, 142, 66, 141, 66, 141, 66, 135, 67, 141, 67, 136, 66, 139, 79, 193, 66, 158, 66, 181, 8, 175, 67, 150, 67, 146, 67, 134, 66, 142, 66, 141, 66, 141, 66, 135, 67, 141, 67, 136, 66, 137, 81, 193, 66, 158, 66, 181, 7, 176, 67, 150, 67, 146, 67, 134, 66, 142, 66, 141, 66, 141, 66, 135, 67, 141, 67, 136, 66, 136, 70, 136, 66, 193, 66, 158, 66, 180, 8, 176, 67, 150, 67, 146, 67, 134, 66, 142, 66, 141, 66, 141, 66, 135, 67, 141, 67, 136, 66, 135, 68, 139, 66, 193, 66, 158, 66, 179, 8, 177, 67, 151, 66, 146, 66, 135, 66, 142, 66, 141, 66, 141, 66, 135, 67, 141, 67, 136, 66, 135, 67, 140, 66, 193, 100, 178, 9, 177, 67, 151, 67, 144, 67, 135, 66, 142, 66, 141, 66, 141, 66, 135, 67, 141, 67, 136, 66, 135, 66, 140, 67, 193, 100, 177, 9, 178, 67, 151, 67, 144, 67, 135, 66, 142, 66, 141, 66, 141, 66, 135, 67, 140, 68, 136, 66, 134, 67, 140, 67, 193, 100, 176, 10, 178, 67, 152, 67, 142, 67, 136, 66, 142, 66, 141, 66, 141, 66, 135, 67, 140, 68, 136, 66, 134, 67, 140, 67, 193, 100, 175, 10, 179, 67, 152, 68, 141, 67, 136, 66, 142, 66, 141, 66, 141, 66, 136, 67, 138, 69, 136, 66, 134, 67, 139, 68, 255, 150, 10, 179, 67, 153, 68, 139, 67, 137, 66, 142, 66, 141, 66, 141, 66, 136, 67, 137, 70, 136, 66, 135, 67, 137, 69, 255, 149, 10, 180, 67, 154, 68, 136, 68, 138, 66, 142, 66, 141, 66, 141, 66, 137, 68, 134, 71, 136, 66, 135, 68, 135, 70, 255, 148, 11, 180, 67, 155, 81, 138, 66, 142, 66, 141, 66, 141, 66, 137, 78, 128, 67, 136, 66, 136, 82, 255, 147, 11, 181, 67, 156, 78, 140, 66, 142, 66, 141, 66, 141, 66, 138, 76, 129, 67, 136, 66, 137, 76, 129, 66, 255, 146, 12, 181, 67, 159, 74, 141, 66, 142, 66, 141, 66, 141, 66, 140, 72, 131, 67, 136, 66, 139, 72, 131, 66, 255, 145, 12, 255, 255, 255, 130, 11, 255, 255, 213, 64, 171, 12, 134, 72, 255, 255, 165, 65, 155, 67, 169, 12, 133, 76, 255, 255, 162, 67, 154, 67, 168, 13, 132, 78, 255, 255, 161, 66, 156, 67, 166, 13, 132, 80, 255, 255, 160, 66, 156, 67, 165, 14, 130, 83, 255, 255, 158, 67, 157, 66, 164, 14, 131, 84, 255, 255, 157, 66, 158, 67, 163, 14, 130, 85, 255, 255, 157, 66, 158, 67, 162, 14, 131, 86, 255, 255, 156, 66, 158, 67, 161, 15, 131, 86, 255, 255, 156, 66, 158, 67, 160, 16, 130, 87, 255, 255, 156, 66, 158, 67, 159, 17, 130, 87, 255, 255, 156, 66, 158, 67, 159, 17, 130, 87, 255, 255, 156, 66, 158, 67, 158, 18, 130, 87, 255, 255, 156, 66, 158, 67, 157, 19, 130, 87, 255, 255, 156, 67, 157, 67, 158, 18, 131, 86, 255, 255, 156, 67, 157, 66, 159, 19, 130, 86, 137, 63, 63, 63, 63, 16, 130, 66, 156, 67, 160, 18, 131, 84, 138, 63, 63, 63, 63, 16, 130, 67, 155, 67, 160, 18, 131, 84, 138, 63, 63, 63, 63, 16, 131, 66, 154, 67, 162, 18, 131, 82, 139, 63, 63, 63, 63, 16, 131, 67, 153, 67, 163, 17, 132, 80, 140, 63, 63, 63, 63, 16, 132, 67, 151, 67, 164, 18, 132, 78, 255, 255, 163, 68, 149, 68, 165, 18, 132, 76, 255, 255, 165, 68, 147, 68, 166, 19, 133, 72, 255, 255, 168, 69, 143, 69, 168, 19, 255, 255, 183, 70, 139, 70, 169, 21, 255, 255, 182, 73, 133, 71, 171, 20, 149, 127, 127, 127, 127, 87, 138, 84, 172, 19, 150, 127, 127, 127, 127, 87, 140, 80, 175, 17, 151, 127, 127, 127, 127, 87, 143, 75, 177, 16, 255, 255, 255, 15, 255, 255, 255, 14, 255, 255, 255, 14, 255, 255, 255, 128, 13, 255, 255, 255, 128, 14, 255, 255, 255, 128, 13, 255, 255, 255, 128, 13, 255, 255, 255, 128, 13, 255, 255, 184, 92, 171, 12, 255, 255, 185, 95, 167, 13, 255, 255, 185, 96, 166, 12, 255, 255, 213, 70, 164, 12, 255, 255, 217, 68, 163, 11, 255, 255, 219, 68, 161, 12, 255, 255, 220, 67, 161, 11, 255, 255, 222, 67, 159, 11, 255, 255, 223, 67, 158, 11, 255, 166, 81, 255, 168, 66, 158, 11, 255, 166, 83, 192, 67, 225, 67, 157, 10, 255, 167, 85, 189, 68, 225, 67, 156, 10, 255, 168, 67, 138, 71, 188, 68, 225, 67, 155, 10, 255, 169, 67, 141, 68, 189, 67, 225, 67, 155, 9, 255, 170, 67, 142, 68, 255, 162, 67, 154, 10, 255, 170, 67, 143, 67, 255, 162, 67, 154, 9, 255, 171, 67, 143, 67, 255, 162, 67, 153, 9, 255, 172, 67, 143, 68, 255, 161, 66, 158, 4, 255, 173, 67, 143, 68, 255, 160, 67, 161, 0, 255, 174, 67, 143, 68, 255, 159, 67, 255, 210, 67, 143, 67, 255, 160, 67, 255, 210, 67, 143, 67, 140, 71, 148, 71, 159, 70, 148, 71, 168, 68, 154, 71, 255, 176, 67, 142, 68, 137, 76, 144, 75, 136, 66, 136, 66, 131, 74, 144, 75, 131, 66, 157, 70, 153, 73, 255, 175, 67, 141, 68, 136, 79, 141, 79, 134, 66, 136, 66, 129, 77, 141, 79, 129, 66, 129, 97, 153, 75, 255, 174, 67, 140, 69, 135, 70, 133, 68, 139, 70, 133, 68, 133, 66, 136, 73, 132, 68, 139, 71, 132, 72, 129, 95, 154, 76, 255, 174, 85, 137, 66, 137, 67, 138, 68, 137, 67, 133, 66, 136, 70, 136, 67, 138, 69, 137, 70, 129, 93, 156, 76, 255, 174, 84, 152, 67, 136, 68, 139, 65, 134, 66, 136, 69, 137, 68, 136, 68, 140, 69, 188, 76, 255, 174, 82, 154, 67, 136, 67, 149, 66, 136, 68, 139, 67, 136, 67, 142, 68, 188, 76, 255, 174, 80, 156, 67, 135, 67, 150, 66, 136, 67, 140, 67, 135, 68, 143, 67, 188, 76, 255, 174, 67, 134, 67, 159, 66, 135, 67, 150, 66, 136, 67, 140, 67, 135, 67, 144, 67, 188, 76, 255, 174, 67, 135, 67, 158, 66, 134, 67, 151, 66, 136, 67, 141, 66, 135, 67, 144, 67, 189, 75, 255, 174, 67, 135, 68, 152, 71, 134, 67, 151, 66, 136, 67, 141, 66, 135, 66, 146, 66, 185, 0, 131, 73, 255, 175, 67, 136, 67, 145, 78, 134, 67, 151, 66, 136, 66, 142, 66, 134, 67, 146, 66, 185, 1, 132, 70, 255, 176, 67, 137, 67, 142, 80, 134, 67, 151, 66, 136, 66, 142, 66, 134, 67, 146, 66, 184, 3, 255, 187, 67, 137, 68, 140, 71, 134, 66, 134, 67, 151, 66, 136, 66, 142, 66, 134, 67, 146, 66, 184, 4, 255, 186, 67, 138, 67, 139, 68, 138, 66, 134, 67, 151, 66, 136, 66, 142, 66, 135, 66, 146, 66, 183, 4, 255, 187, 67, 138, 68, 137, 67, 140, 66, 134, 67, 151, 66, 136, 66, 142, 66, 135, 67, 144, 67, 182, 5, 255, 187, 67, 139, 67, 137, 67, 139, 67, 135, 66, 151, 66, 136, 66, 142, 66, 135, 67, 144, 67, 182, 4, 255, 188, 67, 139, 68, 135, 67, 140, 67, 135, 67, 150, 66, 136, 66, 142, 66, 135, 67, 144, 67, 181, 4, 255, 189, 67, 140, 68, 134, 67, 140, 67, 135, 67, 150, 66, 136, 66, 142, 66, 136, 67, 142, 68, 181, 3, 255, 190, 67, 141, 67, 134, 67, 140, 67, 136, 67, 149, 66, 136, 66, 142, 66, 136, 68, 140, 69, 180, 4, 255, 190, 67, 141, 68, 134, 67, 138, 68, 136, 68, 139, 65, 134, 66, 136, 66, 142, 66, 137, 68, 138, 70, 180, 3, 255, 191, 67, 142, 67, 134, 68, 136, 69, 137, 68, 137, 67, 133, 66, 136, 66, 142, 66, 138, 69, 134, 72, 179, 3, 255, 192, 67, 142, 68, 134, 82, 138, 81, 133, 66, 136, 66, 142, 66, 139, 79, 129, 66, 179, 2, 255, 193, 67, 143, 68, 134, 76, 129, 66, 139, 79, 134, 66, 136, 66, 142, 66, 141, 76, 130, 66, 178, 3, 255, 193, 67, 144, 67, 135, 74, 130, 67, 140, 75, 136, 66, 136, 66, 142, 66, 143, 72, 132, 66, 178, 2, 255, 230, 69, 153, 69, 201, 66, 177, 2, 255, 255, 215, 66, 177, 1, 255, 255, 215, 67, 176, 1, 255, 255, 216, 67, 176, 1, 255, 255, 216, 67, 175, 1, 255, 255, 217, 66, 175, 1, 255, 255, 199, 65, 143, 67, 175, 0, 255, 255, 198, 68, 141, 67, 175, 1, 255, 255, 198, 69, 139, 68, 255, 255, 250, 71, 133, 69, 175, 0, 255, 255, 203, 81, 255, 255, 255, 77, 255, 255, 255, 133, 71, 255, 255, 255, 152};


// TEST END

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN2_Init();
  MX_SDMMC1_SD_Init();
  MX_ADC1_Init();
  MX_ADC3_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_UART4_Init();
  MX_UART7_Init();
  MX_FMC_Init();
  MX_CAN1_Init();
  MX_USART3_UART_Init();
  MX_TIM7_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */

  init_sensors();

  Display_Init();
  UG_FontSelect(&FONT_12X16);
  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_YELLOW);

  Display_CalibrateScreen();

  Display_DriveTemplate();

  if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK) {
      Error_Handler();
  }


  mount_sd_card();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  Display_Update();

	  write_to_sd();

	  telem_send();

	  char sstr[100];
	  sprintf(sstr, "apps1: %d, apps2: %d, bse: %d      ", throttle1.percent, throttle2.percent, brake.percent);
	  UG_PutString(5, 250, sstr);

	  update_sensor_vals(&hadc1, &hadc3);

	  can_tx_vcu_state(&hcan1);

	  can_tx_torque_request(&hcan1);

	  // Traction control
	  if (traction_control_enable()) {
		  traction_control_PID();
	  }

	  // If shutdown circuit opens in any state
	  if (!shutdown_closed()) {
		  report_fault(SHUTDOWN_CIRCUIT_OPEN);
	  }

	  //if hard BSPD trips in any state
 //	  if (!HAL_GPIO_ReadPin(BSPD_LATCH){
 //		  report_fault(HARD_BSPD);
 //	  }
//
//	  if (mc_fault) {
//		  report_fault(MC_FAULT);
//	  }


	  if (!init_fault_cleared) {
		  can_clear_MC_fault(&hcan1);
		  if (mc_fault_clear_success) {
			 init_fault_cleared = 1;
		  }
	  }


//	  if (display_debug_enabled()) {
//		  Display_DebugTemplate();
//	  }
//	  else {
//		  Display_DriveTemplate();
//	  }

	  switch (state) {
		  case STARTUP:
			  run_calibration();

			  if (!hv_switch() && !drive_switch()) {
				  change_state(LV);
			  }
			  break;
		  case LV:
			  run_calibration();

			  if (drive_switch()) {
				  // Drive switch should not be enabled during LV
				  report_fault(DRIVE_REQUEST_FROM_LV);
				  break;
			  }

			  if (hv_switch()) {
				  // HV switch was flipped
				  // check if APPS pedal was calibrated
				  if (sensors_calibrated()) {
					  // Start charging the car to high voltage state
					  add_apps_deadzone();
					  change_state(PRECHARGING);
				  } else {
					  report_fault(UNCALIBRATED);
				  }
			  }

			  break;
		  case PRECHARGING:
//			  if (capacitor_volt > PRECHARGE_THRESHOLD) {

			  // if main AIRs closed
			  if ((shutdown_flags & 0b110) == 0b110) {
				  // Finished charging to HV on time
				  change_state(HV_ENABLED);
				  break;
			  }
			  if (!hv_switch()) {
				  // Driver flipped off HV switch
				  change_state(LV);
				  break;
			  }
			  if (drive_switch()) {
				  // Drive switch should not be enabled during PRECHARGING
				  report_fault(DRIVE_REQUEST_FROM_LV);
				  break;
			  }
			  break;
		  case HV_ENABLED:
			  if (!hv_switch()) {// || capacitor_volt < PRECHARGE_THRESHOLD) { // don't really need volt check by rules
				  // Driver flipped off HV switch
				  change_state(LV);
				  break;
			  }

			  if (drive_switch()) {
				  // Driver flipped on drive switch
				  // Need to press on pedal at the same time to go to drive

//				  if (brake_mashed()) {
					  change_state(DRIVE);
//				  } else {
//					  // Driver didn't press pedal
//					  report_fault(BRAKE_NOT_PRESSED);
//				  }
			  }

			  break;
		  case DRIVE:
			  if (!drive_switch()) {
				  // Drive switch was flipped off
				  // Revert to HV
				  change_state(HV_ENABLED);
				 break;
			  }

			  if (!hv_switch()) {// || capacitor_volt < PRECHARGE_THRESHOLD) { // don't really need volt check by rules || capacitor_volt < PRECHARGE_THRESHOLD) {
				  // HV switched flipped off, so can't drive
				  // or capacitor dropped below threshold
				  report_fault(HV_DISABLED_WHILE_DRIVING);
				  break;
			  }

//			  if (brake_implausible()) {
//				  report_fault(BRAKE_IMPLAUSIBLE);
//			  }

			  break;
		  case FAULT:
			  switch (error) {
				  case BRAKE_NOT_PRESSED:
					  if (!hv_switch())
						  change_state(LV);

					  if (!drive_switch()) {
						  // reset drive switch and try again
						  change_state(HV_ENABLED);
					  }
					  break;
				  case SENSOR_DISCREPANCY:
					  // stop power to motors if discrepancy persists for >100ms
					  // see rule T.4.2.5 in FSAE 2022 rulebook
					  if (!drive_switch()) {
						  discrepancy_timer_ms = 0;
						  change_state(HV_ENABLED);
					  }

					  if (!hv_switch())
						  report_fault(HV_DISABLED_WHILE_DRIVING);

					  break;
				  case BRAKE_IMPLAUSIBLE:
					  if (!brake_implausible() && hv_switch() && drive_switch())
						  change_state(DRIVE);

					  if (!hv_switch() && !drive_switch())
						  change_state(LV);

					  if (!drive_switch())
						  change_state(HV_ENABLED);

					  if (!hv_switch())
						  report_fault(HV_DISABLED_WHILE_DRIVING);

					  break;
				  case SHUTDOWN_CIRCUIT_OPEN:
					  if (shutdown_closed()) {
						  change_state(LV);
					  }
					  break;
				  case HARD_BSPD:
					  //should not be recoverable, but let hardware decide this
 //					  if (!HAL_GPIO_ReadPin(BSPD_LATCH) {
 //						  change_state(LV);
 //			  		  }
					  break;
				  default:  //UNCALIBRATED, DRIVE_REQUEST_FROM_LV, CONSERVATIVE_TIMER_MAXED, HV_DISABLED_WHILE_DRIVING, MC FAULT
					  if (!hv_switch() && !drive_switch()) {
						  change_state(LV);
					  }
					  break;
			  }
			  break;
	 	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
//
  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 2;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 18;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /*##-2- Configure the CAN Filter ###########################################*/
    CAN_FilterTypeDef canfilterconfig;

    canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig.FilterBank = 18;  // which filter bank to use from the assigned ones
    canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    canfilterconfig.FilterIdHigh = 0x0;
    canfilterconfig.FilterIdLow = 0x0;
    canfilterconfig.FilterMaskIdHigh = 0x0;
    canfilterconfig.FilterMaskIdLow = 0x0000;
    canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilterconfig.SlaveStartFilterBank = 20;  // how many filters to assign to the CAN1 (master can)

  	if (HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig) != HAL_OK)
  	{
  	  /* Filter configuration Error */
  	  Error_Handler();
  	}

  	/*##-3- Start the CAN peripheral ###########################################*/
  	if (HAL_CAN_Start(&hcan1) != HAL_OK)
  	{
  	  /* Start Error */
  	  Error_Handler();
  	}

  	/*##-4- Activate CAN RX notification #######################################*/
  	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  	{
  	  /* Notification Error */
  	  Error_Handler();
  	}

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 18;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 8000;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim4, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 8000;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 5000;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */
  NVIC_EnableIRQ(TIM7_IRQn);

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief UART7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART7_Init(void)
{

  /* USER CODE BEGIN UART7_Init 0 */

  /* USER CODE END UART7_Init 0 */

  /* USER CODE BEGIN UART7_Init 1 */

  /* USER CODE END UART7_Init 1 */
  huart7.Instance = UART7;
  huart7.Init.BaudRate = 115200;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_1;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_TX_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_16;
  huart7.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart7.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART7_Init 2 */

  /* USER CODE END UART7_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FMC_NORSRAM_DEVICE;
  hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
  hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
  hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PF2 EXTRA_SENS2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_2|EXTRA_SENS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : EXTRA_SENS_1_Pin */
  GPIO_InitStruct.Pin = EXTRA_SENS_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EXTRA_SENS_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HEARTBEAT_Pin */
  GPIO_InitStruct.Pin = HEARTBEAT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HEARTBEAT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SHORTED_TO_PB11_Pin SHORTED_TO_PB10_Pin BAT_12V_MEASURE_Pin */
  GPIO_InitStruct.Pin = SHORTED_TO_PB11_Pin|SHORTED_TO_PB10_Pin|BAT_12V_MEASURE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BUTTON_4_Pin BUTTON_3_Pin BUTTON_2_Pin BUTTON_1_Pin
                           HV_REQUEST_Pin DRIVE_REQUEST_Pin GASP_INTERRUPT_Pin */
  GPIO_InitStruct.Pin = BUTTON_4_Pin|BUTTON_3_Pin|BUTTON_2_Pin|BUTTON_1_Pin
                          |HV_REQUEST_Pin|DRIVE_REQUEST_Pin|GASP_INTERRUPT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  UG_PutString(5, 250, "HAL ERROR");
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

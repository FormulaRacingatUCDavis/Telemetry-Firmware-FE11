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
#include "fatfs_sd.h"
#include "string.h"
#include "stdio.h"
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

CAN_HandleTypeDef hcan;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;


/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//---------------------------------CAN----------------------------
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;

uint8_t TxData[8];
uint8_t RxData[8];

uint32_t TxMailbox;

int datacheck = 0;
//----------------------------------------------------------------


////---------------------------------SD Card----------------------------
FATFS fs;// file system
FIL fil; // file
FRESULT fresult; //to store the result
char buffer[8192]; //to store data
UINT br, bw; // file read/write count

int num = 0;

/* capacity related variables */
FATFS *pfs;
DWORD fre_clust;
uint32_t total, free_space;

/* to send the data to the uart */
void send_UART (char *string)
{
	uint8_t len = strlen(string);
		HAL_UART_Transmit(&huart1, (uint8_t*) string, len, 2000); //transmit in blocking mode
}

/*to find the size of data in the buffer */
int bufsize(char *buf)
{
	int i = 0;
	while(*buf++ != '\0')
		i++;
	return i;
}

void bufclear(void) //clear buffer
{
	for(int i = 0; i<1024; i++){
		buffer[i] = '\0';
	}
}

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
  MX_ADC1_Init();
  MX_CAN_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */


  char adcStr[30];
  char Max[30];
  char Min[30];
  uint32_t adcValue;
  uint32_t startTime = HAL_GetTick();

  ////////////////////////////////////////////////////* Mount SD Card */////////////////////////////////////////////////////////////////
  fresult = f_mount(&fs, "", 0);

  	  	  	  	  	  	  	  	  	  	  	  	  	  	//  uint32_t TimeRightAfter_Mount = HAL_GetTick();


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  	/////////////////////*The following operation is using PUTS and GETS *///////////////////////////////////

  	  	  /* Open file to write/create a file if it doesn't exist */
  	//  	  fresult = f_open(&fil, "file1.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  	  	  //FA_OPEN_ALWAYS = create file
  	  	  //FA_READ = read file
  	  	  //FA_WRITE = write file


  	  	  /*Writing text*/
  	//  	  fresult = f_puts("This is written by Mark\n\n", &fil);

  	  	  /*Close file*/
  	//  	  fresult = f_close(&fil); //close the file
  	//  	  send_UART("File1.txt created and the data is written \n"); //then send the file through UART

  	  	  /*Open file to read */
  	//  	  fresult = f_open(&fil, "file1.txt", FA_READ); //Open file to Read

  	  	  /*Read String from the file*/
  	//  	  f_gets(buffer, fil.fsize, &fil); //Read Strings in the file

  	//  	  send_UART(buffer);

  	  	  /* Close file */
  	 // 	  f_close(&fil); //close file
  	 // 	  bufclear(); //clear


  	  	/////////////////////*The following operation is using f_write and f_read *///////////////////////////////////
/////////////////////////////////////////////////////////Creat ADC File//////////////////////////////////
  	  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  	  	  /*Creat adcData file with write access and open it */
  //	  	  fresult = f_open(&fil, "adcData.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

  	  	  /*Writing text */
//  	  	  strcpy (buffer, "This file is for saving adcData\n");

 // 	  	  fresult = f_write(&fil, buffer, bufsize(buffer), &bw);
  	  //	uint32_t Time = HAL_GetTick();
//  	  	  send_UART("adcDafa file successfully created\n");
  	  	  //uint32_t Time = HAL_GetTick();
  	  	 // sprintf(buffer,  "Time = %hu\r\n", Time);
  	  	//  send_UART( buffer);
  	  	  /*Close file*/
 // 	  	  f_close(&fil);
 // 	  	  bufclear(); //clear buffer to show that result obtained is from the file

  	  	  /*Open second file to read */
  	  //	  fresult = f_open(&fil, "adcData.txt", FA_READ);

  	  	  /*Read data from the file*/
  	  	//  f_read(&fil, buffer, fil.fsize, &br);
  	  	 // send_UART(buffer);

  	  	  /*close file*/
  	  	 // f_close(&fil);
  	  	 // bufclear();
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////CAN START///////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   //CAN Start
   HAL_CAN_Start(&hcan);
   //Activate the Notification
   HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
   ////////////////////////////////////////////////////////////////////////////////


   ////////////////////////////////////////////////Creat CAN File/////////////////////////////////////////////////
     	  ////////////////////////////////////////////////////////////////////////////////////////////////////////
   /*Creat CANData file with write access and open it */
   	  	  fresult = f_open(&fil, "CANData.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
   	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  //uint32_t TimeRightAfter_Creat_File = HAL_GetTick();
   	  	  /*Writing text */
   	  	  strcpy (buffer, "This file is for saving CAN Data\n");
   	  	  fresult = f_write(&fil, buffer, bufsize(buffer), &bw);
   	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  //uint32_t TimeRightAfter_Write_FirstLine = HAL_GetTick();
   	  //	  send_UART("CAN Data file successfully created\n");

   	  	  /*Close file*/
   	  	  f_close(&fil);
   	  	  bufclear(); //clear buffer to show that result obtained is from the file
	  	  	  	  	  	  	  	  //uint32_t TimeRightAfter_file_Job = HAL_GetTick();

   //-----------------------------------SD Card Functions-----------------------------

    	  	/////////////////////*Updating an existing file *///////////////////////////////////
  	              /*Open the file with write access*/
  	    	  	//  fresult = f_open(&fil, "file2.txt", FA_OPEN_ALWAYS | FA_WRITE);

  	    	  	  /*Move to offset to the end of the file*/
  	    	  //	  fresult = f_lseek(&fil, fil.fsize);

  	    	  	  /*write the string to the file*/
  	    	  //	  fresult = f_puts("11:26pm This is from SD with ADC file \n", &fil);
  	    	  //	  f_close(&fil);

  	    	  	  /*open to read the file*/
  	    	  //	  fresult = f_open(&fil, "file2.txt", FA_READ);

  	    	  	  /*Read String from the file */
  	    	  //	  f_read(&fil, buffer, fil.fsize, &br);
  	    	  //	  send_UART(buffer);

  	    	  	  /*close file*/
  	    	  //	  f_close(&fil);
  	    	  //	  bufclear();

 /*
   	  	 //ADC Saving
  	  	void SavingADCData(uint32_t adcValue, int num){
  	  		char buffer[1024];
  	  	  //Open the file with write access//
  	  	  fresult = f_open(&fil, "adcData.txt", FA_OPEN_ALWAYS | FA_WRITE);

  	  	  //Move to offset to the end of the file//
  	  	  fresult = f_lseek(&fil, fil.fsize);

  	  	//write the string to the file//
  	  	  if(adcValue == 4095){
  	  	  	  sprintf(buffer, "%d. Hitting Max = %hu\r\n", num, adcValue);
  	  	  	  fresult = f_puts(buffer, &fil);
  	  	  }
  	  	  else if(adcValue == 0){
  	  		  sprintf(buffer, "%d. Hitting Min = %hu\r\n", num, adcValue);
  	  		  fresult = f_puts(buffer, &fil);
  	  	  }
  	  	  else{
  	  	  sprintf(buffer, "%d. adcValue = %hu\r\n", num, adcValue);
  	  	  fresult = f_puts(buffer, &fil);
  	  	  }
  	  	  //close file//
  	  	  f_close(&fil);

  	  	  //open to read the file//
  	  	  //fresult = f_open(&fil, "file2.txt", FA_READ);

  	  	  //Read String from the file //
  	  	  //f_read(&fil, buffer, sizeof(buffer), &br);
  	  	  //send_UART(buffer);

  	  	  //close file//
  	  	  //f_close(&fil);
  	  	  bufclear();
  	  	}
*/

  	  	/////////////////////* Remove file from the directory *///////////////////////////////////

  	  	 //fresult = f_unlink("/file1.txt");
  	  	 //if(fresult ==FR_OK) send_UART("filel.txt removed successfully...\n");

  	  	 //fresult = f_unlink("/adcData.txt");
  	  	  //if(fresult == FR_OK) send_UART("adcData.txt removed successfully...\n");

  	  	  /*Unmount SDCARD */
  	  	 // fresult = f_mount(NULL, "", 1);
  	  	 // if(fresult ==FR_OK) send_UART ("SD CARD UNMOUNTED successfully...\n");


 //-----------------------------------ADC Function-----------------------------
/*
  void ADC(void){

  	  //////////////////////ADC///////////////////////
  	      // ADC Start//
  	  	  HAL_ADC_Start(&hadc1);

  	  	  // Wait for the conversion to complete
  	  	  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);

  	  	  // Get the ADC value
  	  	  adcValue = HAL_ADC_GetValue(&hadc1);

  	  	  // Print ADC value
  	  	  if(adcValue != 4095 && adcValue != 0){
  	  	      sprintf(adcStr, "%d. adcValue = %hu\r\n", num, adcValue);
  		      send_UART(adcStr);
  		      //SavingADCData(adcValue,adcStr);
  	  	  }
  	  	  else if(adcValue == 4095){
  			  sprintf(Max, "%d. Hitting Max = %hu\r\n", num, adcValue);
  			  send_UART(Max);

  		  }
  	  	  else if(adcValue == 0){
  			  sprintf(Min, "%d. Hitting Min = %hu\r\n", num, adcValue);
  			  send_UART(Min);

  		  }
  	  	  SavingADCData(adcValue, num);
  }
		*/
//--------------------------------------------------------------------------------------------------------------


//-----------------------------------CAN Function-----------------------------
  	  //Sample Data1
  	  	void TxSetup1(void){
  	     TxHeader.DLC = 2; //data length
  	     TxHeader.IDE = CAN_ID_STD;
  	     TxHeader.RTR = CAN_RTR_DATA;
  	     TxHeader.StdId = 0x446; //ID

  	     TxData[0] = 100; //ms Delay
  	     TxData[1] = 10; //loop repeat
  	   }
  	  //Sample Data2
  	   void TxSetup2(void){
  	   	 TxHeader.DLC = 2; //data length
  	   	 TxHeader.IDE = CAN_ID_STD;
  	   	 TxHeader.RTR = CAN_RTR_DATA;
  	   	 TxHeader.StdId = 0x123; //ID

  	   	  TxData[0] = 100; //ms Delay
  	   	  TxData[1] = 10; //loop repeat

  	   }

  	void CAN(void){

		  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET){
			  TxSetup1();
			  HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
			  HAL_Delay(30);
			  }
		  else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET){
				  TxSetup2();
				 HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
				 HAL_Delay(30);
			  }
			  if(TxHeader.StdId == 0x446){
				  HAL_GPIO_WritePin (GPIOB, GPIO_PIN_10, GPIO_PIN_SET); //LED ON
				  } else {
					  HAL_GPIO_WritePin (GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
				  }


		HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, &RxData);
	  	  	  	  	  	  	  	  uint32_t TimeRightAfter_Get_From_Slave = HAL_GetTick();

		if(RxHeader.StdId ==0xA3){ // When Master get reived the data from Slvae blink the LED



			fresult = f_open(&fil, "CANData.txt", FA_OPEN_ALWAYS | FA_WRITE);
			 		/*Move to offset to the end of the file*/
			fresult = f_lseek(&fil, fil.fsize);
			 					 /*write the string to the file*/
			sprintf(buffer,"%X, %d, %d, %d, %d, %d, %d, %d, %d, %d \n ", RxHeader.StdId, RxData[0], RxData[1], RxData[2], RxData[3], RxData[4], RxData[5], RxData[6], RxData[7], TimeRightAfter_Get_From_Slave );
			 fresult = f_puts(buffer, &fil);
			 	 	 	 	 	 	 	 	 	 	 //uint32_t TimeRightAfter_Write_CANData = HAL_GetTick();
			 	 	 	 	 	 	 	 	 	 	 //sprintf(buffer,"%d\n %d\n", TimeRightAfter_Get_From_Slave, TimeRightAfter_Write_CANData);
			 	 	 	 	 	 	 	 	 	 	 	 //send_UART(buffer);

			 send_UART(buffer);
			 f_close(&fil);
			 bufclear();

			 	 	 	 	 	 	 	 	 	 	 	 // 	 uint32_t TimeRightAfter_Write_CANData = HAL_GetTick();
			 	 	 	 	 	 	 	 	 	 	 	 //	 sprintf(buffer,"%d\n %d\n", TimeRightAfter_Get_From_Slave, TimeRightAfter_Write_CANData);
			 	 	 	 	 	 	 	 	 	 	 	 	 // send_UART(buffer);


			//blink LED
			  for (int i = 0; i<6; i++){
				  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
				  HAL_Delay(20);
			  }
		  }else{
				fresult = f_open(&fil, "CANData.txt", FA_OPEN_ALWAYS | FA_WRITE);
				 		/*Move to offset to the end of the file*/
				fresult = f_lseek(&fil, fil.fsize);
				 					 /*write the string to the file*/
				sprintf(buffer,"%X, %d, %d, %d, %d, %d, %d, %d, %d, %d \n ", RxHeader.StdId, RxData[0], RxData[1], RxData[2], RxData[3], RxData[4], RxData[5], RxData[6], RxData[7], TimeRightAfter_Get_From_Slave );
				 fresult = f_puts(buffer, &fil);
				 	 	 	 	 	 	 	 	 	 	 // uint32_t TimeRightAfter_Write_CANData = HAL_GetTick();
				 	 	 	 	 	 	 	 	 	 	 // sprintf(buffer,"%d\n %d\n", TimeRightAfter_Get_From_Slave, TimeRightAfter_Write_CANData);
				 	 	 	 	 	 	 	 	 	 	 // send_UART(buffer);


				 send_UART(buffer);
				 f_close(&fil);
				 bufclear();

				 	 	 	 	 	 	 	 	 	 	 //	 uint32_t TimeRightAfter_Write_CANData = HAL_GetTick();
				 	 	 	 	 	 	 	 	 	 	 // sprintf(buffer,"%d\n %d\n", TimeRightAfter_Get_From_Slave, TimeRightAfter_Write_CANData);
				 	 	 	 	 	 	 	 	 	 	 // send_UART(buffer);


				 HAL_Delay(10);
		  }


  	}

  	//////////////////////////////////Time Check//////////////////////////
  //	sprintf(buffer,"%d\n %d\n %d\n, %d\n", TimeRightAfter_Mount, TimeRightAfter_Creat_File, TimeRightAfter_Write_FirstLine, TimeRightAfter_file_Job);

	// send_UART(buffer);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  num++;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	 // ADC();

///////////////////////////////////////////////////////////////////////////

	  CAN();



	  	  //CDC_Transmit_FS(adcStr, 10);
	          /* GPIO Output with LED */
	  	  HAL_GPIO_TogglePin (LED_GPIO_Port, LED_Pin);
	  	  HAL_Delay(50);

	  	 if ((HAL_GetTick() - startTime) >= 100000) {
	  	        break;
	  	 }

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 18;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
  CAN_FilterTypeDef canfilterconfig;

     canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
     canfilterconfig.FilterBank = 0; //anything between 0 to SlaveStartFilterBank
     canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0; //Using FIFO#0
     canfilterconfig.FilterIdHigh = 0<<5;
     canfilterconfig.FilterIdLow = 0;
     canfilterconfig.FilterMaskIdHigh = 0 << 5;
     canfilterconfig.FilterMaskIdLow = 0x0000;
     canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK; //or CAN_FILTERMODE_IDLIST mode
     canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT; //
    // canfilterconfig.SlaveStartFilterBank = 12; //13 to 27 are assigned to Slave CAN(CAN2) or 0 to 12 are assigned to Master CAN(CAN1)

     HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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

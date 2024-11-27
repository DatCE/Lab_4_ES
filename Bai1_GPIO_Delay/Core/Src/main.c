/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
#include "uart.h"
#include "ring_buffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define INIT_SYSTEM     0
#define SET_HOUR        1
#define SET_MINUTE      2
#define SET_SEC 		3
#define SET_DAY         4
#define SET_DATE        5
#define SET_MONTH       6
#define SET_YEAR        7
#define MODE_1          0
#define MODE_2          1
#define MODE_3          2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int statusSetupTime = INIT_SYSTEM;
int timeBlink = 0;
int statusSystem = MODE_1;
int pre_hour = 0;
int pre_min = 0;
int pre_sec = 0;
int set_hour = 23;
int set_min = 59;
int set_sec = 59;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void DisplayTime();
void UpdateTime();
unsigned char IsButtonSet();
unsigned char IsButtonMode();
unsigned char IsButtonUp();
unsigned char IsButtonDown();
void SetHour();
void SetMinute();
void SetSecond();
void SetDay();
void SetDate();
void SetMonth();
void SetYear();
void SetUpTime();
void TestUart();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	system_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	lcd_Clear(BLACK);
	UpdateTime();

	while (1) {
		while (!flag_timer2);
		flag_timer2 = 0;
		button_Scan();
        if (statusSystem == MODE_1){
            ds3231_ReadTime();
//            if (ds3231_hours > set_hour)
//            {
//            	HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
//            }
        }
        TestUart();
		if (!isRingBufferEmpty(&buffer)) {
			lcd_ShowIntNum(120, 220, getFromRingBuffer(&buffer), 2, YELLOW, BLACK, 16);
		} else {
			lcd_ShowString(100, 220, "Empty!", WHITE, BLACK, 16, 0);
		}


		/*
		 * TURN OFF DISPLAY TIME
		 *
		DisplayTime();
		SetUpTime();
		*/



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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
	void system_init() {
		HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
		HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
		HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);

		lcd_init();
		ds3231_init();
		uart_init_rs232();

		timer_init();
		setTimer2(50);
	}

	void TestUart() {
		if (button_count[13] == 1) {
			uart_Rs232SendNum(ds3231_hours);
			uart_Rs232SendString((void*)":");
			uart_Rs232SendNum(ds3231_min);
			uart_Rs232SendString((void*)":");
			uart_Rs232SendNum(ds3231_sec);
			uart_Rs232SendString((void*)"\n");
		}
	}

	void UpdateTime(){
		ds3231_Write(ADDRESS_YEAR, 24);
		ds3231_Write(ADDRESS_MONTH, 11);
		ds3231_Write(ADDRESS_DATE, 15);
		ds3231_Write(ADDRESS_DAY, 6);
		ds3231_Write(ADDRESS_HOUR, 2);
		ds3231_Write(ADDRESS_MIN, 45);
		ds3231_Write(ADDRESS_SEC, 0);
	}
	void DisplayTime()
	{
        if (ds3231_hours > set_hour || (ds3231_hours == set_hour && ds3231_min > set_min) ||   (ds3231_hours == set_hour && ds3231_min == set_min && ds3231_sec > set_sec))
        {
        	lcd_ShowString(90, 160, "ALARM", YELLOW, BLACK, 24, 0);
        }
        else
        {
        	lcd_ShowString(90, 160, "     ", YELLOW, BLACK, 24, 0);
        }
        lcd_ShowString(20, 190, "SET", GREEN, BLACK, 24, 0);

		lcd_ShowIntNum(70, 190, set_hour/10, 1, GREEN, BLACK, 24);
		lcd_ShowIntNum(83, 190, set_hour%10, 1, GREEN, BLACK, 24);
		lcd_ShowChar(96, 190, ':', GREEN, BLACK, 24, 0);
		lcd_ShowIntNum(110, 190, set_min/10, 1, GREEN, BLACK, 24);
		lcd_ShowIntNum(123, 190, set_min%10, 1, GREEN, BLACK, 24);
		lcd_ShowChar(136, 190, ':', GREEN, BLACK, 24, 0);
		lcd_ShowIntNum(150, 190, set_sec/10, 1, GREEN, BLACK, 24);
		lcd_ShowIntNum(163, 190, set_sec%10, 1, GREEN, BLACK, 24);

        lcd_ShowString(80, 70, "MODE", YELLOW, BLACK, 24, 0);
        lcd_ShowIntNum(130, 70, statusSystem, 1, YELLOW, BLACK, 24);
		if(statusSetupTime == INIT_SYSTEM) ds3231_ReadTime();

		if(statusSetupTime != SET_HOUR || (statusSetupTime == SET_HOUR && timeBlink >= 5)){
			lcd_ShowIntNum(70, 100, ds3231_hours/10, 1, GREEN, BLACK, 24);
			lcd_ShowIntNum(83, 100, ds3231_hours%10, 1, GREEN, BLACK, 24);
		}

		lcd_ShowChar(96, 100, ':', GREEN, BLACK, 24, 0);

		if(statusSetupTime != SET_MINUTE || (statusSetupTime == SET_MINUTE && timeBlink >= 5)){
			lcd_ShowIntNum(110, 100, ds3231_min/10, 1, GREEN, BLACK, 24);
			lcd_ShowIntNum(123, 100, ds3231_min%10, 1, GREEN, BLACK, 24);
		}

		lcd_ShowChar(136, 100, ':', GREEN, BLACK, 24, 0);

		if(statusSetupTime != SET_SEC || (statusSetupTime == SET_SEC && timeBlink >= 5)){
			lcd_ShowIntNum(150, 100, ds3231_sec/10, 1, GREEN, BLACK, 24);
			lcd_ShowIntNum(163, 100, ds3231_sec%10, 1, GREEN, BLACK, 24);
		}

	    //////day
		if(statusSetupTime != SET_DAY || (statusSetupTime == SET_DAY && timeBlink >= 5)){
			switch(ds3231_day)
			{
				case 1:
					lcd_ShowString(20, 130, "SUN", YELLOW, BLACK, 24, 0);
					break;
				case 2:
					lcd_ShowString(20, 130, "MON", YELLOW, BLACK, 24, 0);
					break;
				case 3:
					lcd_ShowString(20, 130, "TUE", YELLOW, BLACK, 24, 0);
					break;
				case 4:
					lcd_ShowString(20, 130, "WED", YELLOW, BLACK, 24, 0);
					break;
				case 5:
					lcd_ShowString(20, 130, "THU", YELLOW, BLACK, 24, 0);
					break;
				case 6:
					lcd_ShowString(20, 130, "FRI", YELLOW, BLACK, 24, 0);
					break;
				case 7:
					lcd_ShowString(20, 130, "SAT", YELLOW, BLACK, 24, 0);
					break;
			}
		}

		if(statusSetupTime != SET_MONTH || (statusSetupTime == SET_MONTH && timeBlink >= 5)){
			switch(ds3231_month)
			{
				case 1:
					lcd_ShowString(105, 130, "JAN", YELLOW, BLACK, 24, 0);
					break;
				case 2:
					lcd_ShowString(105, 130, "FEB", YELLOW, BLACK, 24, 0);
					break;
				case 3:
					lcd_ShowString(105, 130, "MAR", YELLOW, BLACK, 24, 0);
					break;
				case 4:
					lcd_ShowString(105, 130, "APR", YELLOW, BLACK, 24, 0);
					break;
				case 5:
					lcd_ShowString(105, 130, "MAY", YELLOW, BLACK, 24, 0);
					break;
				case 6:
					lcd_ShowString(105, 130, "JUN", YELLOW, BLACK, 24, 0);
					break;
				case 7:
					lcd_ShowString(105, 130, "JUL", YELLOW, BLACK, 24, 0);
					break;
				case 8:
					lcd_ShowString(105, 130, "AUG", YELLOW, BLACK, 24, 0);
					break;
				case 9:
					lcd_ShowString(105, 130, "SEP", YELLOW, BLACK, 24, 0);
					break;
				case 10:
					lcd_ShowString(105, 130, "OCT", YELLOW, BLACK, 24, 0);
					break;
				case 11:
					lcd_ShowString(105, 130, "NOV", YELLOW, BLACK, 24, 0);
					break;
				case 12:
					lcd_ShowString(105, 130, "DEC", YELLOW, BLACK, 24, 0);
					break;
			}
		}
		if(statusSetupTime != SET_DATE || (statusSetupTime == SET_DATE && timeBlink >= 5)){
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
		}
		if(statusSetupTime != SET_YEAR || (statusSetupTime == SET_YEAR && timeBlink >= 5)){
			lcd_ShowIntNum(150, 130, 20, 2, YELLOW, BLACK, 24);
			lcd_ShowIntNum(176, 130, ds3231_year, 2, YELLOW, BLACK, 24);
		}

	}

	void SetUpTime()
	{
        if (statusSystem == MODE_1)
        {
            if (IsButtonMode())
            {
                statusSystem = MODE_2;
                statusSetupTime = SET_HOUR;
            }
        }
        else if (statusSystem == MODE_2)
        {
            if (IsButtonMode())
            {
            	pre_hour = ds3231_hours;
            	pre_min = ds3231_min;
            	pre_sec = ds3231_sec;
                statusSystem = MODE_3;
            }
            else
            {
        	    switch(statusSetupTime)
        	    {
        	        case SET_HOUR:
        	            SetHour();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_MINUTE;
        	            break;
        	        case SET_MINUTE:
        	            SetMinute();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_SEC;
        	            break;
        	        case SET_SEC:
        	        	SetSecond();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_DAY;
        	            break;
        	        case SET_DAY:
        	            SetDay();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_DATE;
        	            break;
        	        case SET_DATE:
        	            SetDate();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_MONTH;
        	            break;
        	        case SET_MONTH:
        	            SetMonth();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_YEAR;
        	            break;
        	        case SET_YEAR:
        	            SetYear();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_HOUR;
        	            break;
        	        default:
        	            statusSetupTime = SET_HOUR;
        	            break;
        	    }
            }

        }
        else if (statusSystem == MODE_3)
        {
            if (IsButtonMode())
            {
                statusSystem = MODE_1;
                set_hour = ds3231_hours;
                set_min = ds3231_min;
                set_sec = ds3231_sec;
                ds3231_Write(ADDRESS_HOUR, pre_hour);
                ds3231_Write(ADDRESS_MIN, pre_min);
                ds3231_Write(ADDRESS_SEC, pre_sec);
                statusSetupTime = INIT_SYSTEM;

            }
            else
            {
        	    switch(statusSetupTime)
        	    {
        	        case SET_HOUR:
        	            SetHour();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_MINUTE;
        	            break;
        	        case SET_MINUTE:
        	            SetMinute();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_SEC;
        	            break;
        	        case SET_SEC:
        	        	SetSecond();
        	            if(IsButtonSet())
        	                statusSetupTime = SET_HOUR;
        	            break;
        	        default:
        	            statusSetupTime = SET_HOUR;
        	            break;
        	    }
            }
        }

	}
	//
    unsigned char IsButtonSet()
	{
	    if (button_count[12] == 1)
	        return 1;
	    else
	        return 0;
	}
	unsigned char IsButtonMode()
	{
	    if (button_count[14] == 1)
	        return 1;
	    else
	        return 0;
	}

	unsigned char IsButtonUp()
	{
	    if ((button_count[3] == 1) || (button_count[3] >= 10 && button_count[3]%4 == 0))
	        return 1;
	    else
	        return 0;
	}

	unsigned char IsButtonDown()
	{
	    if ((button_count[7] == 1) || (button_count[7] >= 10 && button_count[7]%4 == 0))
	        return 1;
	    else
	        return 0;
	}

	void SetHour()
	{
//		HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 1);
	    timeBlink = (timeBlink + 1)%20;
	    if(timeBlink < 5)
	    	lcd_ShowString(70, 100, "  ", GREEN, BLACK, 24, 0);
	    if(IsButtonUp())
	    {
	        ds3231_hours++;
	        if(ds3231_hours > 23)
	            ds3231_hours = 0;
	        ds3231_Write(ADDRESS_HOUR, ds3231_hours);
	    }
	    if(IsButtonDown())
	    {
	        ds3231_hours--;
	        if(ds3231_hours < 0)
	            ds3231_hours = 23;
	        ds3231_Write(ADDRESS_HOUR, ds3231_hours);
	    }
	}

	void SetMinute()
	{
		timeBlink = (timeBlink + 1)%20;
	    if(timeBlink < 5)
	    	lcd_ShowString(110, 100, "  ", GREEN, BLACK, 24, 0);
	    if(IsButtonUp())
	    {
	        ds3231_min++;
	        if(ds3231_min > 59)
	            ds3231_min = 0;
	        ds3231_Write(ADDRESS_MIN, ds3231_min);
	    }
	    if(IsButtonDown())
	    {
	    	ds3231_min--;
	        if(ds3231_min < 0)
	        	ds3231_min = 59;
	        ds3231_Write(ADDRESS_MIN, ds3231_min);
	    }

	}
	void SetSecond()
	{
		timeBlink = (timeBlink + 1)%20;
	    if(timeBlink < 5)
	    	lcd_ShowString(150, 100, "  ", GREEN, BLACK, 24, 0);
	    if(IsButtonUp())
	    {
	    	ds3231_sec++;
	        if(ds3231_sec > 59)
	        	ds3231_sec = 0;
	        ds3231_Write(ADDRESS_SEC, ds3231_sec);
	    }
	    if(IsButtonDown())
	    {
	    	ds3231_sec--;
	        if(ds3231_sec < 0)
	        	ds3231_sec = 59;
	        ds3231_Write(ADDRESS_SEC, ds3231_sec);
	    }

	}

	void SetDay()
	{
		timeBlink = (timeBlink + 1)%20;
	    if(timeBlink < 5)
	    	lcd_ShowString(20, 130, "  ", GREEN, BLACK, 24, 0);
	    if(IsButtonUp())
	    {
	    	ds3231_day++;
	        if(ds3231_day > 7)
	        	ds3231_day = 1;
	        ds3231_Write(ADDRESS_DAY, ds3231_day);
	    }
	    if(IsButtonDown())
	    {
	    	ds3231_day--;
	        if(ds3231_day < 1)
	        	ds3231_day = 7;
	        ds3231_Write(ADDRESS_DAY, ds3231_day);
	    }
	}

	void SetDate()
	{
		timeBlink = (timeBlink + 1)%20;
	    if(timeBlink < 5)
	    	lcd_ShowString(70, 130, "  ", GREEN, BLACK, 24, 0);
	    if(IsButtonUp())
	    {
	    	ds3231_date++;
	        if(ds3231_date > 31)
	        	ds3231_date = 1;
	        ds3231_Write(ADDRESS_DATE, ds3231_date);
	    }
	    if(IsButtonDown())
	    {
	    	ds3231_date--;
	        if(ds3231_date < 1)
	        	ds3231_date = 31;
	        ds3231_Write(ADDRESS_DATE, ds3231_date);
	    }
	}

	void SetMonth()
	{
		timeBlink = (timeBlink + 1)%20;
	    if(timeBlink < 5)
	    	lcd_ShowString(105, 130, "  ", GREEN, BLACK, 24, 0);
	    if(IsButtonUp())
	    {
	    	ds3231_month++;
	        if(ds3231_month > 12)
	        	ds3231_month = 1;
	        ds3231_Write(ADDRESS_MONTH, ds3231_month);
	    }
	    if(IsButtonDown())
	    {
	    	ds3231_month--;
	        if(ds3231_month < 1)
	        	ds3231_month = 12;
	        ds3231_Write(ADDRESS_MONTH, ds3231_month);
	    }
	}

	void SetYear()
	{
		timeBlink = (timeBlink + 1)%20;
	    if(timeBlink < 5)
	    	lcd_ShowString(176, 130, "  ", GREEN, BLACK, 24, 0);
	    if(IsButtonUp())
	    {
	    	ds3231_year++;
	        if(ds3231_year > 99)
	        	ds3231_year = 0;
	        ds3231_Write(ADDRESS_YEAR, ds3231_year);
	    }
	    if(IsButtonDown())
	    {
	    	ds3231_year--;
	        if(ds3231_year < 0)
	        	ds3231_year = 99;
	        ds3231_Write(ADDRESS_YEAR, ds3231_year);
	    }
	}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

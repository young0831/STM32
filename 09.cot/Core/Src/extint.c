/*
 * extint.c
 *
 *  Created on: Apr 8, 2025
 *      Author: user
 */
#include "extint.h"
#include "stepmotor.h"
volatile uint8_t stepmotor_state;
volatile uint8_t pin_state = 0; // interrupt가 발생했을 때 차광막이 있으면 pinstate = 1, 차광막이 없으면 pinstate = 0
volatile uint8_t current_state; // interrupt가 idle상태로 도달한 층수를 표현하기 위한 변수
/**
 * move from Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xcx_hal_gpio.c to here
 * external INT callback function
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
	case GPIO_PIN_0: //PC0 / BTN0
		pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
			if (pin_state) // LOW
			{
				current_state = FLOOR_4TH;
			}
		break;
	case GPIO_PIN_1:
		pin_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
			if(pin_state)
			{
				current_state = FLOOR_3RD;
			}
		break;
	case GPIO_PIN_2:
		pin_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
			if(pin_state)
			{
				current_state = FLOOR_2ND;
			}
		break;
	case GPIO_PIN_4: //PC4 / BTN1
		pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);
			if (pin_state) // LOW
			{
				current_state = FLOOR_1ST;
			}
		break;
	}
}

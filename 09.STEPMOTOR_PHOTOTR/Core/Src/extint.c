/*
 * extint.c
 *
 *  Created on: Apr 8, 2025
 *      Author: user
 */
#include "extint.h"

extern volatile uint8_t stepmotor_state;
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
		stepmotor_state = IDLE;
		printf("idle-gpio_pin_0\n");
		break;
	case GPIO_PIN_4: //PC4 / BTN1
		if (stepmotor_state == IDLE)
			stepmotor_state = FORWARD;
		else if ( stepmotor_state == FORWARD)
			stepmotor_state = BACKWARD;
		else if ( stepmotor_state == BACKWARD)
			stepmotor_state = FORWARD;
		printf("idle-gpio_pin_4\n");
		break;
	}
}

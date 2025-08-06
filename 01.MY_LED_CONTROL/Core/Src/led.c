#include "led.h"



void led_all_on(void)
{
	//printf("int %d\n", sizeof(int)); // 4로 찍히는지 확인
	*( unsigned int *)GPIOB_ODR = 0xff;
}


void led_all_off(void)
{
	//printf("int %d\n", sizeof(int)); // 4로 찍히는지 확인
	*( unsigned int *)GPIOB_ODR = 0x00;
}


void shift_left_ledon(void)
{
	for(int count = 0; count < 8; count++)
	{
		*( unsigned int *)GPIOB_ODR = 0x01 << count;
		HAL_Delay(100);

	}
	led_all_off();
	HAL_Delay(100);
}

void shift_right_ledon(void)
{
	for(int count = 0; count < 8; count++)
		{
			*( unsigned int *)GPIOB_ODR = 0x80 >> count;
			HAL_Delay(100);

		}
		HAL_Delay(100);
}

void shift_left_keep_ledon(void)
{
	for(int count = 0; count < 8; count++)
	{
		*( unsigned int *)GPIOB_ODR |= 0x01 << count;
		HAL_Delay(100);
	}
	led_all_off();
	HAL_Delay(100);
}


void shift_right_keep_ledon(void)
{
	for(int count = 0; count < 8; count++)
	{
		*( unsigned int *)GPIOB_ODR |= 0x80 >> count;
		HAL_Delay(100);
	}
	led_all_off();
	HAL_Delay(100);
}


void flower_on(void)
{
	for(int i = 0; i < 4; i++)
	{
		* (unsigned int *)GPIOB_ODR |= 0x10 << i | 0x08 >> i;
		HAL_Delay(100);
	}
	led_all_off();
	HAL_Delay(100);
}


void flower_off(void)
{
	led_all_on();
	HAL_Delay(100);
	for(int i = 0; i < 4; i++)
	{
		* (unsigned int *)GPIOB_ODR &= ~(0x80 >> i | 0x01 << i);
		HAL_Delay(100);
	}
	led_all_off();
	HAL_Delay(100);
}

void led_main(void)
{
	while(1)
	{
		shift_left_ledon();
		shift_right_ledon();
		shift_left_keep_ledon();
		shift_right_keep_ledon();
		flower_on();
		flower_off();
	}
}

/*
 * stepmotor.c
 *
 *  Created on: Apr 8, 2025
 *      Author: microsoft
 */

#include "stepmotor.h"
#include "button.h"
#include "extern.h"
volatile uint8_t stepmotor_state = IDLE;

void set_rpm(int rpm);
void stepmotor_main(void);
int stepmotor_drive(int direction);

/*
 * RPM(Revolutions Per Minutes : 분당 최전수
 * 1분 60sec : 1sec(1,000,000us) * 60sec : 60,000,000us
 * 1초 : 1000ms --> 1ms(1,000us) * 1000ms : 1,000,000us
 * 1바퀴 회전 : 4096 step 필요
 * 4096 / 8 ==> 512 sequence : 360도 회전
 * 1 sequence (8step) : 0.70312도 회전
 * 0.70312 x 512 sequence : 360도 회전
 * ---- RPM 조절 함수 ----
 * 예) 1분에 13회전이 최대 속도
 * 13회전 : 60,000,000us(1분) / 4096 step / 13 ==> step과 step간의 간격 time : 1126us
 * 1126us x 4096(1회전하는데 필요 step) ==> 4,615,384us
 *                                 ==> 4615ms(4.6초)
 * 60sec / 4.6sec(1회전하는데 소요 시간) : 13회전
 */

void set_rpm(int rpm) // rmp : 1~13
{
	delay_us(60000000/4096/rpm);
	// 최대 speed 기준(13) : delay_us(1126);
}

// stepmotor demo
void stepmotor_main(void)
{
	GPIOC->PUPDR = 0x55;
#if 1 // O/S
	switch(stepmotor_state)
	{
	case IDLE:
		if (get_button(GPIOC, BTN1_Pin, BTN1) == BUTTON_PRESS)
		{
			stepmotor_state = FORWARD;
		}
		break;
	case FORWARD:
		stepmotor_drive(FORWARD);
		set_rpm(13);
		if (get_button(GPIOC, BTN0_Pin, BTN0) == BUTTON_PRESS)
		{
			stepmotor_state = IDLE;
		}
		if (get_button(GPIOC, BTN1_Pin, BTN1) == BUTTON_PRESS)
		{
			stepmotor_state = BACKWARD;
		}
		break;
	case BACKWARD:
		stepmotor_drive(BACKWARD);
		set_rpm(13);
		if (get_button(GPIOC, BTN0_Pin, BTN0) == BUTTON_PRESS)
		{
			stepmotor_state = IDLE;
		}
		if (get_button(GPIOC, BTN1_Pin, BTN1) == BUTTON_PRESS)
		{
			stepmotor_state = FORWARD;
		}
		break;
	}
#endif

#if 0 // none O/S
	while(1)
	{
		switch(stepmotor_state)
		{
		case IDLE:
			if (get_button(GPIOC, BTN1_Pin, BTN1) == BUTTON_PRESS)
			{
				stepmotor_state = FORWARD;
			}
			break;
		case FORWARD:
			stepmotor_drive(FORWARD);
			set_rpm(13);
			if (get_button(GPIOC, BTN0_Pin, BTN0) == BUTTON_PRESS)
			{
				stepmotor_state = IDLE;
			}
			if (get_button(GPIOC, BTN1_Pin, BTN1) == BUTTON_PRESS)
			{
				stepmotor_state = BACKWARD;
			}
			break;
		case BACKWARD:
			stepmotor_drive(BACKWARD);
			set_rpm(13);
			if (get_button(GPIOC, BTN0_Pin, BTN0) == BUTTON_PRESS)
			{
				stepmotor_state = IDLE;
			}
			if (get_button(GPIOC, BTN1_Pin, BTN1) == BUTTON_PRESS)
			{
				stepmotor_state = FORWARD;
			}
			break;
		}
	}
#endif

#if 0 // demo none O/S
	while(1)
	{
		for (int i = 0; i < 512; i++) // 시계 방향 1회전
		{
			for (int j = 0; j < 8; j++) // 1 sequence : 8 step
			{
				stepmotor_drive(j);
				set_rpm(13); // 1126us wait
			}
		}

		for (int i = 0; i < 512; i++) // 시계 반대 방향 1회전
		{
			for (int j = 7; j >= 0; j--) // 1 sequence : 8 step
			{
				stepmotor_drive(j);
				set_rpm(13); // 1126us wait
			}
		}
	}
#endif
}

int stepmotor_drive(int direction)
{
	static int step = 0;
	switch(step)
	{
	case 0:
		HAL_GPIO_WritePin(GPIOC, IN1_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
		break;
	case 1:
		HAL_GPIO_WritePin(GPIOC, IN1_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN2_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
		break;
	case 2:
		HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN2_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
		break;
	case 3:
		HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN2_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN3_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
		break;
	case 4:
		HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN3_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
		break;
	case 5:
		HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN3_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN4_Pin, 1);
		break;
	case 6:
		HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN4_Pin, 1);
		break;
	case 7:
		HAL_GPIO_WritePin(GPIOC, IN1_Pin, 1);
		HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
		HAL_GPIO_WritePin(GPIOC, IN4_Pin, 1);
		break;
	}

	if (direction == FORWARD)
	{
		step++;
		step %= 8; // 다음 진행할 step for (i = 0; i < 512; i++)
	}
	else // BACKWARD
	{
		step--;
		if (step < 0) // for (j = 7; j <  0; j--)
		{
			step = 7;
		}
	}
}

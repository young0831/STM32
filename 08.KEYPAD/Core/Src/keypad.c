#include "keypad.h"

GPIO_TypeDef* keypadRowPort[4] = {GPIOC, GPIOC, GPIOC, GPIOC}; //R1~R4
GPIO_TypeDef* keypadColPort[4] = {GPIOC, GPIOC, GPIOC, GPIOC}; //C1~C4
uint16_t keypadRowPin[4] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7}; //R1~R4 GPIO Input & Pull-up으로 설정을 해야 한다.
uint16_t keypadColPin[4] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11}; //C1~C4  GPIO Output으로만 설정 한다.

void keypadInit()
{
	for(uint8_t col = 0; col < 4; col++)
	{
		HAL_GPIO_WritePin(keypadColPort[col], keypadColPin[col], SET); //초기 값 1로 셋팅
	}
}

uint8_t getKeypadState(uint8_t col, uint8_t row)
{
#if 1
	uint8_t keypadChar[4][4] = {
			{'1', '4', '7', ' '},
			{'2', '5', '8', '0'},
			{'3', '6', '9', '='},
			{'/', '*', '-', '+'},
	};

#else
	uint8_t keypadChar[4][4] = {
			{'1', '2', '3', '/'},
			{'4', '5', '6', '*'},
			{'7', '8', '9', '-'},
			{' ', '0', '=', '+'},
	};
#endif
	static uint8_t prevState[4][4] = {
			{0, 1, 1, 1},
			{1, 1, 1, 1},
			{1, 1, 1, 1},
			{1, 1, 1, 1},
	};
	uint8_t curState = 1;

	HAL_GPIO_WritePin(keypadColPort[col], keypadColPin[col], RESET);
	curState = HAL_GPIO_ReadPin(keypadRowPort[row], keypadRowPin[row]);

	HAL_GPIO_WritePin(keypadColPort[col], keypadColPin[col], SET);

	if(curState == PUSHED && prevState[col][row] == RELEASED)
	{
		prevState[col][row] = curState;
		return 0;
	}
	else if (curState == RELEASED && prevState[col][row] == PUSHED)
	{
		prevState[col][row] = curState;
		return keypadChar[col][row];
	}
	return 0;
}

uint8_t keypadScan()
{
	uint8_t data;

	for(uint8_t col=0; col<4; col++)
	{
		for(uint8_t row=0; row<4; row++)
		{
			data = getKeypadState(col, row);
			if(data != 0)
			{
				return data;
			}
		}
	}
	return 0;
}

int multiple_number(char a, char b, char c)
{
	int num1, num2;
	num1 = atoi(a);
	num2 = atoi(c);
	int result = 0;

	switch (c)
	{
	case '+' :
		result = num1 + num2;
		break;
	case '-' :
		result = num1 - num2;
		break;
	case '*' :
		result = num1 * num2;
		break;
	case '/' :
		result = num1 / num2;
		break;
	}
	return result;
}








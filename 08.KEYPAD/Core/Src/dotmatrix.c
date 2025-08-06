
#include "main.h"



void dotmatrix_main_test();
void init_dotmatrix(void);
void dotmatrix_main(void);
void dotmatrix_main_func(void);

uint8_t allon[] = {			// allon 문자 정의
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111
};


uint8_t smile[] = {			// 스마일 문자 정의
	0b00111100,
	0b01000010,
	0b10010101,
	0b10100001,
	0b10100001,
	0b10010101,
	0b01000010,
	0b00111100 };

uint8_t hart[] = {		// hart
	0b00000000,    // hart
	0b01100110,
	0b11111111,
	0b11111111,
	0b11111111,
	0b01111110,
	0b00111100,
	0b00011000
};

uint8_t one[] =
{0b00011000,
0b00111000,
0b00011000,
0b00011000,
0b00011000,
0b00011000,
0b01111110,
0b01111110};

uint8_t my_name[] =
{0B01111010,
0B00001010,
0B00001010,
0B11111010,
0B00100010,
0B10101110,
0B10000010,
0B11111110};

uint8_t col[4]={0,0,0,0};

void dotmatrix_main_test()
{
	uint8_t temp;
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_15, 0);
	HAL_Delay(10);

	while (1)
	{
        for (int i=0; i < 3; i++)
        {
			col[0] = ~(1 << i);  // 00000001  --> 11111110
			col[1] = hart[i];
			//HAL_SPI_Transmit(&hspi2, col, 2, 1);
			for ( int j = 0; j < 2; j++)
			{
				for (int k=0; k<8; k++)
				{
					temp = col[j];
					if(temp & (1 << k))
						HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 1);
					else
						HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 0);
						HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 1);	// clk 을 상승에서
					HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 0);	//        하강으로
				}
			}
			GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
			GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up
			HAL_Delay(1);
        }
  }
}


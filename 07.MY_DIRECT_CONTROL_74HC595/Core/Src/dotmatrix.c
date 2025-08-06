
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

uint8_t name_data[20][3 ] =
{
	//박
	{
		0b01010100,
		0b01110110,
		0b01010100,
		0b01110100,
		0b00000000,
		0b01111100,
		0b00000100,
		0b00000000

	},
	//준
	{
		0b01111100,
		0b00010000,
		0b00101000,
		0b01000100,
		0b11111110,
		0b00010000,
		0b10000000,
		0b11111110

	},
	//영
	{
		0b00110010,
		0b01001110,
		0b01001010,
		0b00110110,
		0b00000010,
		0b00111100,
		0b01000010,
		0b00111100
	}
};
uint8_t up_arrow[]=
{
	0b00011000,
	0b00100100,
	0b01000010,
	0b11100111,
	0b00100100,
	0b00100100,
	0b00111100,
	0b00000000
};
uint8_t down_arrow[]=
{
	0b00000000,
	0b00111100,
	0b00100100,
	0b00100100,
	0b11100111,
	0b01000010,
	0b00100100,
	0b00011000
};

uint8_t col[4]={0,0,0,0};
void up_arrow_animation()
{
    uint32_t start_time = HAL_GetTick();
    uint8_t shift_arrow[8];
    memcpy(shift_arrow, up_arrow, 8); // 원본 복사

    while (HAL_GetTick() - start_time < 100) // 30초 동안
    {
        for (int shift = 0; shift < 8; shift++)
        {
            for (int repeat=0; repeat<50; repeat++) // ★ 50번 재빨리 반복
            {
                for (int i=0; i<8; i++)
                {
                    col[0] = ~(1 << i);
                    col[1] = (i+shift < 8) ? shift_arrow[i+shift] : 0x00;
                    dotmatrix_display(col);
                }
            }
        }
    }
}

unsigned char display_data[8];  // 최종 8x8 출력할 데이터
unsigned char scroll_buffer[50][8] = {0};  // 스코롤할 데이타가 들어있는 버퍼
int number_of_character = 3;  // 출력할 문자 갯수

void init_dotmatrix(void)
{
	for (int i=0; i < 8; i++)
	{
		display_data[i] = name_data[i];
	}
	for (int i=1; i < number_of_character+1; i++)
	{
		for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
		{
			scroll_buffer[i][j] = name_data[i-1][j];
		}
	}
}

void name_scroll()
{
    while (1)
    {
        for (int scroll = 0; scroll < 8; scroll++)  // 스크롤 단계
        {
            for (int repeat = 0; repeat < 50; repeat++) // ★ 반복해서 잔상 제거
            {
                for (int name_idx = 0; name_idx < 3; name_idx++) // 박, 준, 영
                {
                    for (int i = 0; i < 8; i++)
                    {
                        col[0] = ~(1 << i);
                        col[1] = (name_data[name_idx][i] >> scroll);
                        dotmatrix_display(col);
                    }
                }
            }
            HAL_Delay(100); // ★ 스크롤 속도 조절 → 커질수록 느려짐
        }
    }
}

void dotmatrix_main_func(void)
{
	static int count=0;  // 컬럼 count
	static int index=0;  // scroll_buffer의 2차원 index값
	static uint32_t past_time=0;  // 이전 tick값 저장

	init_dotmatrix();

	while(1)
	{
		uint32_t now = HAL_GetTick();  // 1ms
		// 1.처음시작시 past_time=0; now: 500 --> past_time=500
		if (now - past_time >= 500) // 500ms scroll
		{
			past_time = now;
			for (int i=0; i < 8; i++)
			{

				display_data[i] = scroll_buffer[index][(i + count) % 8];
			}
			if (++count == 8) // 8칼람을 다 처리 했으면 다음 scroll_buffer로 이동
			{
				count =0;
				index++;  // 다음 scroll_buffer로 이동
				if (index == number_of_character+1) index=0;
				// 11개의 문자를 다 처리 했으면 0번 scroll_buffer를 처리 하기위해 이동
			}
		}
		for (int i=0; i < 8; i++)
		{
			// 공통 양극 방식
			// column에는 0을 ROW에는 1을 출력해야 해당 LED가 on된다.
			col[0] = ~(1 << i);  // 00000001  --> 11111110
			col[1] = display_data[i];
			GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
			GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up
			HAL_Delay(1);
		}
	}
	return 0;
}

void dotmatrix_display(uint8_t *col)
{
    uint8_t temp;
    for (int j=0; j<2; j++)
    {
        for (int k=0; k<8; k++)
        {
            temp = col[j];
            HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, (temp & (1<<k)) ? 1 : 0);
            HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 1);
            HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 0);
        }
    }
    GPIOB->ODR &= ~GPIO_PIN_13;
    GPIOB->ODR |= GPIO_PIN_13;
    HAL_Delay(0);  // or HAL_Delay(1) (되도록 줄이기)
}

void dotmatrix_main_test()
{
	uint8_t temp;
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_15, 0);
	HAL_Delay(10);
	init_dotmatrix();
	up_arrow_animation();
	name_scroll();
	while (1)
	{
//        for (int i=0; i < 8; i++)
//        {
//			col[0] = ~(1 << i);  // 00000001  --> 11111110
//			col[1] = hart[i];
//			//HAL_SPI_Transmit(&hspi2, col, 2, 1);
//			for ( int j = 0; j < 2; j++)
//			{
//				for (int k=0; k<8; k++)
//				{
//					temp = col[j];
//					if(temp & (1 << k))
//						HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 1);
//					else
//						HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 0);
//						HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 1);	// clk 을 상승에서
//					HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 0);	//        하강으로
//				}
//			}
//			GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
//			GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up
//			HAL_Delay(1);
//        }
  }
}


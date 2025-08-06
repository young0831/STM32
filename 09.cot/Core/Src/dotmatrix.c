#if 0
#include "main.h"

// extern SPI_HandleTypeDef hspi2;

void dotmatrix_main_test();
void init_dotmatrix(void);
int dotmatrix_main(void);
int dotmatrix_main_func(void);
int dotmatrix_main_func_rev(void);

uint8_t allon[] = {         // allon 문자 정의
   0b11111111,
   0b11111111,
   0b11111111,
   0b11111111,
   0b11111111,
   0b11111111,
   0b11111111,
   0b11111111
};

uint8_t smile[] = {         // 스마일 문자 정의
   0b00111100,
   0b01000010,
   0b10010101,
   0b10100001,
   0b10100001,
   0b10010101,
   0b01000010,
   0b00111100
};

uint8_t hart[] = {      // hart
   0b00000000,    // hart
   0b01100110,
   0b11111111,
   0b11111111,
   0b11111111,
   0b01111110,
   0b00111100,
   0b00011000
};

uint8_t one[] ={
   0b00000000, // 1
   0b00011000,
   0b00111000,
   0b00011000,
   0b00011000,
   0b00011000,
   0b00011000,
   0b01111110
};

uint8_t my_name[] ={
   0B01111010,
   0B00001010,
   0B00001010,
   0B11111010,
   0B00100010,
   0B10101110,
   0B10000010,
   0B11111110
};

uint8_t fuxku[] = {
   0b00000000,
   0b00000000,
   0b11111000,
   0b11111000,
   0b11111111,
   0b11111000,
   0b01110000,
   0b00000000
};

uint8_t apple[] = {
   0b01111000,
   0b10000100,
   0b10000100,
   0b01001000,
   0b01001010,
   0b10000101,
   0b10110100,
   0b01001000
};

uint8_t col[4]={0,0,0,0};

void dotmatrix_main_test()
{
  //dotmatrix_main();

  while (1)
  {
        for (int i=0; i < 8; i++)
        {
         col[0] = ~(1 << i);  // 00000001  --> 11111110
         col[1] = one[i];
         // HAL_SPI_Transmit(&hspi2, col, 2, 1);
         direct(col, 2);
         GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
         GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up
         HAL_Delay(1);
        }
  }
}

uint8_t number_data[20][10] =
{
   {   0b00000000, // 1
      0b00011000,
      0b00111000,
      0b00011000,
      0b00011000,
      0b00011000,
      0b00011000,
      0b01111110
   },
   {   0b00000000,
      0b01111110,
      0b00000010,
      0b00000010,
      0b01111110,
      0b01000000,
      0b01000000,
      0b01111110
   },
   {   0b00000000,
      0b01111110,
      0b00000010,
      0b00000010,
      0b01111110,
      0b00000010,
      0b00000010,
      0b01111110
   },
   {   0b00000000,
      0b00001100,
      0b00010100,
      0b00100100,
      0b01000100,
      0b01111110,
      0b00000100,
      0b00000100
   }
};

void direct(uint8_t* data, uint8_t len)
{
    uint8_t temp;
    for (int j = 0; j < len; j++)
    {
        for (int k = 0; k < 8; k++)
        {
            temp = data[j];
            if (temp & (1 << k))
                HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 1);
            else
                HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 0);

            // CLK 펄스: 상승 -> 하강
            HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 1);
            HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 0);
        }
    }
}


unsigned char display_data[8];  // 최종 8x8 출력할 데이터
unsigned char scroll_buffer[50][8] = {0};  // 스코롤할 데이타가 들어있는 버퍼
int number_of_character = 4;  // 출력할 문자 갯수

// 초기화 작업
// 1. display_data에 number_data[0]에 있는 내용 복사
// 2. number_data를 scroll_buffer에 복사
// 3. dotmatrix의 led를 off
void init_dotmatrix(void)
{
   for (int i=0; i < 8; i++)
   {
      display_data[i] = number_data[i];
   }
   for (int i=1; i < number_of_character+1; i++)
   {
      for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
      {
         scroll_buffer[i][j] = number_data[i-1][j];
      }
   }
}

// scroll 문자 출력 프로그램
int dotmatrix_main(void)
{
   static int count=0;  // 컬럼 count
   static int index=0;  // scroll_buffer의 2차원 index값
   static uint32_t past_time=0;  // 이전 tick값 저장


   uint32_t now = HAL_GetTick();  // 1ms
   // 1.처음시작시 past_time=0; now: 500 --> past_time=500
   if (now - past_time >= 500) // 500ms scroll
   {
      past_time = now;
      for (int i=0; i < 8; i++)
      {

         display_data[i] = (scroll_buffer[index][i] >> count) |
               (scroll_buffer[index+1][i] << 8 - count);
      }
      if (++count == 8) // 8칼람을 다 처리 했으면 다음 scroll_buffer로 이동
      {
         count =0;
         index++;  // 다음 scroll_buffer로 이동
         if (index == number_of_character+1) index=0;
         // 11개의 문자를 다 처리 했으면 0번 scroll_buffer를 처리 하기위해 이동
      }
   }
/*
       0b00000000,    // hart
      0b01100110,
      0b11111111,
      0b11111111,
      0b11111111,
      0b01111110,
      0b00111100,
      0b00011000
 */
   for (int i=0; i < 8; i++)
   {
      // 공통 양극 방식
      // column에는 0을 ROW에는 1을 출력해야 해당 LED가 on된다.
      col[0] = ~(1 << i);  // 00000001  --> 11111110
      col[1] = display_data[i];
      // HAL_SPI_Transmit(&hspi2, col, 2, 1);
      direct(col, 2);
      GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
      GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up
      HAL_Delay(1);
   }
}

// scroll 문자 출력 프로그램 (수직 스크롤 버전)
int dotmatrix_main_func(void)
{
    static int count = 0;       // 행(row) count (현재 문자 블록 내 스크롤된 행 수)
    static int index = 0;       // scroll_buffer의 문자 블록 인덱스
    static uint32_t past_time = 0;  // 이전 tick값 저장

    init_dotmatrix();  // 점매트릭스 초기화

    while(1)
    {
        uint32_t now = HAL_GetTick();  // 현재 tick (1ms 단위)

        // 500ms 마다 스크롤(수직) 진행
        if (now - past_time >= 500)
        {
            past_time = now;

            // 기존엔 각 칼럼을 대상으로 좌우 스크롤 연산을 했으나,
            // 수직 스크롤 시 각 칼럼의 내부 비트(행)를 위아래로 이동시키도록 함.
            for (int i = 0; i < 8; i++)
            {
                display_data[i] = (scroll_buffer[index][i] << count) |
                                  (scroll_buffer[index+1][i] >> (8 - count));
            }

            // 8행(한 문자 블록)이 모두 스크롤되면 다음 문자 블록으로 이동
            if (++count == 8)
            {
                count = 0;
                index++;  // 다음 문자 블록으로 이동

                // 모든 문자를 다 처리했으면 처음으로 복귀
                if (index == number_of_character + 1)
                    index = 0;
            }
        }

        // dotmatrix 출력: 기존과 동일하게 칼럼 단위 스캔 (공통 양극 방식)
        for (int i = 0; i < 8; i++)
        {
            // 칼럼 선택: 해당 칼럼은 0, 나머지는 1로 출력해야 함
            col[0] = ~(1 << i);  // 예: i==0일 경우 11111110 (0xFE)
            col[1] = display_data[i];

            // SPI 또는 직접 함수(direct)를 통해 데이터를 전송
            direct(col, 2);

            // latch 펄스 생성 (하강 후 상승)
            GPIOB->ODR &= ~GPIO_PIN_13;   // latch 핀 pull-down
            GPIOB->ODR |= GPIO_PIN_13;      // latch 핀 pull-up

            HAL_Delay(1);  // 간단한 딜레이 (LED 깜빡임 제어)
        }
    }

    return 0;
}

// scroll 문자 출력 프로그램 (수직 스크롤 버전)
int dotmatrix_main_func_rev(void)
{
    static int count = 0;       // 행(row) count (현재 문자 블록 내 스크롤된 행 수)
    static int index = 0;       // scroll_buffer의 문자 블록 인덱스
    static uint32_t past_time = 0;  // 이전 tick값 저장

    init_dotmatrix();  // 점매트릭스 초기화

    while(1)
    {
        uint32_t now = HAL_GetTick();  // 현재 tick (1ms 단위)

        // 500ms 마다 스크롤(수직) 진행
        if (now - past_time >= 500)
        {
            past_time = now;

            // 기존엔 각 칼럼을 대상으로 좌우 스크롤 연산을 했으나,
            // 수직 스크롤 시 각 칼럼의 내부 비트(행)를 위아래로 이동시키도록 함.
            for (int i = 0; i < 8; i++)
            {
                display_data[i] = (scroll_buffer[index][i] << count) |
                                  (scroll_buffer[index+1][i] >> (8 - count));
            }

            // 8행(한 문자 블록)이 모두 스크롤되면 다음 문자 블록으로 이동
            if (++count == 8)
            {
                count = 0;
                index++;  // 다음 문자 블록으로 이동

                // 모든 문자를 다 처리했으면 처음으로 복귀
                if (index == number_of_character + 1)
                    index = 0;
            }
        }

        // dotmatrix 출력: 기존과 동일하게 칼럼 단위 스캔 (공통 양극 방식)
        for (int i = 7; i >= 0; i--)
        {
            // 칼럼 선택: 해당 칼럼은 0, 나머지는 1로 출력해야 함
            col[0] = ~(1 << i);  // 예: i==0일 경우 11111110 (0xFE)
            col[1] = display_data[i];

            // SPI 또는 직접 함수(direct)를 통해 데이터를 전송
            direct(col, 2);

            // latch 펄스 생성 (하강 후 상승)
            GPIOB->ODR &= ~GPIO_PIN_13;   // latch 핀 pull-down
            GPIOB->ODR |= GPIO_PIN_13;      // latch 핀 pull-up

            HAL_Delay(1);  // 간단한 딜레이 (LED 깜빡임 제어)
        }
    }

    return 0;
}

#endif

#if 1
#include "main.h"
#include "extern.h"

void dotmatrix_main_test();
void dotmatrix_main_test2();
void init_dotmatrix(void);

volatile uint8_t dot_number = 0;

uint8_t start[] = {
	0b00000000,
	0b01100000,
	0b01100110,
	0b01101111,
	0b11110110,
	0b01100110,
	0b00000110,
	0b00000000
};

uint8_t select[] = {
	0b00111001,
	0b01000010,
	0b10000100,
	0b10101001,
	0b10010001,
	0b10000001,
	0b01000010,
	0b00111100
};

uint8_t one[] =  {
   0b00000000,
   0b00011000,
   0b00111000,
   0b00011000,
   0b00011000,
   0b00011000,
   0b00011000,
   0b01111110
};

uint8_t two[] = {
   0b00000000,
   0b01111110,
   0b00000010,
   0b00000010,
   0b01111110,
   0b01000000,
   0b01000000,
   0b01111110
};

uint8_t three[] = {
   0b00000000,
   0b01111110,
   0b00000010,
   0b00000010,
   0b01111110,
   0b00000010,
   0b00000010,
   0b01111110
};

uint8_t four[] =
{
   0b00000000,
   0b00001100,
   0b00010100,
   0b00100100,
   0b01000100,
   0b01111110,
   0b00000100,
   0b00000100
};

uint8_t col[4]={0,0,0,0};

void dotmatrix_main_test()
{
   uint8_t temp;
   static int i = 0;
   col[1] = start[i];

  col[0] = ~(1 << i);  // 00000001  --> 11111110
  if (dot_number == 1)
    col[1] = one[i];
  else if(dot_number == 2)
    col[1] = two[i];
  else if(dot_number == 3)
    col[1] = three[i];
  else if(dot_number == 4)
      col[1] = four[i];
    // HAL_SPI_Transmit(&hspi2, col, 2, 1);
  for (int j = 0; j < 2; j++)
  {
     for (int k = 0; k < 8; k++)
     {
        temp = col[j];
        if (temp & (1 << k))
        {
           HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 1);
        }
        else
        {
           HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 0);
        }
        HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 1); // clk을 상승에서
        HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 0); //       하강으로
     }
  }
  GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
  GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up

  i++;
  i %= 8; // 다음 진행할 step  for(i = 0; i < 8; i++)
}

void dotmatrix_main_test2()
{
   uint8_t temp;
   static int i = 0;
   col[1] = select[i];

  col[0] = ~(1 << i);  // 00000001  --> 11111110
  if (dot_number == 1)
    col[1] = one[i];
  else if(dot_number == 2)
    col[1] = two[i];
  else if(dot_number == 3)
    col[1] = three[i];
  else if(dot_number == 4)
      col[1] = four[i];
    // HAL_SPI_Transmit(&hspi2, col, 2, 1);
  for (int j = 0; j < 2; j++)
  {
     for (int k = 0; k < 8; k++)
     {
        temp = col[j];
        if (temp & (1 << k))
        {
           HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 1);
        }
        else
        {
           HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 0);
        }
        HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 1); // clk을 상승에서
        HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 0); //       하강으로
     }
  }
  GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
  GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up

  i++;
  i %= 8; // 다음 진행할 step  for(i = 0; i < 8; i++)
}
#endif

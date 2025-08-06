/*
 * fnd.c
 *
 *  Created on: Apr 9, 2025
 *      Author: microsoft
 */

#include "fnd.h"
#include "extern.h"
#include "stepmotor.h"
uint16_t fnd_font[] = { // 00100101      //11100001  11000101  00110001
   ~0xc0, // 0 0b11000000 0b   10100100 0b10000111 0b10100011 10001100
   ~0xf9, // 1
   ~0xa4, // 2
   ~0xb0, // 3
   ~0x99, // 4
   ~0x92, // 5
   ~0x82, // 6
   ~0xd8, // 7
   ~0x80, // 8
   ~0x98, // 9
   ~0x7f  // .
};

uint16_t stop[] = {
		~0x92,
		~0x87,
		~0xc0,
		~0x8c
};
uint16_t go[] = {
		~0xff,
		~0xff,
		~0x90, //0b10010000
		~0xa3
};
void send_1byte74hc595_fnd(uint8_t data)
{
   for (int i = 0; i <8; i++)
   {
   if(data & (1 << i))
   HAL_GPIO_WritePin(FND_DATA_GPIO_Port, FND_DATA_Pin, 1); // 데이터 값 입력
   else
   HAL_GPIO_WritePin(FND_DATA_GPIO_Port, FND_DATA_Pin, 0);

   HAL_GPIO_WritePin(FND_CLK_GPIO_Port, FND_CLK_Pin, 1);
   HAL_GPIO_WritePin(FND_CLK_GPIO_Port, FND_CLK_Pin, 0);
   }
}

void fnd_init(void)
{
   // idle
   HAL_GPIO_WritePin(FND_DATA_GPIO_Port, FND_DATA_Pin, 0);
   HAL_GPIO_WritePin(FND_CLK_GPIO_Port, FND_CLK_Pin, 0);
   HAL_GPIO_WritePin(FND_LATCH_GPIO_Port, FND_LATCH_Pin, 0);
   HAL_Delay(10);   // 안정화 시간
}


uint8_t fnd_data[4]={0,0,0,0};

void fnd_display(void)
{
      uint8_t temp;

      static int i = 0;
      fnd_data[0] = ~(1 << (i+4));      // 자릿수 선택

   //ds1302.minutes
   //ds1302.hours
      if(stepmotor_state == IDLE)
      {
    	  switch(i)
    	  {
		  case 0:
			fnd_data[1] = stop[0];
			break;
		  case 1:
			fnd_data[1] = stop[1];
			break;
		  case 2 :
			fnd_data[1] = stop[2];
			break;
		  case 3:
			fnd_data[1] = stop[3];
			break;
		  }
      }
      else
    	  switch(i)
		  {
		  case 0:
			fnd_data[1] = go[0];
			break;
		  case 1:
			fnd_data[1] = go[1];
			break;
		  case 2 :
			fnd_data[1] = go[2];
			break;
		  case 3:
			fnd_data[1] = go[3];
			break;
		  }

      for(int j = 0; j<2; j++)
      {
         temp = fnd_data[j]; // 보낼 데이터 (1바이트)
         send_1byte74hc595_fnd(temp);
      }
   i++;
   i %=4;
   GPIOC->ODR &= ~FND_LATCH_Pin;   // latch핀을 pull-down
   GPIOC->ODR |= FND_LATCH_Pin;   // latch핀을 pull-up // Latch로 데이터 과정
}

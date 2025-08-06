/*
 * ds1302.c
 *
 *  Created on: Mar 28, 2025
 *      Author: microsoft
 */

#include "ds1302.h"

void ds1302_main(void)
{
	t_ds1302 ds1302;
	t_ds1302 *ds = &ds1302;

	init_date_time(ds);
	init_gpio_ds1302();
	init_ds1302(ds);

	while(1)
	{
		read_time_ds1302(ds);
		read_date_ds1302(ds);
		// 날짜와 시각을 출력
		printf(" %4d-%2d-%2d %2d:%2d:%2d\n",
				ds->year + 2000,
				ds->month,
				ds->date,
				ds->hours,
				ds->minutes,
				ds->seconds);

		HAL_Delay(1000);
	}
}

void read_time_ds1302(t_ds1302 *ds)
{
	ds->seconds = read_ds1302(ADDR_SECONDS);
	ds->minutes = read_ds1302(ADDR_MINUTES);
	ds->hours = read_ds1302(ADDR_HOURS);
}

void read_date_ds1302(t_ds1302 *ds)
{
	ds->date = read_ds1302(ADDR_DATE);
	ds->month = read_ds1302(ADDR_MONTH);
	ds->dayofweek = read_ds1302(ADDR_DAYOFWEEK);
	ds->year = read_ds1302(ADDR_YEAR);
}

uint8_t read_ds1302(uint8_t addr)
{
	unsigned char data8bits = 0; // 1bit씩 넘어온 것을 담을 그릇(변수)
	// 1. CE high
	// HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 1);
	GPIOA_ODR |= 1 << 10;
	// 2. addr 전송
	tx_ds1302(addr + 1); // read addr
	// 3. data를 읽어 들인다.
	rx_ds1302(&data8bits);
	// 4. CE low
	// HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 0);
	GPIOA_ODR &= ~(1 << 10);
	// 5. return (bcd to dec )
	return (bcd2dec(data8bits));
}

// 1. 입력 : bcd
// 예) 25년의 bcd --> dec
//     7654 3210
//     0010 0101
//       2   5
//  +  x10  x1
//  ==============
//          25
uint8_t bcd2dec(uint8_t bcd)
{
	uint8_t high, low;

	low = bcd & 0x0f;
	high = (bcd >> 4) * 10; // 00100101 bcd >> 4 ==> 000000100 x 10

	return (high + low);
}

// dec --> bcd
// 예) 25
//   dec           bcd
// 00011001     0010 0101
uint8_t dec2bcd(uint8_t dec)
{
	uint8_t high, low;

	high = (dec / 10) << 4;
	low = dec % 10;

	return (high + low);
}

void rx_ds1302(unsigned char *pdata)
{
	unsigned char temp = 0;
	// IO 포트를 입력 모드로 전환
	input_dataline_ds1302();
	// DS1302로부터 들어온 bit를 LSB부터 8bit를 받아서 temp변수에 저장
	for (int i = 0; i < 8; i++)
	{
		// 1bit를 읽어 들인다.
		//if (HAL_GPIO_ReadPin(GPIOA, IO_DS1302_Pin))
		if (GPIOA_IDR & IO_DS1302_Pin)
		{
			// 1의 조건만 set
			temp |= 1 << i;
		}
		if (i != 7)
		{
			clock_ds1302();
		}
	}
	*pdata = temp;
}

void init_ds1302(t_ds1302 *ds)
{
	write_ds1302(ADDR_SECONDS, ds->seconds);
	write_ds1302(ADDR_MINUTES, ds->minutes);
	write_ds1302(ADDR_HOURS, ds->hours);
	write_ds1302(ADDR_DATE, ds->date);
	write_ds1302(ADDR_MONTH, ds->month);
	write_ds1302(ADDR_DAYOFWEEK, ds->dayofweek);
	write_ds1302(ADDR_YEAR, ds->year);
}

void write_ds1302(uint8_t addr, uint8_t data)
{
	// 1. CE low --> high
	// HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 1);
	GPIOA_ODR |= 1 << 10;
	// 2. addr 전송
	tx_ds1302(addr);
	// 3. data 전송
	tx_ds1302(dec2bcd(data));
	// 4. CE high --> low
	// HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 0);
	GPIOA_ODR &= ~(1 << 10);
}

void tx_ds1302(uint8_t value)
{
	output_dataline_ds1302();
	// 예) addr 초를 write하는
	// 80h M       L
	//     1000 0000   실제값 (B0를 전송시의 )
	//     0000 0001 &
	//     0000 0000  ==> HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 0);

	//     1000 0000   실제값 (B7을 전송시의 )
	//     1000 0000 &
	//     1000 0000  ==> HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 1);
	for (int i  = 0; i < 8; i++)
	{
		if (value & (1 << i))
		{
			// HAL_GPIO_WritePin(GPIOA, IO_DS1302_Pin, 1);
			GPIOA_ODR |= 1 << 11;
		}
		else
		{
			// HAL_GPIO_WritePin(GPIOA, IO_DS1302_Pin, 0);
			GPIOA_ODR &= ~(1 << 11);
		}
		clock_ds1302();
	}
}

void input_dataline_ds1302(void)
{
//	GPIO_InitTypeDef GPIO_init = {0, };

//	GPIO_init.Pin = IO_DS1302_Pin;
//	GPIO_init.Mode = GPIO_MODE_INPUT; // input mode
//	GPIO_init.Pull = GPIO_NOPULL;
//	HAL_GPIO_Init(GPIOA, &GPIO_init);
	GPIOA_ODR &= ~(1 << 11);
	GPIOA_MODER &= ~(0b11 << 22);
	GPIOA_OTYPER &= ~(0b1 << 11);
	GPIOA_PUPDR &= ~(0b11 << 22);
}

void output_dataline_ds1302(void)
{
//	GPIO_InitTypeDef GPIO_init = {0, };

//	GPIO_init.Pin = IO_DS1302_Pin;
//	GPIO_init.Mode = GPIO_MODE_OUTPUT_PP; // output mode
//	GPIO_init.Pull = GPIO_NOPULL;
//	GPIO_init.Speed = GPIO_SPEED_FREQ_HIGH; // LOW: 2MHz, HIGH: 25~100MHz
//	HAL_GPIO_Init(GPIOA, &GPIO_init);
	GPIOA_ODR &= ~(1 << 11);
	GPIOA_MODER |= 0b1 << 22;
	GPIOA_MODER &= ~(0b1 << 23);
	GPIOA_OTYPER &= ~(0b1 << 11);
	GPIOA_PUPDR &= ~(0b11 << 22);
	GPIOA_OSPEEDR |= 0b11 << 22;
}

void clock_ds1302(void)
{
	// HAL_GPIO_WritePin(GPIOA, CLK_DS1302_Pin, 1);
	GPIOA_ODR |= 1 << 12;
	// HAL_GPIO_WritePin(GPIOA, CLK_DS1302_Pin, 0);
	GPIOA_ODR &= ~(1 << 12);
}

void init_gpio_ds1302(void)
{
	// HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 0);
	GPIOA_ODR &= ~(1 << 10);
	// HAL_GPIO_WritePin(GPIOA, IO_DS1302_Pin, 0);
	GPIOA_ODR &= ~(1 << 11);
	// HAL_GPIO_WritePin(GPIOA, CLK_DS1302_Pin, 0);
	GPIOA_ODR &= ~(1 << 12);
}

void init_date_time(t_ds1302 *ds)
{
	ds->year = 25;
	ds->month = 3;
	ds->date = 28;
	ds->dayofweek  = 6; // Fri
	ds->hours = 11;
	ds->minutes = 30;
	ds->seconds = 00;
}

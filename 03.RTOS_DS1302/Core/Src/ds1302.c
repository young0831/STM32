
#include "ds1302.h"

void ds1302_main(void)
{
	init_date_time();  // ds1302 구조체 안에 있는 변수에 값을 할당
	init_gpio_ds1302();
	init_ds1302();

	while(1)
	{
		read_time_ds1302();
		read_date_ds1302();
		//날짜와 시각을 출력
		printf(" %4d-%2d-%2d %2d:%2d:%2d\n ",
					ds1302.date,
					ds1302.month,
					ds1302.year,
					ds1302.seconds,
					ds1302.minutes,
					ds1302.hours);
		HAL_Delay(1000);
	}
}

void read_time_ds1302(void)
{
	ds1302.seconds  = read_ds1302(ADDR_SECONDS);
	ds1302.minutes  = read_ds1302(ADDR_MINUTES);
	ds1302.hours 	= read_ds1302(ADDR_HOURS);
}

void read_date_ds1302(void)
{
	ds1302.date  = read_ds1302(ADDR_DATE);
	ds1302.month = read_ds1302(ADDR_MONTH);
	ds1302.dayofweek = read_ds1302(ADDR_DAYOFWEEK);
	ds1302.year  = read_ds1302(ADDR_YEAR);
}

uint8_t read_ds1302(uint8_t addr)
{
	unsigned char data8bits = 0; // 1bit씩 넘어온것을 담을 그릇(변수)
	// 1. CE high
	HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 1);// read addr
	// 2. addr 전송
	tx_ds1302(addr + 1);
	// 3. data를 읽어들임.
	rx_ds1302(&data8bits);
	// 4. CE low
	HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 0);
	// 5. return (bcd to dec)
	return (bcd2dec(data8bits));
}
// 1. 입력 : bcd
// 예) 25년의 bcd --> dec
//		7654 3210
//		0010 0101
//		  2	   5
//		x10	  x1
uint8_t bcd2dec(uint8_t bcd) // bcd는 이진화 십진수로써 0~9까지의 수를 4비트 이진수를 이용하여 숫자를 표현 즉, 0000~1001까지의 값이 존재함
{
	uint8_t high, low;

	low = bcd & 0x0f;
	high = (bcd >> 4) * 10;		// 0010 0101 우측으로 4만큼 시프트 하면 0000 0010(2)임 20이라는 값을 만들기 위해 10을 곱함

	return (high + low);
}
// dec --> bcd
// 예) 25
//
uint8_t dec2bcd(uint8_t dec)
{
	uint8_t high, low;

	high = (dec / 10) << 4;
	low  = dec % 10;

	return (high + low);
}

void rx_ds1302(unsigned char *pdata)
{
	unsigned char temp = 0;
	// IO 포트를 입력 모드로 전환
	input_dataline_ds1032();
	// DS1302로 부터 들어온 bit를 LSB부터 8bit를 받아서 temp 변수에 저장
	for ( int i = 0; i < 8; i++)
	{
		if(HAL_GPIO_ReadPin(GPIOA, IO_DS1302_Pin))//1bit를 읽어 드린다
		{
			temp |= 1 << i;
		}
		clock_ds1302();
	}
	*pdata = temp;
}


void init_ds1302(void)
{
	write_ds1302(ADDR_SECONDS, ds1302.seconds);
	write_ds1302(ADDR_MINUTES, ds1302.minutes);
	write_ds1302(ADDR_HOURS, ds1302.hours);
	write_ds1302(ADDR_DATE, ds1302.date);
	write_ds1302(ADDR_MONTH, ds1302.month);
	write_ds1302(ADDR_DAYOFWEEK, ds1302.dayofweek);
	write_ds1302(ADDR_YEAR, ds1302.year);
}

void write_ds1302(uint8_t addr, uint8_t data)
{
	// 1. CE low --> high
	HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 1);
	// 2. addr 전송
	tx_ds1302(addr);
	// 3. data 전송
	tx_ds1302(dec2bcd(data));
	// 4. CE high --> low
	HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 0);
}

tx_ds1302(uint8_t value)
{
	output_dataline_ds1032();
	// 예) addr 초를 write하는
	// 80h MSB			LSB
	//		1000 0000	실제값 (B0를 전송시의)
	//		0000 0001 &
	//		0000 0000	==> HAL_GPIO_WritePin(GPIOA, CE_DS1302_PIN, 0);
	//		1000 0000	실제값 (B7를 전송시의)
	//		1000 0000 &
	//		1000 0000	==> HAL_GPIO_WritePin(GPIOA, CE_DS1302_PIN, 1);
	for ( int i = 0; i < 8; i++)
	{
		if ( value & (1 << i) )
		{
			HAL_GPIO_WritePin(GPIOA, IO_DS1302_Pin, 1);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOA, IO_DS1302_Pin, 0);
		}
		clock_ds1302();
	}

}

void input_dataline_ds1032(void)
{
	GPIO_InitTypeDef GPIO_Init = {0,};

	GPIO_Init.Pin = IO_DS1302_Pin;
	GPIO_Init.Mode = GPIO_MODE_INPUT;	//INPUT mode
	GPIO_Init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_Init);
}

void output_dataline_ds1032(void) //DDR역할을 함. /
{
	GPIO_InitTypeDef GPIO_Init = {0,};

	GPIO_Init.Pin = IO_DS1302_Pin;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;	//OUTPUT mode
	GPIO_Init.Pull = GPIO_NOPULL;
	GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;	// LOW : 2M HIGHL 25~100MHz
	HAL_GPIO_Init(GPIOA, &GPIO_Init);
}

void clock_ds1302(void) // clock high->low로 동작
{
	HAL_GPIO_WritePin(GPIOA, CLK_DS1302_Pin, 1);
	HAL_GPIO_WritePin(GPIOA, CLK_DS1302_Pin, 0);
}

void init_gpio_ds1302(void) // Low 상태 set
{
	HAL_GPIO_WritePin(GPIOA, CE_DS1302_Pin, 0);
	HAL_GPIO_WritePin(GPIOA, IO_DS1302_Pin, 0);
	HAL_GPIO_WritePin(GPIOA, CLK_DS1302_Pin, 0);
}
void init_date_time(void)  //
{
	ds1302.year = 25; //ds1302 구조체 안에 들어있는 year 변수 호출
	ds1302.month = 3;
	ds1302.date = 28;
	ds1302.dayofweek = 6;
	ds1302.hours = 11;
	ds1302.minutes = 30;
	ds1302.seconds = 00;
}

/*
 * ds1302.h
 *
 *  Created on: Mar 28, 2025
 *      Author: microsoft
 */

#ifndef INC_DS1302_H_
#define INC_DS1302_H_

#include "main.h" // HAL GPIO
#define ADDR_SECONDS 0x80 // write addr만 define하기로 하자
						  // read addr은 write addr에 +1만 하면 되니까

#define ADDR_MINUTES 0x82
#define ADDR_HOURS 0x84
#define ADDR_DATE 0x86
#define ADDR_MONTH 0x88
#define ADDR_DAYOFWEEK 0x8a
#define ADDR_YEAR 0x8c
#define ADDR_WRITEPROTECTED 0x8e

#define GPIOA_MODER *(unsigned int *)0x40020000
#define GPIOA_OTYPER *(unsigned int *)0x40020004
#define GPIOA_ODR *(unsigned int *)0x40020014
#define GPIOA_IDR *(unsigned int *)0x40020010
#define GPIOA_PUPDR *(unsigned int *)0x4002000C
#define GPIOA_BSRR *(unsigned int *)0x40020018
#define GPIOA_OSPEEDR *(unsigned int *)0x40020008

typedef struct ds1302
{
	uint8_t seconds; // sec
	uint8_t minutes; // min
	uint8_t hours;
	uint8_t date;
	uint8_t month;
	uint8_t dayofweek; // 1: sun, 2: moon
	uint8_t year;
	uint8_t ampm; // 1: pm, 0: am
	uint8_t hourmode; // 0: 24, 1: 12
} t_ds1302;

//t_ds1302 ds1302;

void ds1302_main(void);
void read_time_ds1302(t_ds1302 *ds);
void read_date_ds1302(t_ds1302 *ds);
uint8_t read_ds1302(uint8_t addr);
uint8_t bcd2dec(uint8_t bcd);
uint8_t dec2bcd(uint8_t dec);
void rx_ds1302(unsigned char *pdata);
void init_ds1302(t_ds1302 *ds);
void write_ds1302(uint8_t addr, uint8_t data);
void tx_ds1302(uint8_t value);
void input_dataline_ds1302(void);
void output_dataline_ds1302(void);
void clock_ds1302(void);
void init_gpio_ds1302(void);
void init_date_time(t_ds1302 *ds);

#endif /* INC_DS1302_H_ */

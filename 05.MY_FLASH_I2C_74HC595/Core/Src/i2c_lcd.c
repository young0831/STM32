/*
 * i2c_lcd.c
 *
 *  Created on: 2019. 9. 4.
 *      Author: k
 */

#include "main.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "i2c_lcd.h"

// 외부에서 선언된 I2C 핸들러 (main.c 에 있을 것)
extern I2C_HandleTypeDef hi2c1;

// 함수 선언
void i2c_lcd_main(void);
void i2c_lcd_init(void);

// I2C 주소 설정 (0x27 은 PCF8574 의 기본 주소임, 왼쪽 쉬프트는 HAL이 8bit 주소를 요구해서)
#define I2C_LCD_ADDRESS (0x27 << 1)

// 테스트용 데이터 (아스키 코드로 '4', '3', '\0')
unsigned char lcd_test[4] = { '4','3', 0 };

// 테스트 메인 함수
void i2c_lcd_main(void)
{
//  while (1) // 무한루프
//  {
//	   // lcd_test 배열을 LCD로 송신 (2바이트만 전송)
//	   // 전송 실패 시 계속 재시도 (에러가 발생하면 HAL_OK가 아님)
//	   while(HAL_I2C_Master_Transmit(&hi2c1, I2C_LCD_ADDRESS,
//			 lcd_test, 2, 100)!=HAL_OK){
//		  // 필요하면 딜레이 추가 가능
//	   }
//	   HAL_Delay(1000); // 1초 대기
//  }

#if 1 // 아래 코드는 주석처리되어 있어서 실제로 실행되진 않음 (테스트용 코드)
	uint8_t value=0;
	i2c_lcd_init(); // LCD 초기화

	while(1)
	{
		move_cursor(0,0); // 0행 0열로 커서 이동
		lcd_string("Hello World!!!"); // 문자열 출력
		move_cursor(1,0); // 1행 0열로 커서 이동
		lcd_data(value + '0'); // 숫자를 아스키코드로 변환 후 출력
		value++; // 값 증가
		if(value>9)value=0; // 0~9 순환
		HAL_Delay(500); // 0.5초 대기
	}
#endif
}

// ========================= LCD 명령어 전송 함수 =========================

void lcd_command(uint8_t command){
	// 고위 nibble (상위 4bit) / 저위 nibble (하위 4bit) 분리
	uint8_t high_nibble, low_nibble;
	uint8_t i2c_buffer[4];

	high_nibble = command & 0xf0;
	low_nibble = (command<<4) & 0xf0;

	// en=1 -> en=0 으로 변화시 falling edge를 만들어야 LCD가 latch 함
	// rs=0 (명령어), rw=0 (쓰기), backlight=1

	i2c_buffer[0] = high_nibble | 0x04 | 0x08; // en=1
	i2c_buffer[1] = high_nibble | 0x00 | 0x08; // en=0
	i2c_buffer[2] = low_nibble  | 0x04 | 0x08; // en=1
	i2c_buffer[3] = low_nibble  | 0x00 | 0x08; // en=0

	// I2C 로 전송
	while(HAL_I2C_Master_Transmit(&hi2c1, I2C_LCD_ADDRESS, i2c_buffer, 4, 100)!=HAL_OK){
		// 필요하면 재시도 딜레이
	}
	return;
}

// ========================= LCD 데이터(문자) 전송 함수 =========================

void lcd_data(uint8_t data){
	uint8_t high_nibble, low_nibble;
	uint8_t i2c_buffer[4];

	high_nibble = data & 0xf0;
	low_nibble = (data<<4) & 0xf0;

	// rs=1 (데이터 모드), rw=0 (쓰기)
	i2c_buffer[0] = high_nibble | 0x05 | 0x08; // en=1
	i2c_buffer[1] = high_nibble | 0x01 | 0x08; // en=0
	i2c_buffer[2] = low_nibble  | 0x05 | 0x08; // en=1
	i2c_buffer[3] = low_nibble  | 0x01 | 0x08; // en=0

	while(HAL_I2C_Master_Transmit(&hi2c1, I2C_LCD_ADDRESS, i2c_buffer, 4, 100)!=HAL_OK){
		// 필요하면 재시도 딜레이
	}
	return;
}

// ========================= LCD 초기화 =========================

void i2c_lcd_init(void){
	lcd_command(0x33); // 초기화 과정 (데이터시트 참고)
	lcd_command(0x32); // 4-bit 모드 설정
	lcd_command(0x28); // Function set: 4-bit, 2-line, 5x8 dots
	lcd_command(DISPLAY_ON); // 화면 ON, 커서 OFF, 블링크 OFF (i2c_lcd.h 에 정의되어야 함)
	lcd_command(0x06); // Entry Mode: Increment cursor
	lcd_command(CLEAR_DISPLAY); // 화면 클리어
	HAL_Delay(2); // LCD는 클리어 후 대기 필요
}

// ========================= 문자열 출력 =========================

void lcd_string(uint8_t *str){
	// 문자열 끝(null 문자)까지 반복
	while(*str) lcd_data(*str++);
}

// ========================= 커서 이동 =========================

void move_cursor(uint8_t row, uint8_t column){
	// 커서 이동 명령어
	// 1st line : 0x80 | column
	// 2nd line : 0x80 | 0x40 | column
	lcd_command(0x80 | row<<6 | column);
	return;
}

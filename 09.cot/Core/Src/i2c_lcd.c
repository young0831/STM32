/*
 * i2c_lcd.c
 *
 *  Created on: 2019. 9. 4.
 *      Author: k
 */
#include "main.h"
#include "i2c_lcd.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "i2c_protocol.h"
#include "extern.h"
#include "stepmotor.h"

extern I2C_HandleTypeDef hi2c1;

void i2c_lcd_main(void);
void i2c_lcd_init(void);
void display_lcd(int state);

unsigned char lcd_test[4] = { '4','3', 0};

void i2c_lcd_main(void)
{
#if 1

   if(stepmotor_state == IDLE)
   {
      move_cursor(0,0);
      lcd_string("Elevator stop.  ");
      move_cursor(1,0);
      lcd_string("Have a good day.");
   }
   else if(stepmotor_state == FORWARD)
   {
      move_cursor(0,0);
      lcd_string("Elevator up.  ");
      move_cursor(1,0);
      lcd_string("Be carefull     ");
   }
   else
   {
      move_cursor(0,0);
      lcd_string("Elevator down.");
      move_cursor(1,0);
      lcd_string("Be carefull      ");
   }

#endif
}

void lcd_command(uint8_t command){

   uint8_t high_nibble, low_nibble;
   uint8_t i2c_buffer[4];
   high_nibble = command & 0xf0;
   low_nibble = (command<<4) & 0xf0;
   i2c_buffer[0] = high_nibble | 0x04 | 0x08; //en=1, rs=0, rw=0, backlight=1
   i2c_buffer[1] = high_nibble | 0x00 | 0x08; //en=0, rs=0, rw=0, backlight=1
   i2c_buffer[2] = low_nibble  | 0x04 | 0x08; //en=1, rs=0, rw=0, backlight=1
   i2c_buffer[3] = low_nibble  | 0x00 | 0x08; //en=0, rs=0, rw=0, backlight=1
#if 0
   while(HAL_I2C_Master_Transmit(&hi2c1, I2C_LCD_ADDRESS,
         i2c_buffer, 4, 100)!=HAL_OK){
      //HAL_Delay(1);
   }
#else
   i2c_start();
   //주소 전송
   if (i2c_send_byte(I2C_LCD_ADDRESS) != 0) {
      // 슬레이브가 ACK하지 않으면 에러 처리
      printf("Error sending slave address for command\n");
      i2c_stop();
      return;
   }
   //데이터 전송
   for(int i=0; i<4; i++)
   {
      i2c_send_byte(i2c_buffer[i]);
   }
   i2c_stop();
#endif
   return;
}
void lcd_data(uint8_t data){ // 1byte를 출력

   uint8_t high_nibble, low_nibble;
   uint8_t i2c_buffer[4];
   high_nibble = data & 0xf0;
   low_nibble = (data<<4) & 0xf0;
   i2c_buffer[0] = high_nibble | 0x05 | 0x08; //en=1, rs=1, rw=0, backlight=1
   i2c_buffer[1] = high_nibble | 0x01 | 0x08; //en=0, rs=1, rw=0, backlight=1
   i2c_buffer[2] = low_nibble  | 0x05 | 0x08; //en=1, rs=1, rw=0, backlight=1
   i2c_buffer[3] = low_nibble  | 0x01 | 0x08; //en=0, rs=1, rw=0, backlight=1
#if 0
   while(HAL_I2C_Master_Transmit(&hi2c1, I2C_LCD_ADDRESS,
         i2c_buffer, 4, 100)!=HAL_OK){
      //HAL_Delay(1);
   }
#else
   i2c_start();
   //주소 전송
   if (i2c_send_byte(I2C_LCD_ADDRESS) != 0) {
      // 슬레이브가 ACK하지 않으면 에러 처리
      printf("Error sending slave address for command\n");
      i2c_stop();
      return;
   }
   // 데이터 전송
   for(int i=0; i<4; i++)
   {
      i2c_send_byte(i2c_buffer[i]);
   }
   i2c_stop();
#endif
   return;
}
void i2c_lcd_init(void){ //초기화
   lcd_command(0x33);
   lcd_command(0x32);
   lcd_command(0x28);   //Function Set 4-bit mode
   lcd_command(DISPLAY_ON);
   lcd_command(0x06);   //Entry mode set
   lcd_command(CLEAR_DISPLAY);
   HAL_Delay(2);
}
void lcd_string(uint8_t *str){ //string값 찍는 함수 , null을 만날 때 까지 출력
   while(*str)lcd_data(*str++);
}
void move_cursor(uint8_t row, uint8_t column){ // 커서 이동 함수
   lcd_command(0x80 | row<<6 | column);
   return;
}

void display_lcd(int state)
{
   if(state == 0)
   {
      move_cursor(0,0);
      lcd_string("Elevator stop.  ");
      move_cursor(1,0);
      lcd_string("Have a good day.");
   }
   else if(state == 1)
   {
      move_cursor(0,0);
      lcd_string("Elevator up.  ");
      move_cursor(1,0);
      lcd_string("Be carefull     ");
   }
   else
   {
      move_cursor(0,0);
      lcd_string("Elevator down.");
      move_cursor(1,0);
      lcd_string("Be carefull      ");
   }
}











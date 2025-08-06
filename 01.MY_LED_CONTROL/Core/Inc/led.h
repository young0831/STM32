#include "main.h" //HAL_Driver GPIO 정보가 들어 있다.

#define GPIOB_ODR 0x40020414 	// GPIOB의 주소 	: 0x40020400
void led_all_on(void);			// OUTPUT DATA Resister offset 값 : 0x14
void led_all_off(void);			// GPIOB ODR의 실제 주소 : offset과 base 주소 OR 연산
void led_main(void);			// (resister가 일정한 비트정렬을 가지고 있을때)
void shift_left_ledon(void);
void shift_right_ledon(void);
void shift_left_keep_ledon(void);
void shift_right_keep_ledon(void);
void flower_on(void);
void flower_off(void);

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "main.h"		//HAL / GPIO config

#define GPIOC_IDR 0x40020810
// GPIOC base address와 Input Data Resister OR 연산 = GPIOC_IDR의 실주소
// 	  0x40020800	 	|			0x10		   = 	0x40020810
#define GPIOC_PUPDR 0x4002080C

#define BTN0  0		//PC0
#define BTN1  1		//PC1
#define BTN2  2		//PC2
#define BTN3  3		//PC3
#define BTN4  4		//PC13 demo board blue button
#define BUTTON_NUMBER  5 // BUTTON 갯수 5개

#define BUTTON_PRESS   0//active low
#define BUTTON_RELEASE 1//button을 뗀 상태

#endif

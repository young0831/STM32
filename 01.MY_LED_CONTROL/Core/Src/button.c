#include "button.h"

void button_led_toggle_test(void);
int get_button( GPIO_TypeDef *GPIO, int GPIO_Pin, int button_num);

void button_led_toggle_test(void)
{
	GPIOC->PUPDR |= 0x55; // 하위 4개 핀(Pin 0~3) PUPDR 각각 01 설정 → Pull-up 활성화

	if(get_button( GPIOC, GPIO_PIN_0, BTN0 ) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_0 );
	}
	if(get_button( GPIOC, GPIO_PIN_1, BTN1 ) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_1 );
	}
	if(get_button( GPIOC, GPIO_PIN_2, BTN2 ) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_2 );
	}
	if(get_button( GPIOC, GPIO_PIN_3, BTN3 ) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin( GPIOB, GPIO_PIN_3 );
	}
	if(get_button( GPIOC, GPIO_PIN_13, BTN4 ) == BUTTON_PRESS) //Demo blue button
	{
		HAL_GPIO_TogglePin( GPIOA, GPIO_PIN_5 );
	}
}

int get_button(GPIO_TypeDef *GPIO, int GPIO_Pin, int button_num)
// 사용 매개변수: 	GPIO 포트 		GPIO 핀 번호 			버튼 번호
{
	static unsigned char button_status[BUTTON_NUMBER] =
		{BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE};
	// static 지역 변수는 함수 호출이 끝나도 값을 유지하여 버튼 상태를 기억함

	int currtn_state;

	currtn_state = (*(unsigned int *)GPIOC_IDR & GPIO_Pin) >> button_num; // 현재 버튼 입력 상태 읽기
	// 버튼이 눌린 상태이고 이전 상태는 RELEASE였다면 (버튼이 처음 눌림)
	if (currtn_state == BUTTON_PRESS && button_status[button_num] == BUTTON_RELEASE)
	{
		HAL_Delay(60); // 노이즈 제거용 딜레이
		button_status[button_num] = BUTTON_PRESS; // 버튼 상태를 PRESS로 변경
		return BUTTON_RELEASE; // 아직 버튼이 완전히 눌렸다 떼어진 상태는 아님
	}
	// 버튼이 떼어진 상태이고 이전 상태는 PRESS였다면 (버튼이 눌렸다가 떼짐)
	else if (currtn_state == BUTTON_RELEASE && button_status[button_num] == BUTTON_PRESS)
	{
		HAL_Delay(60); // 노이즈 제거용 딜레이
		button_status[button_num] = BUTTON_RELEASE; // 상태 초기화
		return BUTTON_PRESS; // 버튼이 완전히 눌렸다 떼어진 상태로 판단
	}

	return BUTTON_RELEASE; // 기본적으로는 버튼이 눌리지 않은 상태
}

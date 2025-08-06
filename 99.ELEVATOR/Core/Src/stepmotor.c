#include "stepmotor.h"           // 스텝 모터 관련 함수, 상수 및 자료구조 정의 포함
#include "button.h"              // 버튼 제어에 필요한 함수와 정의 포함
#include "extern.h"              // 외부 전역 변수 및 외부 함수 선언 포함
#include "buzzer.h"              // 부저 제어 함수 및 정의 포함

// 여러 층을 선택할 수 있도록 하기 위한 배열 및 관련 변수들
#define MAX_FLOORS 5                          // 최대 선택 가능한 층의 개수를 5로 제한

// current_state: 시스템에서 현재 도달한 층 번호를 나타내는 외부 전역 변수(다른 파일에서 정의됨)
extern volatile uint8_t current_state;

// 전역 변수 정의
volatile uint8_t stepmotor_state = IDLE;    // 스텝 모터의 현재 상태를 저장 (예: IDLE, FORWARD, BACKWARD 등)
volatile uint8_t select_floor = 0;            // (사용되지 않는 변수일 수 있음) 선택한 층과 관련된 변수
// floor_state: 현재 이동 목표 층을 저장 (여러 층 선택시 선택 목록 배열에서 가져옴)
volatile uint8_t floor_state = 0;

// floor_selection_mode: 0이면 일반모드, 1이면 층 선택 모드로 동작 (여기서는 1로 초기화하여 시작부터 선택모드)
volatile uint8_t floor_selection_mode = 1;

volatile uint8_t selected_floors[MAX_FLOORS] = {0};  // 사용자가 선택한 층 번호들을 순서대로 저장하는 배열
volatile uint8_t floor_count = 0;             // 현재 선택된 층의 수
volatile uint8_t floor_index = 0;             // 모터 이동 시 선택된 층 목록에서 현재 목표로 할 인덱스

// dot_number 및 dotmatrix_main_test() 함수는 LED dotmatrix 출력에 사용된다고 가정

// 함수 원형 선언
void set_rpm(int rpm);             // 주어진 rpm(회전 속도)에 따라 스텝 간 딜레이를 설정하는 함수
void stepmotor_main(void);         // 모터 제어 및 층 선택 동작을 실행하는 메인 함수
int stepmotor_drive(int direction);  // 모터를 한 스텝씩 구동하고, 현재 스텝을 업데이트하는 함수

/*
 * set_rpm 함수:
 * - 1분(60,000,000µs)을 1바퀴 회전에 필요한 스텝 수(4096)와 rpm으로 나누어
 *   각 스텝 사이의 딜레이를 결정한다.
 */
void set_rpm(int rpm) // rpm : 1~13
{
   delay_us(60000000 / 4096 / rpm);  // 계산된 시간(마이크로초) 만큼 지연
   // 참고: 13rpm의 경우 약 1126µs 정도의 딜레이가 발생하도록 계산됨.
}

/*
 * stepmotor_main 함수:
 * - RTOS 환경 등에서 주기적으로 호출되어 스텝 모터의 동작 및 층 선택을 처리한다.
 * - IDLE 상태에서는 Blue push button(BTN4, GPIOC PIN13)을 통해
 *   선택 모드로 진입하거나, 이미 선택 모드인 경우 선택 완료 후 이동을 시작한다.
 * - 선택 모드에서는 BTN0~BTN3를 이용해 여러 층을 선택(또는 취소)하며,
 *   선택될 때마다 LED 및 dotmatrix에 현재 선택된 층 정보를 표시한다.
 * - 이동 상태(FORWARD, BACKWARD)에서는 현재 위치(current_state)가 목표 층(floor_state)에 도달하면
 *   beep()를 발생시키고, 선택 목록의 다음 층으로 이동을 진행한다.
 */
void stepmotor_main(void)
{
   switch(stepmotor_state)
   {
   case IDLE:
      // Blue push button(BTN4, GPIOC PIN13)이 눌리면(버튼 입력 감지)
      if (get_button(GPIOC, GPIO_PIN_13, BTN4) == BUTTON_PRESS)
      {
         if(floor_selection_mode == 0)
         {
            // 일반 모드인 경우, 버튼을 눌러 선택 모드로 진입
            floor_selection_mode = 1;     // 선택 모드 활성화
            floor_count = 0;              // 이전에 선택된 층 목록 초기화
            floor_index = 0;              // 목표 인덱스 초기화
            // 필요시 LED나 dotmatrix로 선택 모드 진입을 사용자에게 표시할 수 있음.
         }
         else  // 이미 선택 모드인 경우, 다시 버튼을 누르면 선택 완료 처리
         {
            floor_selection_mode = 0;     // 선택 모드 종료
            // 만약 하나 이상의 층이 선택되었다면 선택 목록의 첫 번째 층을 목표로 설정
            if(floor_count > 0)
            {
               floor_state = selected_floors[0];  // 목표 층을 첫번째 선택으로 설정
               // 현재 층(current_state)과 목표 층(floor_state)을 비교하여 모터 이동 방향 결정
               if(current_state < floor_state)
                  stepmotor_state = FORWARD;
               else if(current_state > floor_state)
                  stepmotor_state = BACKWARD;
               // 같으면 이미 도달한 것으로 추가 동작 없이 IDLE 유지
            }
         }
      }

      // 선택 모드(floor_selection_mode == 1)인 경우
      if(floor_selection_mode == 1)
      {
         dotmatrix_main_test2();  // 선택 모드임을 dotmatrix에 표시

         // BTN0: 1층 선택 또는 취소 처리
         if(get_button(GPIOC, GPIO_PIN_0, BTN0) == BUTTON_PRESS)
         {
             int found = 0;  // 1층이 이미 선택되었는지 여부를 검사하는 플래그
             for (int i = 0; i < floor_count; i++)
             {
                  if (selected_floors[i] == 1)
                  {
                     found = 1; // 1층이 이미 배열에 존재함
                     // 배열 내에서 1층 값을 제거(이후 값들을 앞으로 당김)
                     for (int j = i; j < floor_count - 1; j++)
                         selected_floors[j] = selected_floors[j + 1];
                     floor_count--;  // 선택된 층의 수 1 감소
                     break;
                  }
             }
             if (!found)  // 1층이 선택되지 않은 상태면
             {
                  if (floor_count < MAX_FLOORS)
                     selected_floors[floor_count++] = 1;  // 1층 추가
             }
             led_all_off();  // 모든 LED 끔
             // 만약 배열의 마지막 선택된 값이 1이면, 1층 LED 표시 (예: led_one_on())
             if (floor_count > 0 && selected_floors[floor_count - 1] == 1)
                  led_one_on();
             // dot_number 변수에 마지막으로 선택된 층 번호를 저장하여 dotmatrix에 표시 준비
             dot_number = (floor_count > 0 ? selected_floors[floor_count - 1] : 0);
             dotmatrix_main_test();  // dotmatrix에 현재 선택된 층 출력
         }
         // BTN1: 2층 선택 또는 취소 처리 (동일한 방식)
         else if(get_button(GPIOC, GPIO_PIN_1, BTN1) == BUTTON_PRESS)
         {
             int found = 0;
             for (int i = 0; i < floor_count; i++)
             {
                  if (selected_floors[i] == 2)
                  {
                     found = 1;
                     for (int j = i; j < floor_count - 1; j++)
                         selected_floors[j] = selected_floors[j + 1];
                     floor_count--;
                     break;
                  }
             }
             if (!found)
             {
                  if (floor_count < MAX_FLOORS)
                     selected_floors[floor_count++] = 2;  // 2층 추가
             }
             led_all_off();
             if (floor_count > 0 && selected_floors[floor_count - 1] == 2)
                  led_two_on();
             dot_number = (floor_count > 0 ? selected_floors[floor_count - 1] : 0);
             dotmatrix_main_test();
         }
         // BTN2: 3층 선택 또는 취소 처리
         else if(get_button(GPIOC, GPIO_PIN_2, BTN2) == BUTTON_PRESS)
         {
             int found = 0;
             for (int i = 0; i < floor_count; i++)
             {
                  if (selected_floors[i] == 3)
                  {
                     found = 1;
                     for (int j = i; j < floor_count - 1; j++)
                         selected_floors[j] = selected_floors[j + 1];
                     floor_count--;
                     break;
                  }
             }
             if (!found)
             {
                  if (floor_count < MAX_FLOORS)
                     selected_floors[floor_count++] = 3;  // 3층 추가
             }
             led_all_off();
             if (floor_count > 0 && selected_floors[floor_count - 1] == 3)
                  led_three_on();
             dot_number = (floor_count > 0 ? selected_floors[floor_count - 1] : 0);
             dotmatrix_main_test();
         }
         // BTN3: 4층 선택 또는 취소 처리
         else if(get_button(GPIOC, GPIO_PIN_3, BTN3) == BUTTON_PRESS)
         {
             int found = 0;
             for (int i = 0; i < floor_count; i++)
             {
                  if (selected_floors[i] == 4)
                  {
                     found = 1;
                     for (int j = i; j < floor_count - 1; j++)
                         selected_floors[j] = selected_floors[j + 1];
                     floor_count--;
                     break;
                  }
             }
             if (!found)
             {
                  if (floor_count < MAX_FLOORS)
                     selected_floors[floor_count++] = 4;  // 4층 추가
             }
             led_all_off();
             if (floor_count > 0 && selected_floors[floor_count - 1] == 4)
                  led_all_on();  // 4층 선택 시 전체 LED 켜기
             dot_number = (floor_count > 0 ? selected_floors[floor_count - 1] : 0);
             dotmatrix_main_test();
         }
      }
      break;

   case FORWARD:
      // FORWARD 상태: 모터를 전진(앞으로) 한 스텝씩 구동
      stepmotor_drive(FORWARD);         // 한 스텝 전진
      set_rpm(13);                      // 13rpm 기준 스텝 간 딜레이 설정
      dot_number = current_state;       // 현재 도달한 층을 dot_number로 설정하여 dotmatrix 표시
      dotmatrix_main_test();
      // 목표 층(floor_state)에 도달한 경우
      if(current_state == floor_state)
      {
         beep(3);                     // 도착 알림 음 발생
         osDelay(10);                 // 짧은 딜레이 후
         floor_index++;               // 다음 선택된 층으로 이동하기 위해 인덱스 증가
         if(floor_index < floor_count)
         {
            // 다음 선택된 층을 목표로 설정
            floor_state = selected_floors[floor_index];
            // 현재 층과 비교하여 이동 방향 결정
            if(current_state < floor_state)
               stepmotor_state = FORWARD;
            else if(current_state > floor_state)
               stepmotor_state = BACKWARD;
         }
         else
         {
            // 모든 선택된 층을 이동한 경우, 모터 상태를 IDLE로 전환
            stepmotor_state = IDLE;
         }
      }
      break;

   case BACKWARD:
      // BACKWARD 상태: 모터를 후진(뒤로) 한 스텝씩 구동
      stepmotor_drive(BACKWARD);        // 한 스텝 후진
      set_rpm(13);                      // 스텝 간 딜레이 설정
      dot_number = current_state;       // dotmatrix 출력을 위해 현재 층 할당
      dotmatrix_main_test();
      if(current_state == floor_state)
      {
         beep(3);                     // 목표 도달 시 알림 음 발생
         osDelay(10);
         floor_index++;               // 다음 선택된 층으로 이동하기 위해 인덱스 증가
         if(floor_index < floor_count)
         {
            // 다음 목표 층 설정
            floor_state = selected_floors[floor_index];
            // 현재 층과 비교하여 이동 방향 결정
            if(current_state < floor_state)
               stepmotor_state = FORWARD;
            else if(current_state > floor_state)
               stepmotor_state = BACKWARD;
         }
         else
         {
            // 모든 선택 층 이동 완료 시 IDLE 상태 전환
            stepmotor_state = IDLE;
         }
      }
      break;
   }
}

/*
 * stepmotor_drive 함수:
 * - 모터의 한 스텝 진행을 위해 8단계 시퀀스에 따라
 *   GPIOC의 IN1 ~ IN4 핀에 출력 값을 보낸다.
 * - direction 인자가 FORWARD이면 한 단계 증가, BACKWARD이면 감소시키며, 0부터 7까지 순환하도록 한다.
 */
int stepmotor_drive(int direction)
{
   static int step = 0;  // 현재 스텝 값을 저장하는 정적 변수 (범위: 0~7)
   switch(step)
   {
   case 0:
      HAL_GPIO_WritePin(GPIOC, IN1_Pin, 1);  // 단계 0: IN1 활성, 나머지 비활성
      HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
      break;
   case 1:
      HAL_GPIO_WritePin(GPIOC, IN1_Pin, 1);  // 단계 1: IN1과 IN2 활성, IN3, IN4 비활성
      HAL_GPIO_WritePin(GPIOC, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
      break;
   case 2:
      HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);  // 단계 2: IN2 활성, 나머지 비활성
      HAL_GPIO_WritePin(GPIOC, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
      break;
   case 3:
      HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);  // 단계 3: IN2와 IN3 활성, IN1, IN4 비활성
      HAL_GPIO_WritePin(GPIOC, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOC, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
      break;
   case 4:
      HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);  // 단계 4: IN3 활성, 다른 핀은 비활성
      HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOC, IN4_Pin, 0);
      break;
   case 5:
      HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);  // 단계 5: IN3와 IN4 활성, IN1, IN2 비활성
      HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOC, IN4_Pin, 1);
      break;
   case 6:
      HAL_GPIO_WritePin(GPIOC, IN1_Pin, 0);  // 단계 6: IN4 활성, 나머지 비활성
      HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN4_Pin, 1);
      break;
   case 7:
      HAL_GPIO_WritePin(GPIOC, IN1_Pin, 1);  // 단계 7: IN1과 IN4 활성, IN2와 IN3 비활성
      HAL_GPIO_WritePin(GPIOC, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOC, IN4_Pin, 1);
      break;
   }

   // 업데이트: direction이 FORWARD면 스텝 증가, BACKWARD면 감소
   if(direction == FORWARD)
   {
      step++;           // FORWARD이면 step 값을 1 증가
      step %= 8;        // 8 이상이 되면 0~7 사이로 순환
   }
   else  // BACKWARD
   {
      step--;           // BACKWARD이면 step 값을 1 감소
      if(step < 0)
         step = 7;      // 음수가 되면 7로 순환시킴
   }
   return step;          // 업데이트된 스텝 값을 반환
}

#include "dht11.h"
#include "timer.h"
#include <stdint.h>
// us_count : 마이크로초 단위로 DHT11 신호의 HIGH/LOW 지속시간을 측정하기 위한 변수.
// dht11_state : 센서와의 통신 상태를 나타내며, 정상(OK), 타임아웃(TIMEOUT), 데이터 오류(VALUE_ERROR) 등을 포함.
uint8_t us_count = 0;
enum state_define dht11_state = OK;

 // init_dht11()
 //  - DHT11 센서의 데이터 라인을 초기 상태로 설정합니다.
 //  - 센서는 기본적으로 풀업된 상태(HIGH)여야 하므로, 해당 핀을 출력으로 설정하고 HIGH를 출력합니다.
 //  - 외부 풀업(약 10kΩ) 또는 내부 풀업 활성화를 통해 안정적인 동작을 보장할 수 있습니다.
void init_dht11(void)
{

	GPIO_DHT11_MODER &= ~(1 << 0);   // 데이터 핀(PA0)을 출력 모드로 설정
	GPIO_DHT11_MODER |= (1 << 0);
	GPIO_DHT11_ODR |= (1 << 0);		//데이터 출력 활성화
   //DHT11_GPIO_Port |= 1 << DHT_PIN_NUM;   // 출력을 HIGH로 유지하여 기본 풀업 상태를 만듦
}

/*
 * dht11_main()
 *  - DHT11 센서와의 통신을 수행하여 온도 및 습도 데이터를 읽어옵니다.
 *  - 아래 단계에 따라 통신을 진행합니다.
 *
 * [Step 1: Start Signal]
 *  - MCU는 센서에 시작 신호를 보냅니다.
 *    1. 데이터 라인을 출력 모드로 설정하고 HIGH로 유지 (초기 상태)
 *    2. 100ms 대기 후, 데이터 라인을 LOW로 20ms 동안 유지 (datasheet 권장: 최소 18ms)
 *    3. 다시 데이터 라인을 HIGH로 만들고 입력 모드로 전환하여 센서 응답 대기
 *
 * [Step 2: Sensor Response]
 *  - 센서는 시작 신호에 응답하여 아래와 같은 순서로 신호를 보냅니다.
 *    1. 초기에는 MCU가 보낸 HIGH 신호를 감지 후, 센서가 80us 내외의 LOW 신호를 보냄.
 *       -> 만약 50us 이상 HIGH가 지속되면 타임아웃 오류 발생.
 *    2. 이후 센서는 LOW 신호를 80us 내외(여유를 두어 100us) 동안 유지 후 HIGH로 전환.
 *    3. 마지막으로 HIGH 상태도 80us 내외(여유를 두어 100us)로 유지 후 데이터 전송 준비.
 *
 * [Step 3: Data Bit Reception]
 *  - 센서는 총 40비트(5바이트)의 데이터를 전송합니다.
 *  - 각 비트는 다음과 같이 전송됩니다:
 *    - 50us 동안 LOW 펄스 (비트 전환의 시작)
 *    - 이어서 HIGH 펄스가 발생하는데, 지속시간에 따라 비트 값이 결정됨:
 *         * HIGH 펄스가 약 26~28us이면 '0'
 *         * HIGH 펄스가 약 70us이면 '1'
 *    - 코드에서는 HIGH 펄스의 길이가 40us 미만이면 '0', 그 이상이면 '1'로 판단합니다.
 *
 * [Step 4: Checksum Validation & 출력]
 *  - 전송된 5바이트 중 마지막 바이트는 체크섬으로, 앞 4바이트의 합과 같아야 합니다.
 *  - 체크섬이 일치하지 않으면 VALUE_ERROR 상태로 처리합니다.
 *  - 정상 통신이면 온도와 습도를 시리얼 출력합니다.
 *
 * 각 단계마다 타이밍을 미세하게 측정하며, datasheet 기준 값에 여유를 두어 TIMEOUT 검사를 수행합니다.
 */

// DHT11 센서와 통신해 온/습도 데이터를 읽어오는 메인 루프 함수.
// - 무한 루프 내부에서 매번 Start 신호를 보내고, DHT11의 응답을 확인한 뒤
//   40비트를 수신하여 온도/습도를 계산한다.
void dht11_main(void)
{
   // 센서에서 수신한 5바이트 데이터 배열:
   // [0]: 습도 정수, [1]: 습도 소수, [2]: 온도 정수, [3]: 온도 소수, [4]: 체크섬
   uint8_t data[5] = { 0, };

   init_dht11();

   while (1)
   {
      memset(data, 0, sizeof(data));   // 데이터 배열 초기화
      dht11_state = OK;

      // ===== [Step 1: Start Signal] =====
      init_dht11();      // 데이터 라인을 HIGH로 초기화 (풀업 상태)
      delay_us(100000);      // 안정화를 위해 대기

      // DHT_PORT &= ~(1 << DHT_PIN_NUM);   // 데이터 라인을 LOW로 내려서 시작 신호 전송
      GPIO_DHT11_ODR &= ~(1 << 0);
      delay_us(20000);                  // 최소 datasheet 권장 18ms 이상 LOW 유지 (여기서는 20ms)

      //DHT_PORT |= 1 << DHT_PIN_NUM;      // 다시 HIGH로 전환
      GPIO_DHT11_ODR |= 1 << 0;
      //DHT_DDR &= ~(1 << DHT_PIN_NUM);      // 핀을 입력 모드로 전환하여 센서 응답 수신
      GPIO_DHT11_MODER &= ~(1 << 0);
      delay_us(1);                  // 짧은 지연

       // ===== [Step 2: Sensor Response Check Part 1] =====
       // 센서가 시작 신호에 대해 LOW 응답을 시작하기 전까지 HIGH 상태가 지속되면 에러 처리.
      us_count = 0;
      while ( (GPIO_DHT11_IDR & 1 << 0) >> 0 )   // 아직도 HIGH인지 check
      {
         delay_us(2);
         us_count += 2;
         if (us_count > 50)   // 50us 이상 HIGH가 지속되면 타임아웃 발생
         {
        	 printf("%d", us_count);
        	 printf(" 0\n");

            dht11_state = TIMEOUT;
            break;
         }
      }

      // ===== [Step 2: Sensor Response Check Part 2] =====
      // 센서가 LOW 상태로 전환되었으면, LOW 유지 기간(약 80us, 여기서는 여유를 두어 100us) 체크
      if (dht11_state == OK)
      {
         us_count = 0;
         while ( !((GPIO_DHT11_IDR & 1 << 0) >> 0) )
         {
            delay_us(2);
            us_count += 2;
            if (us_count > 100)   // LOW 신호가 100us 이상 지속되지 않으면 타임아웃
            {
            	printf("1");
               dht11_state = TIMEOUT;
               break;
            }
         }
      }

      // ===== [Step 2: Sensor Response Check Part 3] =====
      // 이후 센서가 HIGH 상태로 전환되면, HIGH 유지 시간(약 80us, 여유를 두어 100us) 체크
      if (dht11_state == OK)
      {
         us_count = 0;
         while ( (GPIO_DHT11_IDR & 1 << 0) >> 0 )
         {
            delay_us(2);
            us_count += 2;
            if (us_count > 100)   // HIGH 신호가 너무 길면 타임아웃 처리
            {
            	printf("2");
               dht11_state = TIMEOUT;
               break;
            }
         }
      }
      // 이 시점에서 dht11_state가 OK이면 시작 신호와 센서 응답이 정상적으로 이루어짐.

      // ===== [Step 3: Data Bit Reception] =====
      // 센서가 40비트 데이터를 순차적으로 전송합니다.
      // 각 비트 전송:
      //   - 50us LOW 펄스로 시작한 후 HIGH 펄스 길이로 비트 값 결정
      //       * HIGH 펄스가 약 26~28us이면 '0'
      //       * HIGH 펄스가 약 70us이면 '1'
      // 코드에서는 HIGH 펄스 지속시간이 40us 미만이면 '0', 40us 이상이면 '1'로 판정합니다.
      if (dht11_state == OK)
      {
         for (int i = 0; i < 5; i++) // 5바이트 데이터 수신
         {
            uint8_t pulse[8] = {0, };
            // 한 바이트는 8비트로 구성 (MSB부터 LSB 순으로 읽음)
            for (int j = 7; j >= 0; j--)
            {
                // --- LOW 펄스 체크 (각 비트 시작: 약 50us) ---
               us_count = 0;
               while ( !((GPIO_DHT11_IDR & 1 << 0) >> 0) )
               {
                  delay_us(2);
                  us_count += 2;
                  if (us_count > 70)   // LOW 펄스가 spec(50us)보다 현저히 길면 에러 처리 (여유를 두어 70us)
                  {
                	  printf("3");
                     dht11_state = TIMEOUT;
                     i = 5;   // 바깥 for 문 탈출을 위한 변수 재설정
                     j = -1;
                     break;
                  }
               }

               // --- HIGH 펄스 체크 ---
               // LOW 펄스 종료 후, HIGH 펄스의 길이를 측정하여 비트 '0' 또는 '1'을 결정
               if (dht11_state == OK)
               {
                  us_count = 0;
                  while ( (GPIO_DHT11_IDR & 1 << 0) >> 0 )   // HIGH 일동안 반복
                  {
                     delay_us(2);
                     us_count += 2;
                     if (us_count > 90)   // HIGH 펄스가 90us 이상이면 타임아웃 (비트 '1'은 약 70us, 여유 고려)
                     {
                    	printf("4");
                        dht11_state = TIMEOUT;
                        i = 5;
                        j = -1;      // for문 전체 탈출
                        break;
                     }
                  }
               }
               // HIGH 펄스의 길이에 따라 비트 값을 결정: 40us 미만이면 '0', 이상이면 '1'
               if (dht11_state == OK)
               {
                  if (us_count < 40) {
                     pulse[j] = 0;
                  }
                  else {
                     pulse[j] = 1;
                  }
               }
            }

            // 한 바이트의 8비트가 정상적으로 읽혔다면 pulse 배열을 하나의 바이트로 변환
            if (dht11_state == OK)   // pulse를 40개를 정상 처리했으면
            {
               data[i] = pulse[0] << 0 | pulse[1] << 1 | pulse[2] << 2 | pulse[3] << 3 |
                       pulse[4] << 4 | pulse[5] << 5 | pulse[6] << 6 | pulse[7] << 7;
            }
         }

         // ===== [Step 4: Checksum 검증] =====
         // 전송된 5번째 바이트(data[4])는 체크섬으로, 앞 4바이트의 합과 같아야 합니다.
         if (dht11_state == OK)
         {
            if (data[4] != data[0] + data[1] + data[2] + data[3])
            {
               dht11_state = VALUE_ERROR;
            }
         }
         delay_us(50);   // 데이터 전송 후 약간의 여유 시간 (spec: 50us, 여기서는 60us 여유)
      }

      // ===== [Step 5: 결과 출력 및 안정화 대기] =====
      if (dht11_state == OK)
      {
         printf("temp : %d.%d\n", data[2], data[3]);
         printf("humi : %d.%d\n", data[0], data[1]);
      }
      else
      {
         printf("error code : %d\n", dht11_state);
      }
      delay_us(2000000);   // 센서 안정화 및 다음 측정을 위한 대기 시간

   }
}

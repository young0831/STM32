#include "main.h"

extern TIM_HandleTypeDef htim2;
extern uint8_t rx_data;


extern t_print o_prt;
extern UART_HandleTypeDef huart2;
extern volatile int TIM11_1ms_counter;
extern uint16_t SUM, RH, TEMP;
extern uint8_t keypadScan();
extern int get_button( GPIO_TypeDef *GPIO, int GPIO_Pin, int button_num);


extern void led_main(void);
extern void button_led_toggle_test(void);
extern void ds1302_main(void);
extern void pc_command_processing(void);
extern void led_all_on(void);
extern void led_all_off(void);
extern void set_rtc(char *date_time);
extern void delay_us(int us);
extern void dht11_main(void);
extern void flash_main();
extern void flash_set_time(void);
extern void i2c_lcd_main(void);
extern void i2c_lcd_init(void);
extern void move_cursor(uint8_t row, uint8_t column);
extern void lcd_string(uint8_t *str);
extern void buzzer_main();
extern void servo_motor_main(void);
extern void dotmatrix_main(void);
extern void dotmatrix_main_test();
extern void dotmatrix_main_func(void);
extern void insert_queue(unsigned char value);
extern int queue_empty(void);
extern int queue_full(void);
extern void insert_queue(unsigned char value);
extern unsigned char read_queue();
extern void keypadInit();
extern int multiple_number(char a, char b, char c);

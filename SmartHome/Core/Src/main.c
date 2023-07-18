/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c-lcd.h"
#include "KeyPad.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//hi2c1 --> i2c display
//htim1 (ch1) --> ic hcsr04 apertura
//htim3 (ch2) --> pwm servo motore
//htim4 --> timer cancello
//htim5 --> timer luce interna
//htim9 --> timer motore stepper
//htim10 --> timer porta
//htim11 (ch1) --> ic hcsr04 chiusura

/* VARIABILI NUMERICHE */
/* TIMER */
#define FROM_SECOND_TO_MICROSECOND 1000000
#define FROM_SECOND_TO_DECIMILLISECOND 10000
#define AUTORELOAD_SERVO_MOTOR 500 //(dms)
#define PERIOD_DISPLAY_INATTIVO 500 //(ms)
#define PERIOD_DISPLAY_CONFIGURAZIONE 2000 //(ms)
int time_open_sliding_gate = 6;	//s
int time_open_door = 6;	//s
int time_on_internal_light = 10;	//s
uint32_t start_timer_display = 0;

//NUMPAD
#define password_length 4
char password[] = {'1', '5', '9', 'D'};
char password_inserita[password_length];
int index_password_inserita = 0;
#define data_lenght 2
char configurazione_inserita[data_lenght];
char single_configuration;
int index_configurazione_inserita = 0;

//SLIDING GATE
int stepsperrev;
int numofsequence;
float angleperseq;
float angle;
int rpm;
int numberofsequences;
int seq;
int step;
int direction;

//HCSR04
uint32_t ic_val1_close_position = 0;
uint32_t ic_val2_close_position = 0;
uint8_t is_first_captured_close_position = 0;
uint8_t distance_close_position = 7;
uint32_t ic_val1_open_position = 0;
uint32_t ic_val2_open_position = 0;
uint8_t is_first_captured_open_position = 0;
uint8_t distance_open_position = 7;

/* VARIABILI DI STATO */
//NUMPAD
typedef enum {
	INATTIVO,
	CONFIGURAZIONE,
	ALLARME,
	CONFIGURAZIONE_CANCELLO,
	CONFIGURAZIONE_LUCE,
	CONFIGURAZIONE_PORTA,
	END_CONFIGURAZIONE
} numpad_state;
numpad_state numpad = INATTIVO;

//LED
typedef enum {
	SPENTO,
	ACCESO
} led_state;
led_state internal_light = SPENTO;

//ALARM
typedef enum {
	ALARM_SPENTO,
	ALARM_DISATTIVO,
	ALARM_ATTIVO
} alarm_state;
alarm_state alarm = ALARM_DISATTIVO;

//DOOR
typedef enum {
	SERVO_CHIUSO,
	SERVO_APERTURA,
	SERVO_APERTO,
	SERVO_CHIUSURA
} door_state;
door_state door = SERVO_CHIUSO;

//SLIDING GATE
typedef enum {
	CHIUSO,
	IN_APERTURA,
	APERTO,
	IN_CHIUSURA
} sliding_gate_state;
sliding_gate_state sliding_gate = CHIUSO;

//HCSR04
typedef enum {
	DELAY,
	MISURA
} hc_sr04_state;
hc_sr04_state hc_sr04_close_position = DELAY;
hc_sr04_state hc_sr04_open_position = DELAY;

typedef enum {
	SETUP,
	TIMER
}timer_state;
timer_state timer_1 = SETUP;
timer_state timer_4 = SETUP;
timer_state timer_5 = SETUP;
timer_state timer_9 = SETUP;
timer_state timer_10 = SETUP;
timer_state timer_11 = SETUP;

/* VARIABILI UART */
//numero di byte scambiati per pacchetto
#define num_byte 7

//char inviati
#define sof 'a'
#define eof 'r'
#define padding 's'

#define get_info 'b'
#define send_value 'c'
#define set_parameter 'd'

#define cancello 'e'
#define luce_interna 'g'
#define porta 'i'
#define allarme 'j'
#define configurazione_cancello 'l'
#define configurazione_luce_interna 'n'
#define configurazione_porta 'o'

typedef enum {
	IN_ATTESA_SOF,
	IN_ATTESA_OPERAZIONE,
	IN_ATTESA_IDENTIFICATIVO,
	IN_ATTESA_VALORE_1,
	IN_ATTESA_VALORE_2,
	IN_ATTESA_VALORE_3,
	IN_ATTESA_EOF
} uart_state;
uart_state uart = IN_ATTESA_SOF;

char send_byte[num_byte];
char receive_byte[num_byte];
int index_byte = 0;

uint8_t data_received;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM11_Init(void);
/* USER CODE BEGIN PFP */
/* INIZIALIZZAZIONE */
void init_peripherals();
void init_lcd();
void init_keypad();
void init_step_motor();
/* DISPLAY */
void set_display_configuration();
void set_display_state();
/* NUMPAD */
void read_keypad();
void keypad_alarm();
void keypad_configuration();
void keypad_sliding_gate();
void keypad_internal_light();
void keypad_door();
void switch_to_inattivo();
/* ELABORAZIONE */
void open_door();
void process_door();
void stepper_half_drive (int actual_step);
void HCSR04_Read (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void open_sliding_gate();
void close_sliding_gate();
void stop_sliding_gate();
void start_closing_sliding_gate();
void change_direction_sliding_gate();
void start_sampling_hcsr04(TIM_HandleTypeDef* htim);
void led_on();
void led_off();
void alarm_on();
void alarm_off();
/* TIMER */
void start_timer(TIM_HandleTypeDef* htim);
void stop_timer(TIM_HandleTypeDef* htim);
void set_autoreload(TIM_HandleTypeDef* htim, uint32_t autoreload);
void change_autoreload_timer_start(TIM_HandleTypeDef* htim, uint32_t autoreload);
void set_pwm(TIM_HandleTypeDef* htim, int update, uint32_t pwm);
/* UART */
void prepare_data_sliding_gate(char operazione);
void prepare_data_internal_light(char operazione);
void prepare_data_door(char operazione);
void prepare_data_alarm(char operazione);
void prepare_data_configuration_sliding_gate(char operazione);
void prepare_data_configuration_internal_light(char operazione);
void prepare_data_configuration_door(char operazione);
void send_data();
int process_data(char c);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void set_display_configuration() {
	lcd_clear();
	lcd_put_cur(0, 0);
	lcd_send_string("C:1 L:2 P:3 A:4");
	lcd_put_cur(1, 0);
	lcd_send_string("ESC:7");
	HAL_GPIO_WritePin(LED_CONFIGURATION_GPIO_Port, LED_CONFIGURATION_Pin, GPIO_PIN_SET);
}

void set_display_state() {
	lcd_clear();
	lcd_put_cur(0, 0);
	lcd_send_string("C:");
	switch(sliding_gate){
	case SPENTO:
		lcd_send_data('0');
		break;
	case ACCESO:
		lcd_send_data('1');
		break;
	case APERTO:
		lcd_send_data('2');
		break;
	case IN_CHIUSURA:
		lcd_send_data('3');
		break;
	}
	lcd_send_string(" P:");
	switch(door){
	case SERVO_CHIUSO:
		lcd_send_data('0');
		break;
	case SERVO_APERTO:
		lcd_send_data('1');
		break;
	case SERVO_APERTURA:
	case SERVO_CHIUSURA:
		break;
	}
	lcd_send_string(" A:");
	switch(alarm){
	case ALARM_SPENTO:
		lcd_send_data('0');
		break;
	case ALARM_DISATTIVO:
		lcd_send_data('1');
		break;
	case ALARM_ATTIVO:
		lcd_send_data('2');
		break;
	}
	lcd_put_cur(1, 0);
	lcd_send_string("L:");
	switch(internal_light){
	case SPENTO:
		lcd_send_data('0');
		break;
	case ACCESO:
		lcd_send_data('1');
		break;
	}
}

void read_keypad() {
	switch(numpad) {
	case ALLARME:
		keypad_alarm();
		break;
	case CONFIGURAZIONE:
		keypad_configuration();
		break;
	case CONFIGURAZIONE_CANCELLO:
		keypad_sliding_gate();
		break;
	case CONFIGURAZIONE_LUCE:
		keypad_internal_light();
		break;
	case CONFIGURAZIONE_PORTA:
		keypad_door();
		break;
	case INATTIVO:
	case END_CONFIGURAZIONE:
		break;
	}
}

void keypad_alarm() {
	lcd_clear();
	lcd_put_cur(0, 0);
	lcd_send_string("PIN: ");
	for (int i = 0; i < index_password_inserita; i++) {
		lcd_send_data('*');
	}
	char c = KeyPad_WaitForKeyGetChar(0);
	if (c != 0) {
		if (numpad == ALLARME) {
			lcd_send_data(c);
			if (index_password_inserita < password_length) {
				password_inserita[index_password_inserita] = c;
				index_password_inserita++;
			}
			if (index_password_inserita == password_length) {
				if (strncmp(password, password_inserita, password_length) == 0) {
					if (alarm == ALARM_ATTIVO || alarm == ALARM_DISATTIVO) {
						alarm_off();
					} else {
						if (alarm == ALARM_SPENTO) {
							alarm_on();
						}
					}
					switch_to_inattivo();
				}
				for (int i = 0; i < password_length; i++) {
					password_inserita[i] = '\0';
				}
				index_password_inserita = 0;
			}
		}
	}
}

void keypad_configuration() {
	char c = KeyPad_WaitForKeyGetChar(0);
	switch (c) {
	case '1':
		numpad = CONFIGURAZIONE_CANCELLO;
		lcd_clear();
		lcd_put_cur(0, 0);
		lcd_send_string("1 - 6 s: ");
		lcd_put_cur(1, 0);
		break;
	case '2':
		numpad = CONFIGURAZIONE_LUCE;
		lcd_clear();
		lcd_put_cur(0, 0);
		lcd_send_string("10 - 60 s: ");
		lcd_put_cur(1, 0);
		break;
	case '3':
		numpad = CONFIGURAZIONE_PORTA;
		lcd_clear();
		lcd_put_cur(0, 0);
		lcd_send_string("1 - 6 s: ");
		lcd_put_cur(1, 0);
		break;
	case '4':
		numpad = ALLARME;
		lcd_clear();
		lcd_put_cur(0, 0);
		lcd_send_string("PIN: ");
		break;
	case '5':
		switch_to_inattivo();
	}
}

void keypad_sliding_gate() {
	char c = KeyPad_WaitForKeyGetChar(0);
	if (c != 0) {
		if (c >= '1' && c <= '6') {
			time_open_sliding_gate = c - '0';
			set_autoreload(&htim4, time_open_sliding_gate * FROM_SECOND_TO_DECIMILLISECOND);
			lcd_clear();
			lcd_put_cur(0, 0);
			lcd_send_string("C_N: ");
			lcd_send_data(c);
			numpad = END_CONFIGURAZIONE;
			start_timer_display = HAL_GetTick();
			prepare_data_configuration_sliding_gate(set_parameter);
		} else {
			switch_to_inattivo();
		}
	}
}

void keypad_internal_light() {
	char c = KeyPad_WaitForKeyGetChar(0);
	if (c != 0) {
		lcd_send_data(c);
		if (index_configurazione_inserita < data_lenght) {
			configurazione_inserita[index_configurazione_inserita] = c;
			index_configurazione_inserita++;
		}
		if (index_configurazione_inserita == 2) {
			if (configurazione_inserita[0] >= '1' && configurazione_inserita[0] <= '5') {
				if (configurazione_inserita[1] >= '0' && configurazione_inserita[1] <= '9') {
					time_on_internal_light = (configurazione_inserita[0] - '0') * 10 + (configurazione_inserita[1] - '0');
					set_autoreload(&htim5, time_on_internal_light * FROM_SECOND_TO_MICROSECOND);
					lcd_clear();
					lcd_put_cur(0, 0);
					lcd_send_string("L_N: ");
					lcd_send_data(configurazione_inserita[0]);
					lcd_send_data(configurazione_inserita[1]);
					numpad = END_CONFIGURAZIONE;
					start_timer_display = HAL_GetTick();
					prepare_data_configuration_internal_light(set_parameter);
				} else {
					switch_to_inattivo();
				}
			} else {
				if (configurazione_inserita[0] == '6') {
					if (configurazione_inserita[1] == '0') {
						time_on_internal_light = (configurazione_inserita[0] - '0') * 10 + (configurazione_inserita[1] - '0');
						set_autoreload(&htim5, time_on_internal_light * FROM_SECOND_TO_MICROSECOND);
						lcd_clear();
						lcd_put_cur(0, 0);
						lcd_send_string("L_N: ");
						lcd_send_data(configurazione_inserita[0]);
						lcd_send_data(configurazione_inserita[1]);
						numpad = END_CONFIGURAZIONE;
						start_timer_display = HAL_GetTick();
						prepare_data_configuration_internal_light(set_parameter);
					} else {
						switch_to_inattivo();
					}
				} else {
					switch_to_inattivo();
				}
			}
			for (int i = 0; i < data_lenght; i++) {
				configurazione_inserita[i] = '\0';
			}
			index_configurazione_inserita = 0;
		}
	}
}

void keypad_door() {
	char c = KeyPad_WaitForKeyGetChar(0);
	if (c != 0) {
		if (c >= '1' && c <= '6') {
			time_open_door = c - '0';
			lcd_clear();
			lcd_put_cur(0, 0);
			lcd_send_string("P_N: ");
			lcd_send_data(c);
			numpad = END_CONFIGURAZIONE;
			start_timer_display = HAL_GetTick();
			prepare_data_configuration_door(set_parameter);
		} else {
			switch_to_inattivo();
		}
	}
}

void switch_to_inattivo() {
	numpad = INATTIVO;
	lcd_clear();
	lcd_put_cur(0, 0);
	HAL_GPIO_WritePin(LED_CONFIGURATION_GPIO_Port, LED_CONFIGURATION_Pin, GPIO_PIN_RESET);
}

void open_door() {
	if (door == SERVO_CHIUSO) {
		door = SERVO_APERTURA;
		start_timer(&htim3);
		set_pwm(&htim3, 0, 750);
		set_autoreload(&htim10, AUTORELOAD_SERVO_MOTOR);
		start_timer(&htim10);
	}
}

void process_door() {
	if (door == SERVO_APERTURA) {
		if (htim3.Instance->CCR2 == 1500) {
			door = SERVO_APERTO;
			change_autoreload_timer_start(&htim10, time_open_door * FROM_SECOND_TO_DECIMILLISECOND);
			prepare_data_door(send_value);
		} else {
			set_pwm(&htim3, 1, 250);
		}
	} else {
		if (door == SERVO_APERTO) {
			door = SERVO_CHIUSURA;
			change_autoreload_timer_start(&htim10, AUTORELOAD_SERVO_MOTOR);
			set_pwm(&htim3, 0, 1250);
		} else {
			if (door == SERVO_CHIUSURA) {
				if (htim3.Instance->CCR2 == 500) {
					stop_timer(&htim3);
					stop_timer(&htim10);
					door = SERVO_CHIUSO;
					prepare_data_door(send_value);
				} else {
					set_pwm(&htim3, 1, -250);
				}
			}
		}
	}
}

void stepper_half_drive (int actual_step) {
	switch (actual_step){
	case 0:
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN1_GPIO_Port, STEPPER_MOTOR_IN1_Pin, GPIO_PIN_SET);   // IN1
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN2_GPIO_Port, STEPPER_MOTOR_IN2_Pin, GPIO_PIN_RESET);   // IN2
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN3_GPIO_Port, STEPPER_MOTOR_IN3_Pin, GPIO_PIN_RESET);   // IN3
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN4_GPIO_Port, STEPPER_MOTOR_IN4_Pin, GPIO_PIN_RESET);   // IN4
		break;

	case 1:
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN1_GPIO_Port, STEPPER_MOTOR_IN1_Pin, GPIO_PIN_SET);   // IN1
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN2_GPIO_Port, STEPPER_MOTOR_IN2_Pin, GPIO_PIN_SET);   // IN2
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN3_GPIO_Port, STEPPER_MOTOR_IN3_Pin, GPIO_PIN_RESET);   // IN3
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN4_GPIO_Port, STEPPER_MOTOR_IN4_Pin, GPIO_PIN_RESET);   // IN4
		break;

	case 2:
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN1_GPIO_Port, STEPPER_MOTOR_IN1_Pin, GPIO_PIN_RESET);   // IN1
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN2_GPIO_Port, STEPPER_MOTOR_IN2_Pin, GPIO_PIN_SET);   // IN2
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN3_GPIO_Port, STEPPER_MOTOR_IN3_Pin, GPIO_PIN_RESET);   // IN3
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN4_GPIO_Port, STEPPER_MOTOR_IN4_Pin, GPIO_PIN_RESET);   // IN4
		break;

	case 3:
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN1_GPIO_Port, STEPPER_MOTOR_IN1_Pin, GPIO_PIN_RESET);   // IN1
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN2_GPIO_Port, STEPPER_MOTOR_IN2_Pin, GPIO_PIN_SET);   // IN2
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN3_GPIO_Port, STEPPER_MOTOR_IN3_Pin, GPIO_PIN_SET);   // IN3
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN4_GPIO_Port, STEPPER_MOTOR_IN4_Pin, GPIO_PIN_RESET);   // IN4
		break;

	case 4:
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN1_GPIO_Port, STEPPER_MOTOR_IN1_Pin, GPIO_PIN_RESET);   // IN1
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN2_GPIO_Port, STEPPER_MOTOR_IN2_Pin, GPIO_PIN_RESET);   // IN2
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN3_GPIO_Port, STEPPER_MOTOR_IN3_Pin, GPIO_PIN_SET);   // IN3
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN4_GPIO_Port, STEPPER_MOTOR_IN4_Pin, GPIO_PIN_RESET);   // IN4
		break;

	case 5:
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN1_GPIO_Port, STEPPER_MOTOR_IN1_Pin, GPIO_PIN_RESET);   // IN1
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN2_GPIO_Port, STEPPER_MOTOR_IN2_Pin, GPIO_PIN_RESET);   // IN2
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN3_GPIO_Port, STEPPER_MOTOR_IN3_Pin, GPIO_PIN_SET);   // IN3
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN4_GPIO_Port, STEPPER_MOTOR_IN4_Pin, GPIO_PIN_SET);   // IN4
		break;

	case 6:
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN1_GPIO_Port, STEPPER_MOTOR_IN1_Pin, GPIO_PIN_RESET);   // IN1
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN2_GPIO_Port, STEPPER_MOTOR_IN2_Pin, GPIO_PIN_RESET);   // IN2
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN3_GPIO_Port, STEPPER_MOTOR_IN3_Pin, GPIO_PIN_RESET);   // IN3
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN4_GPIO_Port, STEPPER_MOTOR_IN4_Pin, GPIO_PIN_SET);   // IN4
		break;

	case 7:
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN1_GPIO_Port, STEPPER_MOTOR_IN1_Pin, GPIO_PIN_SET);   // IN1
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN2_GPIO_Port, STEPPER_MOTOR_IN2_Pin, GPIO_PIN_RESET);   // IN2
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN3_GPIO_Port, STEPPER_MOTOR_IN3_Pin, GPIO_PIN_RESET);   // IN3
		HAL_GPIO_WritePin(STEPPER_MOTOR_IN4_GPIO_Port, STEPPER_MOTOR_IN4_Pin, GPIO_PIN_SET);   // IN4
		break;
	}
	step += direction;
	if (direction == -1) {
		if (step == -1) {
			step = 7;
			seq++;
			if (seq == numberofsequences) {
				seq = 0;
			}
		}
	} else {
		if (direction == 1) {
			if (step == 8) {
				step = 0;
				seq++;
				if (seq == numberofsequences) {
					seq = 0;
				}
			}
		}
	}
}

void HCSR04_Read (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	if (GPIOx == HC_SR04_CLOSE_POSITION_TRIGGER_GPIO_Port &&
			GPIO_Pin == HC_SR04_CLOSE_POSITION_TRIGGER_Pin) {
		HAL_GPIO_WritePin(HC_SR04_CLOSE_POSITION_TRIGGER_GPIO_Port, HC_SR04_CLOSE_POSITION_TRIGGER_Pin, GPIO_PIN_SET);  // pull the TRIG pin HIGH
		change_autoreload_timer_start(&htim1, 10);
	} else {
		if (GPIOx == HC_SR04_OPEN_POSITION_TRIGGER_GPIO_Port &&
				GPIO_Pin == HC_SR04_OPEN_POSITION_TRIGGER_Pin) {
			HAL_GPIO_WritePin(HC_SR04_OPEN_POSITION_TRIGGER_GPIO_Port, HC_SR04_OPEN_POSITION_TRIGGER_Pin, GPIO_PIN_SET);  // pull the TRIG pin HIGH
			change_autoreload_timer_start(&htim1, 10);
		}
	}
}

void open_sliding_gate() {
	if (sliding_gate == CHIUSO) {
		sliding_gate = IN_APERTURA;
		prepare_data_sliding_gate(send_value);
		HAL_GPIO_WritePin(LED_SLIDING_GATE_GPIO_Port, LED_SLIDING_GATE_Pin, GPIO_PIN_SET);
		step = 7;
		seq = 0;
		direction = -1;
		__HAL_TIM_SET_COUNTER(&htim9, 0);
		HAL_TIM_Base_Start_IT(&htim9);
		start_timer(&htim1);
		HCSR04_Read(HC_SR04_OPEN_POSITION_TRIGGER_GPIO_Port, HC_SR04_OPEN_POSITION_TRIGGER_Pin);
	}
}

void close_sliding_gate() {
	sliding_gate = APERTO;
	prepare_data_sliding_gate(send_value);
	stop_timer(&htim9);
	stop_timer(&htim1);
	hc_sr04_open_position = DELAY;
	start_timer(&htim4);
}

void stop_sliding_gate() {
	sliding_gate = CHIUSO;
	prepare_data_sliding_gate(send_value);
	stop_timer(&htim9);
	stop_timer(&htim11);
	hc_sr04_close_position = DELAY;
	HAL_GPIO_WritePin(LED_SLIDING_GATE_GPIO_Port, LED_SLIDING_GATE_Pin, GPIO_PIN_RESET);
}

void start_closing_sliding_gate() {
	sliding_gate = IN_CHIUSURA;
	prepare_data_sliding_gate(send_value);
	stop_timer(&htim4);
	step = 0;
	seq = 0;
	direction = 1;
	start_timer(&htim9);
	start_timer (&htim11);
	HCSR04_Read(HC_SR04_CLOSE_POSITION_TRIGGER_GPIO_Port, HC_SR04_CLOSE_POSITION_TRIGGER_Pin);
}

void change_direction_sliding_gate() {
	if (sliding_gate == IN_CHIUSURA) {
		sliding_gate = IN_APERTURA;
		prepare_data_sliding_gate(send_value);
		direction = -1;
		stop_timer(&htim11);
		hc_sr04_close_position = DELAY;
		start_timer(&htim1);
		HCSR04_Read(HC_SR04_OPEN_POSITION_TRIGGER_GPIO_Port, HC_SR04_OPEN_POSITION_TRIGGER_Pin);
	}
}

void start_sampling_hcsr04(TIM_HandleTypeDef* htim) {
	if (htim == &htim1) {
		if (hc_sr04_open_position == DELAY) {
			HAL_GPIO_WritePin(HC_SR04_OPEN_POSITION_TRIGGER_GPIO_Port, HC_SR04_OPEN_POSITION_TRIGGER_Pin, GPIO_PIN_RESET);
			__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1);
			change_autoreload_timer_start(&htim1, 0xffff-1);
			hc_sr04_open_position = MISURA;
		}
	} else {
		if (htim == &htim11) {
			if (hc_sr04_close_position == DELAY) {
				HAL_GPIO_WritePin(HC_SR04_CLOSE_POSITION_TRIGGER_GPIO_Port, HC_SR04_CLOSE_POSITION_TRIGGER_Pin, GPIO_PIN_RESET);
				__HAL_TIM_ENABLE_IT(&htim11, TIM_IT_CC1);
				change_autoreload_timer_start(&htim11, 0xffff-1);
				hc_sr04_close_position = MISURA;
			}
		}
	}
}

void led_on() {
	HAL_GPIO_WritePin(LED_INTERNAL_LIGHT_GPIO_Port, LED_INTERNAL_LIGHT_Pin, GPIO_PIN_SET);
	if (internal_light == SPENTO) {
		internal_light = ACCESO;
		prepare_data_internal_light(send_value);
		start_timer(&htim5);
	}
}

void led_off() {
	stop_timer(&htim5);
	HAL_GPIO_WritePin(LED_INTERNAL_LIGHT_GPIO_Port, LED_INTERNAL_LIGHT_Pin, GPIO_PIN_RESET);
	internal_light = SPENTO;
	prepare_data_internal_light(send_value);
}

void alarm_on() {
	alarm = ALARM_DISATTIVO;
	prepare_data_alarm(send_value);
	HAL_GPIO_WritePin(LED_ALARM_GPIO_Port, LED_ALARM_Pin, GPIO_PIN_SET);
}

void alarm_off() {
	alarm = SPENTO;
	prepare_data_alarm(send_value);
	HAL_GPIO_WritePin(ACTIVE_BUZZER_GPIO_Port, ACTIVE_BUZZER_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_ALARM_GPIO_Port, LED_ALARM_Pin, GPIO_PIN_RESET);
}

void start_timer(TIM_HandleTypeDef* htim) {
	if (htim == &htim3) {
		__HAL_TIM_SET_COUNTER(htim, 0);
		HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
	} else {
		if (htim == &htim10 || htim == &htim5 || htim == &htim9 || htim == &htim4) {
			__HAL_TIM_SET_COUNTER(htim, 0);
			HAL_TIM_Base_Start_IT(htim);
		} else {
			if (htim == &htim1 || htim == &htim11) {
				HAL_TIM_Base_Start_IT(htim);
				HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1);
			}
		}
	}
}

void stop_timer(TIM_HandleTypeDef* htim) {
	if (htim == &htim3) {
		HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_2);
		__HAL_TIM_SET_COUNTER(htim, 0);
	} else {
		if (htim == &htim10 || htim == &htim5 || htim == &htim9 || htim == &htim4) {
			HAL_TIM_Base_Stop_IT(htim);
			__HAL_TIM_SET_COUNTER(htim, 0);
		} else {
			if (htim == &htim1 || htim == &htim11) {
				HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_1);
				HAL_TIM_Base_Stop_IT(htim);
				__HAL_TIM_SET_COUNTER(htim, 0);
			}
		}
	}

}

void set_autoreload(TIM_HandleTypeDef* htim, uint32_t autoreload) {
	__HAL_TIM_SET_AUTORELOAD(htim, autoreload);
}

void change_autoreload_timer_start(TIM_HandleTypeDef* htim, uint32_t autoreload) {
	set_autoreload(htim, autoreload);
	__HAL_TIM_SET_COUNTER(htim, 0);
}

void set_pwm(TIM_HandleTypeDef* htim, int update, uint32_t pwm) {
	if (htim == &htim3) {
		if (update == 1) {
			htim->Instance->CCR2 += pwm;
		} else {
			if (update == 0) {
				htim->Instance->CCR2 = pwm;
			}
		}
	}
}

void prepare_data_sliding_gate(char operazione) {
	send_byte[0] = sof;
	send_byte[1] = operazione;
	send_byte[2] = cancello;
	if (sliding_gate == CHIUSO) {
		send_byte[3] = '0';
	} else {
		if (sliding_gate == IN_APERTURA) {
			send_byte[3] = '1';
		} else {
			if (sliding_gate == APERTO) {
				send_byte[3] = '2';
			} else {
				if (sliding_gate == IN_CHIUSURA) {
					send_byte[3] = '3';
				}
			}
		}
	}
	send_byte[4] = padding;
	send_byte[5] = padding;
	send_byte[6] = eof;

	send_data();
}

void prepare_data_internal_light(char operazione) {
	send_byte[0] = sof;
	send_byte[1] = operazione;
	send_byte[2] = luce_interna;
	if (internal_light == SPENTO) {
		send_byte[3] = '0';
	} else {
		if (internal_light == ACCESO) {
			send_byte[3] = '1';
		}
	}
	send_byte[4] = padding;
	send_byte[5] = padding;
	send_byte[6] = eof;

	send_data();
}

void prepare_data_door(char operazione) {
	send_byte[0] = sof;
	send_byte[1] = operazione;
	send_byte[2] = porta;
	if (door == SERVO_CHIUSO) {
		send_byte[3] = '0';
	} else {
		if (door == SERVO_APERTO) {
			send_byte[3] = '1';
		}
	}
	send_byte[4] = padding;
	send_byte[5] = padding;
	send_byte[6] = eof;

	send_data();
}

void prepare_data_alarm(char operazione) {
	send_byte[0] = sof;
	send_byte[1] = operazione;
	send_byte[2] = allarme;
	if (alarm == ALARM_SPENTO) {
		send_byte[3] = '0';
	} else {
		if (alarm == ALARM_DISATTIVO) {
			send_byte[3] = '1';
		} else {
			if (alarm == ALARM_ATTIVO) {
				send_byte[3] = '2';
			}
		}
	}
	send_byte[4] = padding;
	send_byte[5] = padding;
	send_byte[6] = eof;

	send_data();
}

void prepare_data_configuration_sliding_gate(char operazione) {
	send_byte[0] = sof;
	send_byte[1] = operazione;
	send_byte[2] = configurazione_cancello;
	send_byte[3] = time_open_sliding_gate + '0';
	send_byte[4] = padding;
	send_byte[5] = padding;
	send_byte[6] = eof;

	send_data();
}

void prepare_data_configuration_internal_light(char operazione) {
	send_byte[0] = sof;
	send_byte[1] = operazione;
	send_byte[2] = configurazione_luce_interna;
	send_byte[3] = (time_on_internal_light / 10) + '0';
	send_byte[4] = (time_on_internal_light - (time_on_internal_light / 10) * 10) + '0';
	send_byte[5] = padding;
	send_byte[6] = eof;

	send_data();
}

void prepare_data_configuration_door(char operazione) {
	send_byte[0] = sof;
	send_byte[1] = operazione;
	send_byte[2] = configurazione_porta;
	send_byte[3] = time_open_door + '0';
	send_byte[4] = padding;
	send_byte[5] = padding;
	send_byte[6] = eof;

	send_data();
}

void send_data() {
	HAL_UART_Transmit(&huart2, send_byte, num_byte, 1000);
}

int process_data(char c) {
	int ok = false;
	switch(uart) {
	case IN_ATTESA_SOF:
		if (c == sof) {
			uart = IN_ATTESA_OPERAZIONE;
			ok = true;
		}
		break;
	case IN_ATTESA_OPERAZIONE:
		if (c == get_info) {
			uart = IN_ATTESA_IDENTIFICATIVO;
			ok = true;
		}
		break;
	case IN_ATTESA_IDENTIFICATIVO:
		if (c == padding) {
			ok = true;
			uart = IN_ATTESA_VALORE_1;
		}
		break;
	case IN_ATTESA_VALORE_1:
		if (c == padding) {
			ok = true;
			uart = IN_ATTESA_VALORE_2;
		}
		break;
	case IN_ATTESA_VALORE_2:
		if (c == padding) {
			ok = true;
			uart = IN_ATTESA_VALORE_3;
		}
		break;
	case IN_ATTESA_VALORE_3:
		if (c == padding) {
			ok = true;
			uart = IN_ATTESA_EOF;
		}
		break;
	case IN_ATTESA_EOF:
		if (c == eof) {
			uart = IN_ATTESA_SOF;
			ok = true;
		}
		break;
	}
	return ok;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM9_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
	init_peripherals();
	HAL_GPIO_WritePin(LED_ALARM_GPIO_Port, LED_ALARM_Pin, GPIO_PIN_SET);
	set_display_state();
	HAL_UART_Receive_IT(&huart2, &data_received, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		read_keypad();
		if (HAL_GPIO_ReadPin(PIR_INTERNAL_LIGHT_GPIO_Port, PIR_INTERNAL_LIGHT_Pin) == GPIO_PIN_SET) {
			led_on();
			if (alarm == ALARM_DISATTIVO) {
				alarm = ALARM_ATTIVO;
				prepare_data_alarm(send_value);
				HAL_GPIO_WritePin(ACTIVE_BUZZER_GPIO_Port, ACTIVE_BUZZER_Pin, GPIO_PIN_SET);
				if (numpad != INATTIVO) {
					index_configurazione_inserita = 0;
					for (int i = 0; i < data_lenght; i++) {
						configurazione_inserita[i] = '\0';
					}
					HAL_GPIO_WritePin(LED_CONFIGURATION_GPIO_Port, LED_CONFIGURATION_Pin, GPIO_PIN_RESET);
				} else {
					numpad = ALLARME;
				}
			}
		}
		if (numpad == INATTIVO) {
			if (HAL_GetTick() - start_timer_display >= PERIOD_DISPLAY_INATTIVO) {
				start_timer_display = HAL_GetTick();
				set_display_state();
			}
		} else {
			if (numpad == END_CONFIGURAZIONE) {
				if (HAL_GetTick() - start_timer_display >= PERIOD_DISPLAY_CONFIGURAZIONE) {
					start_timer_display = HAL_GetTick();
					switch_to_inattivo();
				}
			}
		}
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 84-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 20000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 8400-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 60000-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 84-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 10000000-1;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 84-1;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 1220-1;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 8400-1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 60000-1;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 84-1;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 65535;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim11, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, NUMPAD_R2_Pin|NUMPAD_R3_Pin|HC_SR04_OPEN_POSITION_TRIGGER_Pin|LED_INTERNAL_LIGHT_Pin
                          |LED_ALARM_Pin|LED_SLIDING_GATE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ACTIVE_BUZZER_Pin|STEPPER_MOTOR_IN4_Pin|STEPPER_MOTOR_IN1_Pin|STEPPER_MOTOR_IN2_Pin
                          |STEPPER_MOTOR_IN3_Pin|HC_SR04_CLOSE_POSITION_TRIGGER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NUMPAD_R0_Pin|NUMPAD_R1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_CONFIGURATION_GPIO_Port, LED_CONFIGURATION_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BUTTON_CONFIGURATION_Pin BUTTON_SLIDING_GATE_FRONT_Pin BUTTON_SLIDING_GATE_RETRO_Pin BUTTON_DOOR_FRONT_Pin */
  GPIO_InitStruct.Pin = BUTTON_CONFIGURATION_Pin|BUTTON_SLIDING_GATE_FRONT_Pin|BUTTON_SLIDING_GATE_RETRO_Pin|BUTTON_DOOR_FRONT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PIR_INTERNAL_LIGHT_Pin */
  GPIO_InitStruct.Pin = PIR_INTERNAL_LIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PIR_INTERNAL_LIGHT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_DOOR_RETRO_Pin */
  GPIO_InitStruct.Pin = BUTTON_DOOR_RETRO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BUTTON_DOOR_RETRO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : NUMPAD_C3_Pin */
  GPIO_InitStruct.Pin = NUMPAD_C3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(NUMPAD_C3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NUMPAD_R2_Pin NUMPAD_R3_Pin HC_SR04_OPEN_POSITION_TRIGGER_Pin LED_INTERNAL_LIGHT_Pin
                           LED_ALARM_Pin LED_SLIDING_GATE_Pin */
  GPIO_InitStruct.Pin = NUMPAD_R2_Pin|NUMPAD_R3_Pin|HC_SR04_OPEN_POSITION_TRIGGER_Pin|LED_INTERNAL_LIGHT_Pin
                          |LED_ALARM_Pin|LED_SLIDING_GATE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ACTIVE_BUZZER_Pin STEPPER_MOTOR_IN4_Pin STEPPER_MOTOR_IN1_Pin STEPPER_MOTOR_IN2_Pin
                           STEPPER_MOTOR_IN3_Pin HC_SR04_CLOSE_POSITION_TRIGGER_Pin */
  GPIO_InitStruct.Pin = ACTIVE_BUZZER_Pin|STEPPER_MOTOR_IN4_Pin|STEPPER_MOTOR_IN1_Pin|STEPPER_MOTOR_IN2_Pin
                          |STEPPER_MOTOR_IN3_Pin|HC_SR04_CLOSE_POSITION_TRIGGER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PHOTO_INTERRUPTER_Pin */
  GPIO_InitStruct.Pin = PHOTO_INTERRUPTER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PHOTO_INTERRUPTER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NUMPAD_C0_Pin NUMPAD_C1_Pin */
  GPIO_InitStruct.Pin = NUMPAD_C0_Pin|NUMPAD_C1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : NUMPAD_R0_Pin NUMPAD_R1_Pin */
  GPIO_InitStruct.Pin = NUMPAD_R0_Pin|NUMPAD_R1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_CONFIGURATION_Pin */
  GPIO_InitStruct.Pin = LED_CONFIGURATION_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_CONFIGURATION_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : NUMPAD_C2_Pin */
  GPIO_InitStruct.Pin = NUMPAD_C2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(NUMPAD_C2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void init_peripherals() {
	init_lcd();
	init_keypad();
	init_step_motor();
}

void init_lcd() {
	lcd_init();
	lcd_clear();
	lcd_put_cur(0, 0);
}

void init_keypad() {
	KeyPad_Init();
}

void init_step_motor() {
	stepsperrev = 4096;
	rpm = 12;
	angle = 22.5;
	numofsequence = 512;
	angleperseq = 360.0 / (float)numofsequence;
	numberofsequences = angle / angleperseq;
	sliding_gate = CHIUSO;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == BUTTON_DOOR_FRONT_Pin ||
			GPIO_Pin == BUTTON_DOOR_RETRO_Pin) {
		open_door();
	} else {
		if (GPIO_Pin == BUTTON_SLIDING_GATE_FRONT_Pin ||
				GPIO_Pin == BUTTON_SLIDING_GATE_RETRO_Pin) {
			if (sliding_gate == CHIUSO) {
				open_sliding_gate();
			}
		} else {
			if (GPIO_Pin == BUTTON_CONFIGURATION_Pin) {
				if (numpad == INATTIVO) {
					numpad = CONFIGURAZIONE;
					start_timer_display = HAL_GetTick();
					set_display_configuration();
					HAL_GPIO_WritePin(LED_CONFIGURATION_GPIO_Port, LED_CONFIGURATION_Pin, GPIO_PIN_SET);
				}
			} else {
				if (GPIO_Pin == PHOTO_INTERRUPTER_Pin) {
					change_direction_sliding_gate();
				}
			}

		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim1) {
		if (timer_1 == SETUP) {
			timer_1 = TIMER;
		} else {
			if (timer_1 == TIMER) {
				if (hc_sr04_open_position == DELAY) {
					start_sampling_hcsr04(&htim1);
				}
			}
		}
	} else {
		if (htim == &htim4) {
			if (timer_4 == SETUP) {
				timer_4 = TIMER;
			} else {
				if (timer_4 == TIMER) {
					start_closing_sliding_gate();
				}
			}
		} else {
			if (htim == &htim5) {
				if (timer_5 == SETUP) {
					timer_5 = TIMER;
				} else {
					if (timer_5 == TIMER) {
						led_off();
					}
				}
			} else {
				if (htim == &htim9) {
					if (timer_9 ==  SETUP) {
						timer_9 = TIMER;
					} else {
						if (timer_9 == TIMER) {
							stepper_half_drive(step);
						}
					}
				} else {
					if (htim == &htim10) {
						if (timer_10 == SETUP) {
							timer_10 = TIMER;
						} else {
							if (timer_10 == TIMER) {
								process_door();
							}
						}
					} else {
						if (htim == &htim11) {
							if (timer_11 == SETUP) {
								timer_11 = TIMER;
							} else {
								if (timer_11 == TIMER) {
									if (hc_sr04_close_position == DELAY) {
										start_sampling_hcsr04(&htim11);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim11) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			if (is_first_captured_close_position == 0) {
				ic_val1_close_position = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
				is_first_captured_close_position = 1;
				__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
			} else {
				if (is_first_captured_close_position == 1) {
					uint32_t difference = 0;
					ic_val2_close_position = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
					__HAL_TIM_SET_COUNTER(htim, 0);
					if (ic_val2_close_position > ic_val1_close_position) {
						difference = ic_val2_close_position - ic_val1_close_position;
					} else {
						if (ic_val1_close_position > ic_val2_close_position) {
							difference = (0xffff - ic_val1_close_position) + ic_val2_close_position;
						}
					}
					uint32_t distance = 0;
					distance = difference * .034/2;
					is_first_captured_close_position = 0;
					__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
					__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC1);
					if (distance < distance_close_position) {
						stop_sliding_gate();
					} else {
						hc_sr04_close_position = DELAY;
						HCSR04_Read(HC_SR04_CLOSE_POSITION_TRIGGER_GPIO_Port, HC_SR04_CLOSE_POSITION_TRIGGER_Pin);
					}
				}
			}
		}
	} else {
		if (htim == &htim1) {
			if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
				if (is_first_captured_open_position == 0) {
					ic_val1_open_position = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
					is_first_captured_open_position = 1;
					__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
				} else {
					if (is_first_captured_open_position == 1) {
						uint32_t difference = 0;
						ic_val2_open_position = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
						__HAL_TIM_SET_COUNTER(htim, 0);
						if (ic_val2_open_position > ic_val1_open_position) {
							difference = ic_val2_open_position - ic_val1_open_position;
						} else {
							if (ic_val1_open_position > ic_val2_open_position) {
								difference = (0xffff - ic_val1_open_position) + ic_val2_open_position;
							}
						}
						uint32_t distance = 0;
						distance = difference * .034/2;
						is_first_captured_open_position = 0;
						__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
						__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC1);
						if (distance < distance_open_position) {
							close_sliding_gate();
						} else {
							hc_sr04_open_position = DELAY;
							HCSR04_Read(HC_SR04_OPEN_POSITION_TRIGGER_GPIO_Port, HC_SR04_OPEN_POSITION_TRIGGER_Pin);
						}
					}
				}
			}
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart2) {
		if (process_data(data_received)) {
			receive_byte[index_byte] = data_received;
			index_byte++;
			if (index_byte == num_byte) {
				index_byte = 0;
				prepare_data_sliding_gate(get_info);
				prepare_data_internal_light(get_info);
				prepare_data_door(get_info);
				prepare_data_alarm(get_info);
				prepare_data_configuration_sliding_gate(get_info);
				prepare_data_configuration_internal_light(get_info);
				prepare_data_configuration_door(get_info);
			}
		} else {
			uart = IN_ATTESA_SOF;
			index_byte = 0;
		}
		HAL_UART_Receive_IT(&huart2, &data_received, 1);
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

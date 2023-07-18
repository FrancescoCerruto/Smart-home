#ifndef	_KEYPADCONFIG_H
#define	_KEYPADCONFIG_H
#include "main.h"

#define           _KEYPAD_DEBOUNCE_TIME_MS        20
#define           _KEYPAD_USE_FREERTOS            0


const GPIO_TypeDef* _KEYPAD_COLUMN_GPIO_PORT[] =
{
	NUMPAD_R0_GPIO_Port,
	NUMPAD_R1_GPIO_Port,
	NUMPAD_R2_GPIO_Port,
	NUMPAD_R3_GPIO_Port
};

const uint16_t _KEYPAD_COLUMN_GPIO_PIN[] =
{
	NUMPAD_R0_Pin,
	NUMPAD_R1_Pin,
	NUMPAD_R2_Pin,
	NUMPAD_R3_Pin
};

const GPIO_TypeDef* _KEYPAD_ROW_GPIO_PORT[] =
{
	NUMPAD_C0_GPIO_Port,
	NUMPAD_C1_GPIO_Port,
	NUMPAD_C2_GPIO_Port,
	NUMPAD_C3_GPIO_Port
};

const uint16_t _KEYPAD_ROW_GPIO_PIN[] =
{
	NUMPAD_C0_Pin,
	NUMPAD_C1_Pin,
	NUMPAD_C2_Pin,
	NUMPAD_C3_Pin
};

#endif

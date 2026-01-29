/*
 * led_effect.c
 *
 *  Created on: Jan 27, 2026
 *      Author: Blah
 */

#include "main.h"

extern TimerHandle_t timers[4];

static void LED_Effect_Stop() {
	for (uint8_t i = 0; i < 4; i++) {
		xTimerStop(timers[i], portMAX_DELAY);
	}
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, RESET);
}

static void LED_ToggleAll() {
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
}

static void LED_ToggleEvensOdds() {
	static uint8_t odds = 0;
	if (odds) {
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, RESET);
	} else {
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, RESET);
	}
	odds ^= 1;
}

static void LED_Clockwise() {
	static uint8_t current = 0;
	switch (current) {
	case 0:
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
		current++;
		break;
	case 1:
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		current++;
		break;
	case 2:
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		current++;
		break;
	case 3:
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
		current = 0;
		break;
	}
}

static void LED_CounterClockwise() {
	static uint8_t current = 3;
	switch (current) {
	case 0:
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
		current = 3;
		break;
	case 1:
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		current--;
		break;
	case 2:
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		current--;
		break;
	case 3:
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
		current--;
		break;
	}
}

void LED_Effect_Ctrl1() {
	LED_ToggleAll();
}

void LED_Effect_Ctrl2() {
	LED_ToggleEvensOdds();
}

void LED_Effect_Ctrl3() {
	LED_Clockwise();
}

void LED_Effect_Ctrl4() {
	LED_CounterClockwise();
}

void LED_Effect(uint8_t option) {
	LED_Effect_Stop();
	xTimerStart(timers[option - 1], portMAX_DELAY);
}

/*
 * task_handlers.c
 *
 *  Created on: Jan 27, 2026
 *      Author: Blah
 */

/*
 * task_handler.c
 *
 *  Created on: Jan 27, 2026
 *      Author: Blah
 */

#include "main.h"
#include <string.h>

extern TaskHandle_t led_task, rtc_task, print_task, command_handling_task, menu_task;
extern QueueHandle_t print_queue, input_data_queue;
extern State_t curr_state;
extern UART_HandleTypeDef huart2;
extern TimerHandle_t rtc_timer;

void LED_Effect(uint8_t);

const char* msg_invalid = "Invalid option entered\n";

static int extract_cmd(Command_t* cmd) {
	uint8_t item, index = 0;
	BaseType_t status;
	status = uxQueueMessagesWaiting(input_data_queue);
	if (!status) return -1;
	do {
		status = xQueueReceive(input_data_queue, &item, 0);
		if (status == pdTRUE) {
			cmd->payload[index++] = item;
		}
	} while(item != '\n');
	cmd->payload[index - 1] = '\0';
	cmd->len = index - 1;
	return 0;
}

static void process_cmd(Command_t* cmd) {
	switch (curr_state) {
	case sMainMenu:
		xTaskNotify(menu_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;
	case sLEDEffect:
		xTaskNotify(led_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;
	case sRTCDateConfig:
	case sRTCMenu:
	case sRTCReport:
	case sRTCTimeConfig:
		xTaskNotify(rtc_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;
	}
}

static uint8_t getnumber(uint8_t *p , int len)
{

	int value ;

	if(len > 1)
	   value =  ( ((p[0]-48) * 10) + (p[1] - 48) );
	else
		value = p[0] - 48;

	return value;

}

void LEDEffect_Handler(void* parameters) {
	uint32_t cmd_addr;
	Command_t* cmd;
	const char *msg_led = "====================\n"
						  "|     LED Effect   |\n"
			              "none, e1, e2, e3, e4\n"
						  "Enter your choice here: ";
	for(;;) {
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		xQueueSend(print_queue, &msg_led, portMAX_DELAY);
		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
		cmd = (Command_t*) cmd_addr;
		if (cmd->len <= 4) {
			if (!strcmp((char*)cmd->payload, "none")) {
				LED_Effect(0);
			} else if (!strcmp((char*)cmd->payload, "e1")) {
				LED_Effect(1);
			} else if (!strcmp((char*)cmd->payload, "e2")) {
				LED_Effect(2);
			} else if (!strcmp((char*)cmd->payload, "e3")) {
				LED_Effect(3);
			} else if (!strcmp((char*)cmd->payload, "e4")) {
				LED_Effect(4);
			} else {
				xQueueSend(print_queue, &msg_invalid, portMAX_DELAY);
			}
		} else {
			xQueueSend(print_queue, &msg_invalid, portMAX_DELAY);
		}
		curr_state = sMainMenu;
		xTaskNotify(menu_task, 0, eNoAction);
	}
}

void RTC_Handler(void* parameters) {
	const char* msg_rtc1 = "========================\n"
								"|         RTC          |\n"
								"========================\n";

		const char* msg_rtc2 = "Configure Time            ----> 0\n"
								"Configure Date            ----> 1\n"
								"Enable reporting          ----> 2\n"
								"Exit                      ----> 4\n"
								"Enter your choice here : ";


		const char *msg_rtc_hh = "Enter hour(1-12):";
		const char *msg_rtc_mm = "Enter minutes(0-59):";
		const char *msg_rtc_ss = "Enter seconds(0-59):";

		const char *msg_rtc_dd  = "Enter date(1-31):";
		const char *msg_rtc_mo  ="Enter month(1-12):";
		const char *msg_rtc_dow  = "Enter day(1-7 sun:1):";
		const char *msg_rtc_yr  = "Enter year(0-99):";

		const char *msg_conf = "Configuration successful\n";
		const char *msg_rtc_report = "Enable time&date reporting(y/n)?: ";


		uint32_t cmd_addr;
		Command_t *cmd;
		static int rtc_state = 0;
		RTC_TimeTypeDef time;
		RTC_DateTypeDef date;
		for (;;){
			xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
			xQueueSend(print_queue, &msg_rtc1, portMAX_DELAY);
			show_time_date();
			xQueueSend(print_queue, &msg_rtc2, portMAX_DELAY);
			while(curr_state != sMainMenu){
				xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
				cmd = (Command_t*) cmd_addr;
				switch(curr_state)
				{
					case sRTCMenu:{
						if (cmd->len == 1) {
							uint8_t menu_code;
							menu_code = cmd->payload[0] - 48;
							switch(menu_code)
							{
							case 0:
								curr_state = sRTCTimeConfig;
								xQueueSend(print_queue,&msg_rtc_hh,portMAX_DELAY);
								break;
							case 1:
								curr_state = sRTCDateConfig;
								xQueueSend(print_queue,&msg_rtc_dd,portMAX_DELAY);
								break;
							case 2 :
								curr_state = sRTCReport;
								xQueueSend(print_queue,&msg_rtc_report,portMAX_DELAY);
								break;
							case 3 :
								curr_state = sMainMenu;
								break;
							default:
								curr_state = sMainMenu;
								xQueueSend(print_queue,&msg_invalid,portMAX_DELAY);
							}

						}else{
							curr_state = sMainMenu;
							xQueueSend(print_queue,&msg_invalid,portMAX_DELAY);
						}
					}

					case sRTCTimeConfig:{
						switch(rtc_state)
						{
							case HH_CONFIG:{
								uint8_t hour = getnumber(cmd->payload , cmd->len);
								time.Hours = hour;
								rtc_state = MM_CONFIG;
								xQueueSend(print_queue,&msg_rtc_mm,portMAX_DELAY);
								break;}
							case MM_CONFIG:{
								uint8_t min = getnumber(cmd->payload , cmd->len);
								time.Minutes = min;
								rtc_state = SS_CONFIG;
								xQueueSend(print_queue,&msg_rtc_ss,portMAX_DELAY);
								break;}
							case SS_CONFIG:{
								uint8_t sec = getnumber(cmd->payload , cmd->len);
								time.Seconds = sec;
								if(!validate_rtc_information(&time,NULL))
								{
									rtc_configure_time(&time);
									xQueueSend(print_queue,&msg_conf,portMAX_DELAY);
									show_time_date();
								}else
									xQueueSend(print_queue,&msg_invalid,portMAX_DELAY);

								curr_state = sMainMenu;
								rtc_state = 0;
								break;}
						}
						break;}

					case sRTCDateConfig:{
						switch(rtc_state)
						{
							case DATE_CONFIG:{
								uint8_t d = getnumber(cmd->payload , cmd->len);
								date.Date = d;
								rtc_state = MONTH_CONFIG;
								xQueueSend(print_queue,&msg_rtc_mo,portMAX_DELAY);
								break;}
							case MONTH_CONFIG:{
								uint8_t month = getnumber(cmd->payload , cmd->len);
								date.Month = month;
								rtc_state = DAY_CONFIG;
								xQueueSend(print_queue,&msg_rtc_dow,portMAX_DELAY);
								break;}
							case DAY_CONFIG:{
								uint8_t day = getnumber(cmd->payload , cmd->len);
								date.WeekDay = day;
								rtc_state = YEAR_CONFIG;
								xQueueSend(print_queue,&msg_rtc_yr,portMAX_DELAY);
								break;}
							case YEAR_CONFIG:{
								uint8_t year = getnumber(cmd->payload , cmd->len);
								date.Year = year;

								if(!validate_rtc_information(NULL,&date))
								{
									rtc_configure_date(&date);
									xQueueSend(print_queue,&msg_conf,portMAX_DELAY);
									show_time_date();
								}else
									xQueueSend(print_queue,&msg_invalid,portMAX_DELAY);

								curr_state = sMainMenu;
								rtc_state = 0;
								break;}
						}
						break;}

					case sRTCReport:{
						if(cmd->len == 1)
						{
							if(cmd->payload[0] == 'y'){
								if(xTimerIsTimerActive(rtc_timer) == pdFALSE)
									xTimerStart(rtc_timer,portMAX_DELAY);
							}else if (cmd->payload[0] == 'n'){
								xTimerStop(rtc_timer,portMAX_DELAY);
							}else{
								xQueueSend(print_queue,&msg_invalid,portMAX_DELAY);
							}

						}else
							xQueueSend(print_queue,&msg_invalid,portMAX_DELAY);

						curr_state = sMainMenu;
						break;}
					default:
						break;
				}

			}
			xTaskNotify(menu_task, 0 , eNoAction);

			}
}

void Print_Handler(void* parameters) {
	uint32_t* msg;
	for(;;) {
		xQueueReceive(print_queue, &msg, portMAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen((char*)msg), HAL_MAX_DELAY);
	}
}

void Command_Handler(void* parameters) {
	BaseType_t status;
	Command_t cmd;
	for(;;) {
		status = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		if (status == pdTRUE) {
			extract_cmd(&cmd);
			process_cmd(&cmd);
		}
	}
}

void Menu_Handler(void* parameters) {
	uint32_t cmd_addr;
	uint8_t option;
	Command_t* cmd;
	const char* main_menu = "====================\n"
							"|        Menu      |\n"
							"====================\n"
							"LED Effect -> 0\n"
							"Date and time -> 1\n"
							"Exit -> 2\n"
							"Enter selection: ";
	for(;;) {
		xQueueSend(print_queue, &main_menu, portMAX_DELAY);
		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
		cmd = (Command_t*)cmd_addr;

		if (cmd->len == 1) {
			option = cmd->payload[0] - 48;
			switch (option) {
			case 0:
				curr_state = sLEDEffect;
				xTaskNotify(led_task, 0, eNoAction);
				break;
			case 1:
				curr_state = sRTCMenu;
				xTaskNotify(rtc_task, 0, eNoAction);
				break;
			case 2:
				break;
			default:
				xQueueSend(print_queue, &msg_invalid, portMAX_DELAY);
				continue;
			}
		} else {
			// invalid
			xQueueSend(print_queue, &msg_invalid, portMAX_DELAY);
			continue;
		}
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
	}
}


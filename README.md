# stm32f407xx_FreeRTOS_RTC_LED_Controller


# Implementation

This project was implemented using FreeRTOS along with STM32 HAL libraries. It is programmed in C and uses FreeRTOS tasks, notifications, and queues for intertask communication. It requires an outside program/board driving the
operations via UART communication. It will send the menu to the device via UART and wait for a response and then execute the option that the user has selected.

It allows the user to configure the on-board LEDs with differect effects, and configure the onboard RTC to keep track of the time and date.

# Background

I did this project to gain hands-on experience with FreeRTOS and all the APIs and resources it provides. I gained experience using tasks, queues/notifications for task communication, and software timers.
I also learned a lot about how the scheduler works and the general API design. For example, the FromISR APIs that must be used when calling FreeRTOS APIs from ISRs, and the need to keep ISRs as short as possible
to avoid starving the tasks of CPU time.

# Known Issues
RTC calendar configuration sometimes needs to be redone

# ***TODO***
Allow the user to set whether the current time is AM or PM during configuration

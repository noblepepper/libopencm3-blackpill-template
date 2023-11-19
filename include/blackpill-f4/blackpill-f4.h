/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file provides the platform specific declarations for the blackpill-f4 implementation. */

#ifndef INCLUDE_BLACKPILL_F4_H
#define INCLUDE_BLACKPILL_F4_H

#include "gpio.h"
#include "timing.h"

/* Hardware definitions... */
#define LED_PORT       GPIOC
#define LED_IDLE_RUN   GPIO13
/* USART2 with PA2 and PA3 is selected as USART_CONSOLE Alternatively USART1 with PB6 and PB7 can be used. */
#define USART_CONSOLE               USART2
#define USART_CONSOLE_CR1           USART2_CR1
#define USART_CONSOLE_DR            USART2_DR
#define USART_CONSOLE_CLK           RCC_USART2
#define USART_CONSOLE_PORT          GPIOA
#define USART_CONSOLE_TX_PIN        GPIO2
#define USART_CONSOLE_RX_PIN        GPIO3

#define UART_PIN_SETUP()                                                                            \
	do {                                                                                            \
		gpio_mode_setup(USART_CONSOLE_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART_CONSOLE_TX_PIN);              \
		gpio_set_output_options(USART_CONSOLE_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, USART_CONSOLE_TX_PIN); \
		gpio_set_af(USART_CONSOLE_PORT, GPIO_AF7, USART_CONSOLE_TX_PIN);                                      \
		gpio_mode_setup(USART_CONSOLE_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_CONSOLE_RX_PIN);            \
		gpio_set_output_options(USART_CONSOLE_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, USART_CONSOLE_RX_PIN); \
		gpio_set_af(USART_CONSOLE_PORT, GPIO_AF7, USART_CONSOLE_RX_PIN);                                      \
	} while (0)

#define USB_DRIVER stm32f107_usb_driver
#define USB_IRQ    NVIC_OTG_FS_IRQ
#define USB_ISR(x) otg_fs_isr(x)
/*
 * Interrupt priorities. Low numbers are high priority.
 * TIM3 is used for traceswo capture and must be highest priority.
 */
#define IRQ_PRI_USB          (1U << 4U)

#endif /* INCLUDE_BLACKPILL_F4_H */

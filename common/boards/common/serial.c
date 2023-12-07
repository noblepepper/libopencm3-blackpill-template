/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2022 1BitSquared <info@1bitsquared.com>
 * Written by Rachel Mant <git@dragonmux.network>
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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>

#include "general.h"
#include "board.h"
#include "usb_serial.h"
#include "serial.h"

static uint32_t serial_active_baud_rate;

static volatile uint8_t serial_led_state = 0;


static void serial_set_baudrate(const uint32_t baud_rate)
{
	usart_set_baudrate(USART_CONSOLE, baud_rate);
	serial_active_baud_rate = baud_rate;
}

void serial_init(void)
{
	/* Enable clocks */
	rcc_periph_clock_enable(USART_CONSOLE_CLK);

	/* Setup UART parameters */
	UART_PIN_SETUP();
	serial_set_baudrate(115200);
	usart_set_databits(USART_CONSOLE, 8);
	usart_set_stopbits(USART_CONSOLE, USART_STOPBITS_1);
	usart_set_mode(USART_CONSOLE, USART_MODE_TX_RX);
	usart_set_parity(USART_CONSOLE, USART_PARITY_NONE);
	usart_set_flow_control(USART_CONSOLE, USART_FLOWCONTROL_NONE);
//	USART_CR1(USART_CONSOLE) |= USART_CR1_IDLEIE;

	/* Finally enable the USART */
	usart_enable(USART_CONSOLE);
}

void serial_set_encoding(const usb_cdc_line_coding_s *const coding)
{
	/* Some devices require that the usart is disabled before
	 * changing the usart registers. */
	usart_disable(USART_CONSOLE);
	serial_set_baudrate(coding->dwDTERate);

	if (coding->bParityType != USB_CDC_NO_PARITY)
		usart_set_databits(USART_CONSOLE, coding->bDataBits + 1U <= 8U ? 8 : 9);
	else
		usart_set_databits(USART_CONSOLE, coding->bDataBits <= 8U ? 8 : 9);

	uint32_t stop_bits = USART_STOPBITS_2;
	switch (coding->bCharFormat) {
	case USB_CDC_1_STOP_BITS:
		stop_bits = USART_STOPBITS_1;
		break;
	case USB_CDC_1_5_STOP_BITS:
		stop_bits = USART_STOPBITS_1_5;
		break;
	case USB_CDC_2_STOP_BITS:
	default:
		break;
	}
	usart_set_stopbits(USART_CONSOLE, stop_bits);

	switch (coding->bParityType) {
	case USB_CDC_NO_PARITY:
	default:
		usart_set_parity(USART_CONSOLE, USART_PARITY_NONE);
		break;
	case USB_CDC_ODD_PARITY:
		usart_set_parity(USART_CONSOLE, USART_PARITY_ODD);
		break;
	case USB_CDC_EVEN_PARITY:
		usart_set_parity(USART_CONSOLE, USART_PARITY_EVEN);
		break;
	}
	usart_enable(USART_CONSOLE);
}

void serial_get_encoding(usb_cdc_line_coding_s *const coding)
{
	coding->dwDTERate = serial_active_baud_rate;

	switch (usart_get_stopbits(USART_CONSOLE)) {
	case USART_STOPBITS_1:
		coding->bCharFormat = USB_CDC_1_STOP_BITS;
		break;
	case USART_STOPBITS_2:
	default:
		coding->bCharFormat = USB_CDC_2_STOP_BITS;
		break;
	}

	switch (usart_get_parity(USART_CONSOLE)) {
	case USART_PARITY_NONE:
	default:
		coding->bParityType = USB_CDC_NO_PARITY;
		break;
	case USART_PARITY_ODD:
		coding->bParityType = USB_CDC_ODD_PARITY;
		break;
	case USART_PARITY_EVEN:
		coding->bParityType = USB_CDC_EVEN_PARITY;
		break;
	}

	const uint32_t data_bits = usart_get_databits(USART_CONSOLE);
	if (coding->bParityType == USB_CDC_NO_PARITY)
		coding->bDataBits = data_bits;
	else
		coding->bDataBits = data_bits - 1;
}


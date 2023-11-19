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
#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

#include <stddef.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include "usb_types.h"

#define USART_DMA_BUF_SHIFT 7U

typedef struct usb_cdc_line_coding usb_cdc_line_coding_s;

void serial_init(void);
void serial_set_encoding(const usb_cdc_line_coding_s *coding);
void serial_get_encoding(usb_cdc_line_coding_s *coding);

typedef enum serial_led {
	SERIAL_LED_TX = (1U << 0U),
	SERIAL_LED_RX = (1U << 1U)
} serial_led_e;

void serial_set_led(serial_led_e led);
void serial_clear_led(serial_led_e led);

#endif /* INCLUDE_SERIAL_H */

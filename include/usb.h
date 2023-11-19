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

#ifndef INCLUDE_USB_H
#define INCLUDE_USB_H

#include <stdint.h>
#include <libopencm3/usb/usbd.h>

extern usbd_device *usbdev;
extern uint16_t usb_config;

#if defined(USB_HS)
#define CDCACM_PACKET_SIZE  512U
#define TRACE_ENDPOINT_SIZE 512U
#else
#define CDCACM_PACKET_SIZE  64U
#define TRACE_ENDPOINT_SIZE 64U
#endif

#if !defined(USB_MAX_INTERVAL)
#define USB_MAX_INTERVAL 255U
#endif

#define CDCACM_UART_ENDPOINT 3U

#define UART_IF_NO 2U
#define TOTAL_INTERFACES 2U

void blackmagic_usb_init(void);

/* Returns current usb configuration, or 0 if not configured. */
uint16_t usb_get_config(void);

/* Returns true if usb config has been updated. */
bool usb_config_is_updated(void);

/* Clears usb config updated flag. */
void usb_config_clear_updated(void);

#endif /* INCLUDE_USB_H */

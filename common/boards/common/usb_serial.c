/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011 Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
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

/* This file implements a the USB Communications Device Class - Abstract
 * Control Model (CDC-ACM) as defined in CDC PSTN subclass 1.2.
 * The device's unique id is used as the USB serial number string.
 *
 * Endpoint Usage
 *
 *     0 Control Endpoint
 * IN  3 UART CDC DATA
 * OUT 3 UART CDC DATA
 * OUT 4 UART CDC CTRL
 *
 */

#include <sys/stat.h>
#include <string.h>
typedef struct stat stat_s;
#include "general.h"
#include "board.h"
#include "usb_serial.h"
#include "serial.h"
#include "usb_types.h"
#include "usbwrap.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dma.h>

static void usb_serial_set_state(usbd_device *dev, uint16_t iface, uint8_t ep);

static bool serial_send_complete = true;

static usbd_request_return_codes_e serial_control_request(usbd_device *dev, usb_setup_data_s *req, uint8_t **buf,
	uint16_t *const len, void (**complete)(usbd_device *dev, usb_setup_data_s *req))
{
	(void)complete;
	/* Is the request for the physical/debug UART interface? */
	if (req->wIndex != UART_IF_NO)
		return USBD_REQ_NEXT_CALLBACK;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		/* Send a notification back on the notification endpoint */
		usb_serial_set_state(dev, req->wIndex, CDCACM_UART_ENDPOINT + 1U);
		return USBD_REQ_HANDLED;
	case USB_CDC_REQ_SET_LINE_CODING:
		if (*len < sizeof(usb_cdc_line_coding_s))
			return USBD_REQ_NOTSUPP;
		serial_set_encoding((usb_cdc_line_coding_s *)*buf);
		return USBD_REQ_HANDLED;
	case USB_CDC_REQ_GET_LINE_CODING:
		if (*len < sizeof(usb_cdc_line_coding_s))
			return USBD_REQ_NOTSUPP;
		serial_get_encoding((usb_cdc_line_coding_s *)*buf);
		return USBD_REQ_HANDLED;
	}
	return USBD_REQ_NOTSUPP;
}

void usb_serial_set_state(usbd_device *const dev, const uint16_t iface, const uint8_t ep)
{
	uint8_t buf[10];
	usb_cdc_notification_s *notif = (void *)buf;
	/* We echo signals back to host as notification */
	notif->bmRequestType = 0xa1;
	notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
	notif->wValue = 0;
	notif->wIndex = iface;
	notif->wLength = 2;
	buf[8] = 3U;
	buf[9] = 0U;
	usbd_ep_write_packet(dev, ep, buf, sizeof(buf));
}

void usb_serial_set_config(usbd_device *dev, uint16_t value)
{
	usb_config = value;

	/* Serial interface */
	usbd_ep_setup( \
		dev, CDCACM_UART_ENDPOINT, USB_ENDPOINT_ATTR_BULK, \
		CDCACM_PACKET_SIZE / 2U, read_from_usb);
	usbd_ep_setup( \
		dev, CDCACM_UART_ENDPOINT | USB_REQ_TYPE_IN, \
		USB_ENDPOINT_ATTR_BULK, CDCACM_PACKET_SIZE, \
		send_to_usb);
	usbd_ep_setup( \
		dev, (CDCACM_UART_ENDPOINT + 1U) | USB_REQ_TYPE_IN, \
		USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);


	usbd_register_control_callback(\
		dev, USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE, \
		USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, \
		serial_control_request);
	/* Notify the host that DCD is asserted.
	 * Allows the use of /dev/tty* devices on *BSD/MacOS
	 */
	usb_serial_set_state(dev, UART_IF_NO, CDCACM_UART_ENDPOINT);

}

void serial_send_stdout(const uint8_t *const data, const size_t len)
{
	for (size_t offset = 0; offset < len; offset += CDCACM_PACKET_SIZE) {
		const size_t count = MIN(len - offset, CDCACM_PACKET_SIZE);
		nvic_disable_irq(USB_IRQ);
		/* XXX: Do we actually care if this fails? Possibly not.. */
		usbd_ep_write_packet(usbdev, CDCACM_UART_ENDPOINT, data + offset, count);
		nvic_enable_irq(USB_IRQ);
	}
}

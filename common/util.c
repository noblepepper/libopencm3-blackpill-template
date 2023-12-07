/*
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


#include "general.h"
#include "board.h"
#include "usb.h"
#include "serial.h"
#include "serialno.h"
#include "usb_descriptors.h"
#include "usb_serial.h"
#include "usbwrap.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/usb/dwc/otg_fs.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/spi.h>
#include <stdio.h>
#include <errno.h>

uint32_t DelayCounter;

void sys_tick_handler(void)
{
	DelayCounter++;
}

void delay_ms(uint32_t ms)
{
	DelayCounter = 0;
	while (DelayCounter < ms * 1000) __asm__("nop")
		;
}

void delay_us(uint32_t us)
{
	DelayCounter = 0;
	while (DelayCounter < us) __asm__("nop")
		;
}

int _write(int file, uint8_t *const ptr, const size_t len)
{
	if (file == 1) {
		size_t sent = 0;
		for (size_t offset = 0; offset < len; offset += 64)
		{
			const size_t count = MIN(len - offset, 64);
			nvic_disable_irq(USB_IRQ);
			usbd_ep_write_packet(usbdev, CDCACM_UART_ENDPOINT, ptr + offset, count);
			nvic_enable_irq(USB_IRQ);
			sent += count;
			delay_us(125);/*why is this needed? */
		}
		return sent;
	}

	errno = EIO;
	return -1;
/*	int i;
	if (file == 1) {
		for (i = 0; i < len; i++)
			usart_send_blocking(USART2, ptr[i]);
		return i;
	}

	errno = EIO;
	return -1;*/
}

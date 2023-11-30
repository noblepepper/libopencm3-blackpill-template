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

/* Provides main entry point. Initialise subsystems */

#include "general.h"
#include "board.h"
#include "usbwrap.h"
#include "setup.h"
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>

uint8_t read_register(uint8_t address)
{
	uint8_t data_in;
	spi_enable(SPI3);
	gpio_clear(GPIOA, GPIO15);
	spi_send(SPI3, address);
	data_in = spi_read(SPI3);
	spi_send(SPI3, 0X0);
	data_in = spi_clean_disable(SPI3);
	gpio_set(GPIOA, GPIO15);
	for (int i=0; i<10; i++) __asm__("nop");
	return(data_in);
}

void write_register(uint8_t address, uint8_t data_out)
{
	uint8_t data_in;
	gpio_clear(GPIOA, GPIO15);
	spi_enable(SPI3);
	spi_send(SPI3, address);
	data_in=spi_read(SPI3);
	spi_send(SPI3, data_out);
	data_in=spi_clean_disable(SPI3);
	gpio_set(GPIOA, GPIO15);
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	uint8_t register_address;
	uint8_t register_value;
	clock_setup();
	systick_setup();
	usb_setup();
	usart_setup();
	gpio_setup();
	spi3_setup();
	printf("Hello\r\n");
	for (register_address = 0x0; register_address < 0x8; register_address++)
	{
		register_value = read_register(register_address);
		printf("register %#.2x value %#.2x\r\n", \
			register_address, register_value);
	}
	printf("\r\n\r\n");
	printf("setting to power up \r\n");
/* set 31865 to powerup state */
	/* clear fault */ 
	write_register(0x80, 0x2);
	/* configuration = 00h */ 
	write_register(0x80, 0x0);
	/* high fault MSB = ffh  */ 
	write_register(0x83, 0xff);
	/* high fault LSB = FFh  */ 
	write_register(0x84, 0xFF);
	/* low fault MSB = 00h  */ 
	write_register(0x85, 0x00);
	/* low fault LSB = 00h  */ 
	write_register(0x86, 0x00);
/* 31865 is now at powerup state */
	for (register_address = 0x0; register_address < 0x8; register_address++)
	{
		register_value = read_register(register_address);
		printf("register %#.2x value %#.2x\r\n", \
			register_address, register_value);
	}
	printf("\r\n\r\n");
	write_register(0x80, 0x90);// write configuration bias on 3 wire mode
	for (register_address = 0x0; register_address < 0x8; register_address++)
	{
		register_value = read_register(register_address);
		printf("register %#.2x value %#.2x\r\n", \
			register_address, register_value);
	}


	/* Initialize MAX31865 */
	/* Set PT100 Wire Scheme */
	/* Config Register Bit 4 000X0000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = 3 wire  0 = 2 or 4 wire */

	/* Enable Bias Current */
	/* Config Register Bit 7 X0000000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = on  0 = off */

	/* Set to Autoconvert */
	/* Config Register Bit 6 0X000000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = auto  0 = off */
	
	/* Set Fault Thresholds */
	/* High Fault Register MSB 03h LSB 04h  */
	/* FFFF = max */
	/* Low Fault Register MSB 05h LSB 06h  */
	/* 0000 = min */
	
	/* Clear Faults */
	/* Config Register Bit 1 000000X0 */
	/* 1 = clear */

	/* 50/60hz */
	/* Config Register Bit 0 0000000X */
	/* 1 = 50hz 0= 60hz*/

	while (true) {
		if (usb_data_waiting())
		{
			putchar(usb_recv_blocking());
			fflush(stdout);
		}
		if (usart_data_waiting(USART_CONSOLE))
		{
			usb_send_blocking(usart_recv(USART_CONSOLE));
		}
/*		strncpy(message, "command", 8);
		tosend = message;
		messagelength=8;
		while(messagelength > 0) {
			putchar(*tosend);
			fflush(stdout);
			//spi_send(SPI3, *message);
			tosend++;
			messagelength--;
		}
*/
		asm("nop");
	}

	return 0;
}

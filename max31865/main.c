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
#include "util.h"
#include "max31865.h"
#include <math.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>


int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	clock_setup();
	systick_setup();
	usb_setup();
	usart_setup();
	gpio_setup();
	spi3_setup();
	delay_ms(5000);
	printf("\r\n\r\nHello\r\n");

	set_max31865_to_power_up();
	init_max31865_triggered_60hz();
	print_max31865_registers();
	
	float resistance, temperature, temperature2, temperature3;
	float Rref = 439;
	delay_ms(1000);
	while (true) {
		int i;
		gpio_toggle(GPIOC, GPIO13);
		resistance = 0;
		for (i = 0; i < 512; i++)
		{
			one_shot();
			while (!gpio_get(GPIOB, GPIO6));
			resistance += read_rtd_resistance(439);
		}
		resistance = resistance / 512;
		temperature = get_temperature_method1(resistance);
		temperature2 = get_temperature_method2(resistance);
		temperature3 = get_temperature_method3(resistance);
		printf("temperature F  %3.2f ", temperature);
		printf("temperature2 F  %3.2f ", temperature2);
		printf("temperature3 F  %3.2f \r\n", temperature3);
	}

	return 0;
}

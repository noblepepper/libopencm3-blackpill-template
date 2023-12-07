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

/* This file implements the platform specific functions for the blackpill-f4 implementation. */

#include "general.h"
#include "board.h"
#include "usb.h"
#include "serial.h"
#include "serialno.h"
#include "usb_descriptors.h"
#include "usb_serial.h"

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


#define SYSTICKHZ 1000U

#define SYSTICKMS (1000U / SYSTICKHZ)

static volatile uint32_t time_ms = 0;

int _write(int file, char *ptr, int len);

void clock_setup(void)
{
	/* main clock 96Mhz */
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[BOARD_CLOCK_FREQ]);

	/* Enable GPIO peripherals */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOB);

	/* Enable usb peripherals */
	rcc_periph_clock_enable(RCC_OTGFS);
	rcc_periph_clock_enable(RCC_CRC);
}

void systick_setup(void)
{
	/* Setup heartbeat timer */
	/* 12 Mhz */
//	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	/* Interrupt us at 100 Hz */
//	systick_set_reload(rcc_ahb_frequency / (8U * SYSTICKHZ));
	/* SYSTICK_IRQ with low priority */
//	nvic_set_priority(NVIC_SYSTICK_IRQ, 14U << 4U);
//	systick_interrupt_enable();
	systick_set_frequency(1000000, 96000000);
	systick_counter_enable();
	systick_interrupt_enable();
//	systick_counter_enable();
}

void usart_setup(void)
{
	/* Enable clocks */
	rcc_periph_clock_enable(USART_CONSOLE_CLK);

	/* Setup UART parameters */
	UART_PIN_SETUP();
	usart_set_baudrate(USART_CONSOLE, 115200);
	usart_set_databits(USART_CONSOLE, 8);
	usart_set_stopbits(USART_CONSOLE, USART_STOPBITS_1);
	usart_set_mode(USART_CONSOLE, USART_MODE_TX_RX);
	usart_set_parity(USART_CONSOLE, USART_PARITY_NONE);
	usart_set_flow_control(USART_CONSOLE, USART_FLOWCONTROL_NONE);
//	USART_CR1(USART_CONSOLE) |= USART_CR1_IDLEIE;

	/* Finally enable the USART */
	usart_enable(USART_CONSOLE);
	usart_enable_tx_dma(USART_CONSOLE);
	usart_enable_rx_dma(USART_CONSOLE);
}

void gpio_setup(void)
{
	/* Set up LED pins */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO6);
	gpio_set_output_options(GPIOA, 
			GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO15);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);
}

static bool usb_config_updated = true;

static void usb_config_set_updated(usbd_device *const dev, const uint16_t value)
{
	(void)dev;
	(void)value;
	usb_config_updated = true;
}

void usb_setup(void)
{
/* We need a special large control buffer for this device: */
static uint8_t usbd_control_buffer[512];

	/* Set up DM/DP pins. PA9/PA10 are not routed to USB-C. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

	GPIOA_OSPEEDR &= 0x3c00000cU;
	GPIOA_OSPEEDR |= 0x28000008U;

	usbdev = usbd_init(&USB_DRIVER, &dev_desc, &config, usb_strings,
		 ARRAY_LENGTH(usb_strings), usbd_control_buffer,
		 sizeof(usbd_control_buffer));

	usbd_register_set_config_callback(usbdev, usb_serial_set_config);
	usbd_register_set_config_callback(usbdev, usb_config_set_updated);

	nvic_set_priority(USB_IRQ, IRQ_PRI_USB);
	nvic_enable_irq(USB_IRQ);
	
	/* https://github.com/libopencm3/libopencm3/pull/1256#issuecomment-779424001 */
	OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS | OTG_GCCFG_PWRDWN;
	OTG_FS_GCCFG &= ~(OTG_GCCFG_VBUSBSEN | OTG_GCCFG_VBUSASEN);

}

/* Initialize I2C1 interface */
void i2c_setup(void) {
	/* Enable GPIOB and I2C1 clocks */
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_I2C1);
	/* Set alternate functions for SCL and SDA pins of I2C1 */
	gpio_set_output_options(GPIOB, 
			GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO6|GPIO7);
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6|GPIO7);
	gpio_set_af(GPIOB, GPIO_AF4 , GPIO6|GPIO7);
	/* Disable the I2C peripheral before configuration */
	i2c_peripheral_disable(I2C1);
	/* APB1 running at 48MHz */
	i2c_set_clock_frequency(I2C1, 48);
	/* 400kHz - I2C fast mode */
	i2c_set_speed(I2C1, i2c_speed_fm_400k, 48);
	/* And go */
	i2c_peripheral_enable(I2C1);
}

/* Initialize SPI interfaces */
void spi1_setup(void) {
	/* Set alternate functions for SCK, MISO and MOSI pins of SPI1 */
	/* MOSI=PA7 MISO=PA6 SCK=PA5 NSS=PA4 */
	gpio_set_output_options(GPIOA, 
			GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5|GPIO7);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5|GPIO7);
	gpio_set_af(GPIOA, GPIO_AF5, GPIO5|GPIO7);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
	gpio_set_af(GPIOA, GPIO_AF5, GPIO6);
	/* Enable GPIOB and SPI clocks */
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_SPI1);
	spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_32, 
			SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                        SPI_CR1_CPHA_CLK_TRANSITION_1,
                        SPI_CR1_DFF_8BIT,
                        SPI_CR1_MSBFIRST);
	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);
}

void spi3_setup(void) {
	/* Set alternate functions for SCK, MISO and MOSI pins of SPI3 */
	/* MOSI=PB5 MISO=PB4 SCK=PB3 NSS=PA15 */
	gpio_set_output_options(GPIOB, 
			GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO3|GPIO5);
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3|GPIO5);
	gpio_set_af(GPIOB, GPIO_AF6, GPIO3|GPIO5);
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);
	gpio_set_af(GPIOB, GPIO_AF6, GPIO4);
	/* Enable GPIOB and SPI3 clocks */
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_SPI3);
	spi_init_master(SPI3, SPI_CR1_BAUDRATE_FPCLK_DIV_16, 
			SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                        SPI_CR1_CPHA_CLK_TRANSITION_1,
                        SPI_CR1_DFF_8BIT,
                        SPI_CR1_MSBFIRST);
	spi_enable_software_slave_management(SPI3);
	spi_set_nss_high(SPI3);
}

#include "general.h"
#include "board.h"
#include "usb.h"
#include "serial.h"
#include "serialno.h"
#include "usb_descriptors.h"
#include "usb_serial.h"
#include "usb_serial.h"
#include "util.h"
#include "max31865.h"

#include <math.h>
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

uint8_t register_address;
uint8_t register_value;


uint8_t read_register(uint8_t address)
{
	uint8_t data_in;
	gpio_clear(GPIOA, GPIO15);
	delay_us(400);
	spi_enable(SPI3);
	spi_send(SPI3, address);
	data_in = spi_read(SPI3);
	spi_send(SPI3, 0X0);
	delay_us(100);
	data_in = spi_clean_disable(SPI3);
	gpio_set(GPIOA, GPIO15);
	delay_us(400);
	return(data_in);
}

void write_register(uint8_t address, uint8_t data_out)
{
	uint8_t data_in;
	gpio_clear(GPIOA, GPIO15);
	delay_us(400);
	spi_enable(SPI3);
	spi_send(SPI3, address);
	data_in=spi_read(SPI3);
	spi_send(SPI3, data_out);
	delay_us(100);
	data_in=spi_clean_disable(SPI3);
	gpio_set(GPIOA, GPIO15);
	delay_us(400);
}

void set_bias(bool on)
{
	uint8_t previous_val;
	previous_val = read_register(CONF_REG_READ); 
	if (!on)
		write_register(CONF_REG_WRITE, previous_val & ~BIAS_ON);
	else
		write_register(CONF_REG_WRITE, previous_val | BIAS_ON);
}

void set_3_wire(bool on)
{
	uint8_t previous_val;
	previous_val = read_register(CONF_REG_READ); 
	if (!on)
		write_register(CONF_REG_WRITE, previous_val & ~_3_WIRE);
	else
		write_register(CONF_REG_WRITE, previous_val | _3_WIRE);
}

void set_conv_auto(bool on)
{
	uint8_t previous_val;
	previous_val = read_register(CONF_REG_READ); 
	if (!on)
		write_register(CONF_REG_WRITE, previous_val & ~CONV_AUTO);
	else
		write_register(CONF_REG_WRITE, previous_val | CONV_AUTO);
}

void one_shot(void)
{
	uint8_t previous_val;
	previous_val = read_register(CONF_REG_READ); 
	write_register(CONF_REG_WRITE, previous_val | ONE_SHOT);
}

void clear_fault(void)
{
	uint8_t previous_val;
	previous_val = read_register(CONF_REG_READ); 
	write_register(CONF_REG_WRITE, previous_val | FAULT_CLEAR);
}

void set_60_hz(bool on)
{
	uint8_t previous_val;
	previous_val = read_register(CONF_REG_READ); 
	if (!on)
		write_register(CONF_REG_WRITE, previous_val | _50HZ);
	else
		write_register(CONF_REG_WRITE, previous_val & ~_50HZ);
}

uint8_t read_rtd_msb(void)
{
	uint8_t rtd;
	rtd = read_register(RTD_MSB_REG_READ);
	return rtd;
}

uint8_t read_rtd_lsb(void)
{
	uint8_t rtd;
	rtd = read_register(RTD_LSB_REG_READ);
	return rtd;
}

float read_rtd_resistance(float Rref)
{
	uint8_t lsb, msb;
	uint16_t value;
	lsb = read_rtd_lsb();
	lsb >>= 1;
	msb = read_rtd_msb();
	value = msb;
	value *= 127; 
	value += lsb;
	return  value*Rref/32768;
}

float get_temperature_method1(float resistance)
{
	return (-247.29 + resistance * \
		(2.3992+resistance* \
		(0.000639622+0.0000010241*resistance)))/5*9+32;
}

float get_temperature_method2(float resistance)
{
	return (resistance*32768/400/32 -256)/5*9+32;
	//temperature2 =((value*Rref/400)/32) - 256;
		//temperature2 = temperature2/5*9+32;
}

float get_temperature_method3(float resistance)
{
	float alpha = 0.00390830, beta = -0.0000005775;
	float a, b, c;
	a = beta*100/resistance;
	b = alpha*100/resistance;
	c = 100/resistance - 1;
	return (( -b + sqrt( b*b - 4*a*c) )/a/2)/5*9+32;
}

void set_max31865_to_power_up(void)
{
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
}

void print_max31865_registers(void)
{
	for (register_address = 0x0; register_address < 0x8; register_address++)
	{
		register_value = read_register(register_address);
		printf("register %#.2x value %#.2x\r\n", \
			register_address, register_value);
	}
	printf("\r\n\r\n");
}

void init_max31865_auto_60hz(void)
{
	/* Initialize MAX31865 */
	
	/* Clear Faults */
	/* Config Register Bit 1 000000X0 */
	/* 1 = clear */

	clear_fault();

	/* Set PT100 Wire Scheme */
	/* Config Register Bit 4 000X0000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = 3 wire  0 = 2 or 4 wire */
	set_3_wire(true);
	

	/* Enable Bias Current */
	/* Config Register Bit 7 X0000000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = on  0 = off */
	set_bias(true);
	
	/* 50/60hz */
	/* Config Register Bit 0 0000000X */
	/* 1 = 50hz 0= 60hz*/
	/* MUST BE DONE BEFORE AUTOCONVERT TURNED ON */
	set_60_hz(true);

	/* Set to Autoconvert */
	/* Config Register Bit 6 0X000000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = auto  0 = off */

	set_conv_auto(true);
	
	/* Set Fault Thresholds */
	/* High Fault Register MSB 03h LSB 04h  */
	/* FFFF = max */
	/* Low Fault Register MSB 05h LSB 06h  */
	/* 0000 = min */
	
	delay_ms(100);
}

void init_max31865_triggered_60hz(void)
{
	/* Initialize MAX31865 */
	
	/* Clear Faults */
	/* Config Register Bit 1 000000X0 */
	/* 1 = clear */

	clear_fault();

	/* Set PT100 Wire Scheme */
	/* Config Register Bit 4 000X0000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = 3 wire  0 = 2 or 4 wire */
	set_3_wire(true);
	

	/* Enable Bias Current */
	/* Config Register Bit 7 X0000000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = on  0 = off */
	set_bias(true);
	
	/* 50/60hz */
	/* Config Register Bit 0 0000000X */
	/* 1 = 50hz 0= 60hz*/
	/* MUST BE DONE BEFORE AUTOCONVERT TURNED ON */
	set_60_hz(true);

	/* Set to Autoconvert */
	/* Config Register Bit 6 0X000000 */
	/* Read Address 00H Write Address 80h */ 
	/* 1 = auto  0 = off */

	set_conv_auto(false);
	
	/* Set Fault Thresholds */
	/* High Fault Register MSB 03h LSB 04h  */
	/* FFFF = max */
	/* Low Fault Register MSB 05h LSB 06h  */
	/* 0000 = min */
	
	delay_ms(100);
}

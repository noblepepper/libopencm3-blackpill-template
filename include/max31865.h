
uint8_t read_register(uint8_t address);
void write_register(uint8_t address, uint8_t data_out);
void set_bias(bool on);
void set_3_wire(bool on);
void set_conv_auto(bool on);
void one_shot(void);
void clear_fault(void);
void set_60_hz(bool on);
uint8_t read_rtd_msb(void);
uint8_t read_rtd_lsb(void);
float read_rtd_resistance(float);
float get_temperature_method1(float resistance);
float get_temperature_method2(float resistance);
float get_temperature_method3(float resistance);
void set_max31865_to_power_up(void);
void print_max31865_registers(void);
void init_max31865_auto_60hz(void);
void init_max31865_triggered_60hz(void);

#define CONF_REG_WRITE 0X80
#define HIGH_FAULT_THRESH_MSB_REG_WRITE 0X83
#define HIGH_FAULT_THRESH_LSB_REG_WRITE 0X84
#define LOW_FAULT_THRESH_MSB_REG_WRITE 0X85
#define LOW_FAULT_THRESH_LSB_REG_WRITE 0X86
#define FAULT_STATUS_REG_WRITE 0X87
#define CONF_REG_READ 0X00
#define RTD_MSB_REG_READ 0X01
#define RTD_LSB_REG_READ 0X02
#define HIGH_FAULT_THRESH_MSB_REG_READ 0X03
#define HIGH_FAULT_THRESH_LSB_REG_READ 0X04
#define LOW_FAULT_THRESH_MSB_REG_READ 0X05
#define LOW_FAULT_THRESH_LSB_REG_READ 0X06
#define FAULT_STATUS_REG_READ 0X07

#define BIAS_ON 0X80
#define CONV_AUTO 0X40
#define ONE_SHOT 0X20
#define _3_WIRE 0X10
#define FAULT_CLEAR 0X02
#define _50HZ 0X01


#ifndef HAL_ARDUINO_H_
#define HAL_ARDUINO_H_

#include "../atca_command.h"
#include "atca_hal.h"
#include "../atca_device.h"
#include "../basic/atca_basic.h"

# define I2C_BUSES 1
# define WAKE_WORD 0x00
# define WORD 0x03
# define DELAY_READ_VALUE 100


typedef struct atcaI2Cmaster {
	int _index_bus;
	int ref_ct;
} ATCAI2CMaster_t;

struct Version
{
	uint8_t revision[4];
	ATCADeviceType type;
};

void change_i2c_speed(ATCAIface iface, uint32_t speed);


#endif /* HAL_ARDUINO_H_*/
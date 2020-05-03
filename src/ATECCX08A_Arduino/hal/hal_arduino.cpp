#include <string.h>
#include <stdio.h>
#include <Arduino.h>


#include "../atca_command.h"
#include "atca_hal.h"
#include "../atca_device.h"
#include "../basic/atca_basic.h"
#include "hal_arduino.h"
#include "TWI/hal_arduino_twi.h"

extern "C" {

	/**
	** Var
	**/
	static bool awake = false;


	static const Version revs[] = {
	{{ 0x00, 0x00, 0x60, 0x02 }, ATECC608A},
	{{ 0x00, 0x00, 0x50, 0x00 }, ATECC508A},
	{{ 0x80, 0x00, 0x10, 0x01 }, ATECC108A},
	{{ 0x00, 0x00, 0x10, 0x05 }, ATECC108A},
	{{ 0x00, 0x02, 0x00, 0x08 }, ATSHA204A},
	{{ 0x00, 0x02, 0x00, 0x09 }, ATSHA204A},
	{{ 0x00, 0x04, 0x05, 0x00 }, ATSHA204A},
	};

	/**
	** Tools Functions
	**/

	ATCADeviceType compare_ateccx08_type(uint8_t* buff) {
		ATCADeviceType answer = ATCA_DEV_UNKNOWN;

		size_t size_of_rev = (sizeof(revs) / sizeof((revs)[0]));

		for (size_t i = 0; i < size_of_rev; ++i) {
			if (memcmp(buff, revs[i].revision, sizeof(buff)) == 0) {
				answer = revs[i].type;
				break;
			}
		}
		return answer;
	}

	static void slaveTX() {}
	static void slaveRX(uint8_t*, int) {}

	/**
	** Code Functions
	**/

	ATCA_STATUS hal_i2c_discover_buses(int *buses, int len_buses)
	{
		if (len_buses < 1) {
			return ATCA_SUCCESS;
		}

		buses[0] = 2;

		for (int i = 1; i < len_buses; ++i) {
			buses[i] = -1;
		}
		return ATCA_SUCCESS;
	}

	ATCA_STATUS hal_i2c_discover_devices(int busNum, ATCAIfaceCfg *cfg, int *found)
	{
		cfg->iface_type = ATCA_I2C_IFACE;
		cfg->devtype = ATCA_DEV_UNKNOWN;
		cfg->wake_delay = 1500;
		cfg->rx_retries = 20;
		cfg->atcai2c = {
			.slave_address = 0xC0,
			.bus = (uint8_t)busNum,
			.baud = 100000
		};

		uint8_t buff[ATCA_SERIAL_NUM_SIZE] = { 0 };
		if (atcab_info(buff) != ATCA_SUCCESS) {
			return ATCA_COMM_FAIL;
		}

		ATCADeviceType type = compare_ateccx08_type(buff);
		if (type == ATCA_DEV_UNKNOWN) {
			return ATCA_COMM_FAIL;
		}
		cfg->devtype = type;
		*found = 1;

		return ATCA_SUCCESS;
	}

	ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
	{
		return ATCA_SUCCESS;
	}

	ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
	{
		twi_init();
		twi_attachSlaveTxEvent(slaveTX);
		twi_attachSlaveRxEvent(slaveRX);
		return ATCA_SUCCESS;
	}

	ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
	{
		const ATCAIfaceCfg *cfg = atgetifacecfg(iface);

		// Word for Send DATA
		txdata[0] = WORD;
		// Increment length of 1
		txlength++;

		const uint8_t answer = twi_writeTo(
			cfg->atcai2c.slave_address >> 1,
			txdata,
			txlength,
			1,
			true);

		/*
		* Cf documentation of TWI.CPP Line 174
		*/
		if (answer == 0)
			return ATCA_SUCCESS;
		else if (answer == 1) // Too long message
			return ATCA_BAD_PARAM;
		else
			return ATCA_COMM_FAIL;
	}


	ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
	{
		ATCAIfaceCfg *cfg = atgetifacecfg(iface);
		int retries = cfg->rx_retries;
		const uint8_t slave_address = cfg->atcai2c.slave_address >> 1;

		if (*rxlength == 0) {
			return ATCA_SUCCESS;
		}

		uint8_t msg_len = 0;

		while (twi_readFrom(slave_address, &msg_len, 1, true) < 1 && retries > 0) {
			delay(DELAY_READ_VALUE);
			--retries;
		}
		if (retries <= 0) return ATCA_COMM_FAIL;

		*(rxdata++) = msg_len;

		uint8_t read_size = -1;
		if (msg_len - 1 <= *rxlength)
			read_size = msg_len - 1;
		else
			read_size = *rxlength;

		retries = cfg->rx_retries;
		while (read_size > 0 && retries > 0) {
			const uint8_t length_read = twi_readFrom(slave_address, rxdata, read_size, true);
			if (length_read > read_size) {
				return ATCA_COMM_FAIL;
			}
			if (length_read == 0) {
				delay(DELAY_READ_VALUE);
				--retries;
				continue;
			}
			read_size -= length_read;
			rxdata += length_read;
		}
		if (read_size > 0) {
			return ATCA_COMM_FAIL;
		}

		return ATCA_SUCCESS;
	}

	ATCA_STATUS hal_i2c_wake(ATCAIface iface)
	{
		if (awake)
			return ATCA_SUCCESS;

		ATCAIfaceCfg *cfg = atgetifacecfg(iface);
		int retries = cfg->rx_retries;
		const uint8_t slave_address = cfg->atcai2c.slave_address >> 1;

		twi_writeTo(0, NULL, 0, 1, true);

		atca_delay_us(cfg->wake_delay);

		uint8_t buff[4] = { 0 };
		while (twi_readFrom(slave_address, buff, sizeof(buff), true) < 1 && retries > 0) {
			delay(DELAY_READ_VALUE);
			--retries;
		}
		if (retries <= 0) {
			return ATCA_COMM_FAIL;
		}
		if (hal_check_wake(buff, sizeof(buff)) != ATCA_SUCCESS) {
			return ATCA_COMM_FAIL;
		}

		awake = true;

		return ATCA_SUCCESS;
	}

	ATCA_STATUS hal_i2c_idle(ATCAIface iface)
	{
		uint8_t idle = 0x02;

		const ATCAIfaceCfg *cfg = atgetifacecfg(iface);

		const uint8_t answer = twi_writeTo(
			cfg->atcai2c.slave_address >> 1,
			&idle,
			sizeof(idle),
			1,
			true);

		if (answer == 0) {
			awake = false;
			return ATCA_SUCCESS;
		}
		return ATCA_COMM_FAIL;
	}

	ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
	{
		uint8_t sleep = 0x01;

		const ATCAIfaceCfg *cfg = atgetifacecfg(iface);

		const uint8_t answer = twi_writeTo(
			cfg->atcai2c.slave_address >> 1,
			&sleep,
			sizeof(sleep),
			1,
			true);

		return answer == 0 ? ATCA_SUCCESS : ATCA_COMM_FAIL;
	}

	ATCA_STATUS hal_i2c_release(void *hal_data)
	{
		return ATCA_SUCCESS;
	}

	void atca_delay_ms(uint32_t ms)
	{
		delay(ms);
	}

	void atca_delay_us(uint32_t us)
	{
		delayMicroseconds(us);
	}
} /* End Extern "C" */
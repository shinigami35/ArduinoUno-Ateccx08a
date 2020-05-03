#ifndef ATCA_CONFIG_H
#define ATCA_CONFIG_H

#define ATCA_HAL_I2C


/* Include HALS */
#include "hal\atca_hal.h"

/* Included device support */
#define ATCA_ATSHA204A_SUPPORT
#define ATCA_ATSHA206A_SUPPORT
#define ATCA_ATECC108A_SUPPORT
#define ATCA_ATECC508A_SUPPORT
#define ATCA_ATECC608A_SUPPORT

// Print function
#define ATCAPRINTF

#endif // ATCA_CONFIG_H

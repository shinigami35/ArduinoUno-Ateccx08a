#include <Arduino.h>
#include "ATECCX08A_Arduino/cryptoauthlib.h"
#include "Configuration.h"

/** \brief Lock the CONFIG_ZONE or the DATA_ZONE
 *  \param[in] cfg  Logical interface configuration. Some predefined
 *                  configurations can be found in atca_cfgs.h
 *  \param[in] zone LOCK_ZONE_DATA or LOCK_ZONE_CONFIG
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS lock_zone(ATCAIfaceCfg *cfg, uint8_t zone)
{
    if (zone != (uint8_t)LOCK_ZONE_CONFIG && zone != (uint8_t)LOCK_ZONE_DATA)
        return ATCA_BAD_PARAM;

    ATCA_STATUS status = atcab_init(cfg);

    if (status == ATCA_SUCCESS)
    {
        if (zone == (uint8_t)LOCK_ZONE_DATA)
        {
            return atcab_lock_data_zone();
        }
        else if (zone == (uint8_t)LOCK_ZONE_CONFIG)
        {
            return atcab_lock_config_zone();
        }
        else
        {
            return ATCA_BAD_PARAM;
        }
    }
    return ATCA_BAD_PARAM;
}

/** \brief Check if a the DATA_ZONE or CONFIG_ZONE is locked
 *  \param[in] cfg  Logical interface configuration. Some predefined
 *                  configurations can be found in atca_cfgs.h
 *  \param[in] zone LOCK_ZONE_DATA or LOCK_ZONE_CONFIG
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS check_lock_zone(ATCAIfaceCfg *cfg, uint8_t zone)
{
    ATCA_STATUS status;
    bool lock = false;

    if (zone != (uint8_t)LOCK_ZONE_CONFIG && zone != (uint8_t)LOCK_ZONE_DATA)
        return ATCA_BAD_PARAM;

    status = atcab_init(cfg);
    if (status == ATCA_SUCCESS)
    {
        if (ATCA_SUCCESS != (status = atcab_is_locked(zone, &lock)))
        {
            return ATCA_FUNC_FAIL;
        }
        if (!lock)
        {
            return ATCA_NOT_LOCKED;
        }
        return ATCA_SUCCESS;
    }
    return ATCA_BAD_PARAM;
}

/** \brief Write a new configuration to the chip.
 *  \param[in] cfg  Logical interface configuration. Some predefined
 *                  configurations can be found in atca_cfgs.h
 *  \param[in] config Array uint8_t of configuration (length 112)
 *  \param[in] len Size of the configuration array
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS write_configuration(ATCAIfaceCfg *cfg, uint8_t *config, size_t len)
{
    if (len != 112)
        return ATCA_BAD_PARAM;

    ATCA_STATUS status;

    status = atcab_init(cfg);
    if (status == ATCA_SUCCESS)
    {

        // Write the configuration Array to the chip
        // Padding of 16 byte (16 first bytes cannot be writed)
        status = atcab_write_bytes_zone(ATCA_ZONE_CONFIG, 0, 16, (uint8_t *)config, len);
        return status;
    }
    return status;
}

/** \brief Write AES key in a given slot.
 *  \param[in] cfg      Logical interface configuration. Some predefined
 *                      configurations can be found in atca_cfgs.h
 *  \param[in] key      key slot number
 *  \param[in] datakey  key array uint8_t
 *  \param[in] len      Size of the key array
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS write_key_slot(ATCAIfaceCfg *cfg, uint8_t key, uint8_t *datakey, size_t len)
{
    if (key < 1 && key > 16)
        return ATCA_BAD_PARAM;

    if (len != 32)
        return ATCA_BAD_PARAM;

    ATCA_STATUS status = atcab_init(cfg);

    if (status == ATCA_SUCCESS)
    {
        status = atcab_write_zone(ATCA_ZONE_DATA, (uint16_t)key, 0, 0, datakey, 32);
        if (status != ATCA_SUCCESS)
            return status;
    }
    return status;
}
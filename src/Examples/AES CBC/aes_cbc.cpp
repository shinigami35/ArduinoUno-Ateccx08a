#include <HardwareSerial.h>
#include "ATECCX08A_Arduino/cryptoauthlib.h"
#include "aes_cbc.h"

/** \brief Encrypt data using AES CBC algorithme
 *  \param[in] cfg          Logical interface configuration. Some predefined
 *                          configurations can be found in atca_cfgs.h
 *  \param[in] data         Words to encypt (must be divided by 16, max length 240)
 *  \param[in] len          length of Words to encypt (must be divided by 16, max length 240)
 *  \param[out] iv          Initial Vector used in the AES CBC (return the vector in this var)
 *  \param[out] ciphertext  return here the Cypher text
 *  \param[in] key          Slot number of the key
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS aes_cbc_encrypt(ATCAIfaceCfg *cfg, uint8_t *data, int len, uint8_t *iv, uint8_t *ciphertext, uint8_t key)
{

    atca_aes_cbc_ctx_t ctx;

    if (len > LIMIT_DATA_SIZE_CBC && len % 16 != 0)
    {
        Serial.print(F("ERROR : ATCA_BAD_PARAM"));
        return ATCA_BAD_PARAM;
    }
    uint8_t tmp_iv[IV_LENGTH_CBC];
    uint8_t tmp_data[len];

    ATCA_STATUS status = atcab_init(cfg);
    if (status == ATCA_SUCCESS)
    {
        status = atcab_aes_cbc_init(&ctx, key, 0, tmp_iv);

        if (status != ATCA_SUCCESS)
        {
            Serial.print(F("ERROR Encrypt : atcab_aes_cbc_init, Code Error 0x"));
            Serial.println(status, HEX);
            return;
        }
        memcpy(iv, tmp_iv, IV_LENGTH_CBC);
        memcpy(tmp_data, data, len);

        int max = len / 16;

        for (int j = 0; j < max; j++)
        {
            status = atcab_aes_cbc_encrypt_block(&ctx, &tmp_data[j * 16], &ciphertext[j * 16]);
        }
        if (status != ATCA_SUCCESS)
        {
            Serial.print(F("ERROR Encrypt : atcab_aes_cbc_encrypt_block, Code Error 0x"));
            Serial.println(status, HEX);
        }
        return status;
    }
    return status;
}

/** \brief Decrypt data using AES CBC algorithme
 *  \param[in] cfg          Logical interface configuration. Some predefined
 *                          configurations can be found in atca_cfgs.h
 *  \param[in] ciphertext   Words to decypt (must be divided by 16, max length 240)
 *  \param[in] len          length of Words to decypt (must be divided by 16, max length 240)
 *  \param[in] iv           Initial Vector to use in the AES CBC 
 *  \param[out] plaintext   return here the decrypted text
 *  \param[in] key          Slot number of the key
 *  \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS aes_cbc_decrypt(ATCAIfaceCfg *cfg, uint8_t *ciphertext, int len, uint8_t *iv, uint8_t *plaintext, uint8_t key)
{

    atca_aes_cbc_ctx_t ctx;

    if (len > LIMIT_DATA_SIZE_CBC || len % 16 != 0)
    {
        Serial.print(F("ERROR Decrypt : ATCA_BAD_PARAM"));
        return ATCA_BAD_PARAM;
    }

    ATCA_STATUS status = atcab_init(cfg);
    if (status == ATCA_SUCCESS)
    {
        status = atcab_aes_cbc_init(&ctx, key, 0, iv);

        if (status != ATCA_SUCCESS)
        {
            Serial.print(F("ERROR Decrypt: atcab_aes_cbc_init, Code Error 0x"));
            Serial.println(status, HEX);
            return;
        }
        int max = len / 16;

        for (int j = 0; j < max; j++)
        {
            status = atcab_aes_cbc_decrypt_block(&ctx, &ciphertext[j * 16], &plaintext[j * 16]);
        }
        if (status != ATCA_SUCCESS)
        {
            Serial.print(F("ERROR Decrypt : atcab_aes_cbc_encrypt_block, Code Error 0x"));
            Serial.println(status, HEX);
        }

        return status;
    }
    return status;
}

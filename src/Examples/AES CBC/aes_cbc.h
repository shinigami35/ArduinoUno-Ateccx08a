#ifndef AES_CBC_H
#define AES_CBC_H

 /*  DO NOT MODIFIED */
#define IV_LENGTH_CBC 16
#define LIMIT_DATA_SIZE_CBC 240

ATCA_STATUS aes_cbc_decrypt(ATCAIfaceCfg *cfg, uint8_t *ciphertext, int len, uint8_t *iv, uint8_t *plaintext, uint8_t key);
ATCA_STATUS aes_cbc_encrypt(ATCAIfaceCfg *cfg, uint8_t *data, int len, uint8_t *iv, uint8_t *ciphertext, uint8_t key);

#endif /* aes_cbc.h */
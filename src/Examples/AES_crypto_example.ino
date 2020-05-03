#include <Arduino.h>
#include "ATECCX08A_Arduino/cryptoauthlib.h"
#include "Examples/AES CBC/aes_cbc.h"

// Key Slot number
uint8_t KEY_SLOT = (uint8_t)9;

ATCAIfaceCfg cfg;
ATCA_STATUS status;

void setup()
{
    Serial.begin(9600);

    // Init the constuctor for the library
    cfg.iface_type = ATCA_I2C_IFACE;  // Type of communication -> I2C mode
    cfg.devtype = ATECC608A;          // Type of chip
    cfg.atcai2c.slave_address = 0XC0; // I2C addresse (default value)
    cfg.atcai2c.bus = 1;
    cfg.atcai2c.baud = 100000;
    cfg.wake_delay = 1500; // Delay of wake up (1500 ms)
    cfg.rx_retries = 20;
}

void loop()
{
    ATCA_STATUS status = atcab_init(&cfg);
    if (status != ATCA_SUCCESS)
    {
        Serial.println(F("atcab_init() failed : Code -> 0x"));
        Serial.println(status, HEX);
    }

    uint8_t plaintext[16] = "AAAAAAAAAAAAAAA";

    uint8_t iv[IV_LENGTH_CBC];
    uint8_t cypherdata[sizeof(plaintext)];
    uint8_t decryptdata[sizeof(plaintext)];

    Serial.println("Beginning of the encryption !");
    status = aes_cbc_encrypt(&cfg, plaintext, sizeof(plaintext), iv, cypherdata, KEY_SLOT);
    if (status == ATCA_SUCCESS)
    {
        status = aes_cbc_decrypt(&cfg, cypherdata, sizeof(cypherdata), iv, decryptdata, KEY_SLOT);
        if (status == ATCA_SUCCESS)
        {
            Serial.print("Decrypted text is : ");
            for (size_t i = 0; i < sizeof(decryptdata); i++)
            {
                Serial.print((char)decryptdata[i]);
            }
            Serial.println("");
        }
        else
        {
            // See file atca_status.h for the code Error
            Serial.print(F("Impossible do the decryption | Code Error 0x"));
            Serial.println(status, HEX);
            return;
        }
    }
    else
    {
        // See file atca_status.h for the code Error
        Serial.print(F("Impossible do the encryption | Code Error 0x"));
        Serial.println(status, HEX);
        return;
    }
}
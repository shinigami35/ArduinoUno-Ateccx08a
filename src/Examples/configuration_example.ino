#include <Arduino.h>
#include "ATECCX08A_Arduino/cryptoauthlib.h"
#include "Examples/Configuration/Configuration.h"

uint8_t KEY_SLOT = (uint8_t)9;

ATCAIfaceCfg cfg;
ATCA_STATUS status;

// Example of AES KEY (len 32)
uint8_t example_of_key[32] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

// Exemple of configuration for ATECC608A
static uint8_t configuration_example[112] = {
    0xC0, // I2C address<br>        0x00,
    0x00,
    0x00,
    0x83, 0x20, //  Slot Config Slot 1
    0x85, 0x20, //  Slot Config Slot 2
    0x8F, 0x20, //  Slot Config Slot 3
    0xC4, 0x8F, //  Slot Config Slot 4
    0x8F, 0x8F, //  Slot Config Slot 5
    0x8F, 0x8F, //  Slot Config Slot 6
    0x9F, 0x8F, //  Slot Config Slot 7
    0x0F, 0x0F, //  Slot Config Slot 8
    0x8F, 0x0F, //  Slot Config Slot 9
    0x8F, 0x0F, //  Slot Config Slot 10
    0x8F, 0x0F, //  Slot Config Slot 11
    0x8F, 0x0F, //  Slot Config Slot 12
    0x8F, 0x0F, //  Slot Config Slot 13
    0x00, 0x00, //  Slot Config Slot 14
    0x00, 0x00, //  Slot Config Slot 15
    0xAF, 0x8F, //  Slot Config Slot 16
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00,
    0x00,
    0x00, 0x00,
    0x00,
    0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xFF, 0xFF,
    0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x33, 0x00, //  Key Config Slot 1
    0x33, 0x00, //  Key Config Slot 2
    0x33, 0x00, //  Key Config Slot 3
    0x1C, 0x00, //  Key Config Slot 4
    0x1C, 0x00, //  Key Config Slot 5
    0x1C, 0x00, //  Key Config Slot 6
    0x1C, 0x00, //  Key Config Slot 7
    0x3C, 0x00, //  Key Config Slot 8
    0x1A, 0x00, //  Key Config Slot 9
    0x3A, 0x00, //  Key Config Slot 10
    0x1A, 0x00, //  Key Config Slot 11
    0x3A, 0x00, //  Key Config Slot 12
    0x3A, 0x00, //  Key Config Slot 13
    0x3C, 0x00, //  Key Config Slot 14
    0x3C, 0x00, //  Key Config Slot 15
    0x1C, 0x00  //  Key Config Slot 16
};

bool menu(String message)
{
  Serial.println(message);
  while (!Serial.available())
  {
    ;
  }
  String answer = Serial.readString();
  return (answer == "yes") ? true : menu(message);
}

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

  status = atcab_init(&cfg);
  if (status == ATCA_SUCCESS)
  {
    // Write the configuration
    if (!menu(F("Do you want to write the configuration ?")))
      return;
    status = write_configuration(&cfg, configuration_example, sizeof(configuration_example));
    if (status == ATCA_SUCCESS)
    {
      // Lock the configuration Zone
      if (!menu(F("Do you want to lock the Configuration zone (No more change can be done after that) ?")))
        return;
      status = lock_zone(&cfg, LOCK_ZONE_CONFIG);
      if (status == ATCA_SUCCESS)
      {
        // Write the key in the given slot
        if (!menu(F("Do you want to write the key in the given slot ?")))
          return;
        status = write_key_slot(&cfg, KEY_SLOT, example_of_key, sizeof(example_of_key));
        if (status == ATCA_SUCCESS)
        {
          // Lock the Data Zone
          if (!menu(F("Do you want to lock the Data zone (No more change can be done after that) ?")))
            return;
          status = lock_zone(&cfg, LOCK_ZONE_DATA);
          if (status == ATCA_SUCCESS)
          {
            ATCA_STATUS lock_config = check_lock_zone(&cfg, LOCK_ZONE_CONFIG);
            ATCA_STATUS lock_data = check_lock_zone(&cfg, LOCK_ZONE_DATA);
            if (lock_config == ATCA_SUCCESS && lock_data == ATCA_SUCCESS)
            {
              Serial.println("Your chip has been set, you can use it now !");
              return;
            }
            else
            {
              if (lock_config != ATCA_SUCCESS)
                Serial.println("Your config zone has not been locked !");
              if (lock_data != ATCA_SUCCESS)
                Serial.println("Your data zone has not been locked !");
              return;
            }
          }
          else
          {
            // See file atca_status.h for the code Error
            Serial.print(F("Impossible to Lock data | Code Error 0x"));
            Serial.println(status, HEX);
            return;
          }
        }
        else
        {
          // See file atca_status.h for the code Error
          Serial.print(F("Impossible to Write the key | Code Error 0x"));
          Serial.println(status, HEX);
          return;
        }
      }
      else
      {
        // See file atca_status.h for the code Error
        Serial.print(F("Impossible to Lock configuration | Code Error 0x"));
        Serial.println(status, HEX);
        return;
      }
    }
    else
    {
      // See file atca_status.h for the code Error
      Serial.print(F("Impossible to Write configuration | Code Error 0x"));
      Serial.println(status, HEX);
      return;
    }
  }
  else
  {
    // See file atca_status.h for the code Error
    Serial.print(F("Impossible to Init configuration | Code Error 0x"));
    Serial.println(status, HEX);
    return;
  }
}

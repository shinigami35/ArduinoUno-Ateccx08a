#ifndef CONFIGURATION_H
#define CONFIGURATION_H


ATCA_STATUS lock_zone(ATCAIfaceCfg *cfg, uint8_t zone);
ATCA_STATUS check_lock_zone(ATCAIfaceCfg *cfg, uint8_t zone);
ATCA_STATUS write_configuration(ATCAIfaceCfg *cfg, uint8_t *config, size_t len);
ATCA_STATUS write_key_slot(ATCAIfaceCfg *cfg, uint8_t key, uint8_t *datakey, size_t len);

#endif /* Configuration.h */
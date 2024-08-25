#ifndef PN532_H_
#define PN532_H_

#include <zephyr/device.h>

#define PN532_MIFARE_ISO14443A (0x00)

int pn532_get_firmware_version(const struct device *i2c_dev, uint32_t *version);
int pn532_start_passive_target_id_detection(const struct device *i2c_dev, uint8_t card_baudrate);
int pn532_read_passive_target_id(const struct device *i2c_dev, uint8_t *uid, uint8_t *uid_length, uint16_t timeout);

#endif /* PN532_H_ */

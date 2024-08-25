#ifndef PN532_H_
#define PN532_H_

#include <zephyr/device.h>

int pn532_get_firmware_version(const struct device *i2c_dev, uint32_t *version);
int pn532_read_passive_target_uid(const struct device *i2c_dev, uint8_t *uid, uint8_t *uid_length, uint16_t timeout);

#endif /* PN532_H_ */

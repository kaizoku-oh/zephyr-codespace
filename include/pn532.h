#include <zephyr/device.h>

int pn532_get_firmware_version(const struct device *i2c_dev, uint32_t *version);

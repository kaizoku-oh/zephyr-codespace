#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include "pn532.h"

LOG_MODULE_REGISTER(pn532_example);

static void nfc_thread_handler(void);

K_THREAD_DEFINE(nfc_thread, 1024, nfc_thread_handler, NULL, NULL, NULL, 7, 0, 0);

static void nfc_thread_handler(void)
{
  int ret = 0;
  uint32_t version = 0;
  const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

  if (!device_is_ready(i2c_dev)) {
    LOG_ERR("I2C device is not ready");
    while (true);
  }

  while (true)
  {
    ret = pn532_get_firmware_version(i2c_dev, &version);
    if (ret == 0)
    {
      LOG_INF("Found chip PN5%x", (version >> 24) & 0xFF);
      LOG_INF("Firmware version %d.%d", (version >> 16) & 0xFF, (version >> 8) & 0xFF);
    }
    else
    {
      LOG_ERR("Error reading firmware version: %d", ret);
    }
    k_msleep(1000);
  }
}
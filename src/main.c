#include <stdlib.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#define PN532_I2C_ADDRESS (0x48 >> 1)
#define PN532_CMD_GET_FIRMWARE_VERSION (0x02)

static void temperature_thread_handler(void);
static void nfc_thread_handler(void);

K_THREAD_DEFINE(temperature_thread, 1024, temperature_thread_handler, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(nfc_thread, 1024, nfc_thread_handler, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
  while (true)
  {
    LOG_INF("Running...");
    k_msleep(1000);
  }

  return EXIT_FAILURE;
}

static void temperature_thread_handler(void)
{
  int ret = 0;
  struct sensor_value value = {0};
  const struct device *die_temp_dev = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  /* Check if internal temperature sensor is ready to be used */
  if (!device_is_ready(die_temp_dev)) {
    LOG_ERR("Temperature device is not ready");
    while (true);
  }

  while (true)
  {
    /* Fetch a sample from the sensor */
    ret = sensor_sample_fetch(die_temp_dev);
    if (ret) {
      LOG_ERR("Failed to fetch sample (%d)", ret);
      while (true);
    }

    /* Get a reading from a sensor device */
    ret = sensor_channel_get(die_temp_dev, SENSOR_CHAN_DIE_TEMP, &value);
    if (ret) {
      LOG_ERR("Failed to get data (%d)", ret);
      while (true);
    }

    /* Print temperature value */
    LOG_INF("CPU temperature: %.2f °C", sensor_value_to_double(&value));

    k_msleep(1000);
  }
}

static void nfc_thread_handler(void)
{
  int ret = 0;
  uint8_t command = PN532_CMD_GET_FIRMWARE_VERSION;
  uint8_t response[6] = {0};
  const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

  /* Check if i2c device is ready to be used */
  if (!device_is_ready(i2c_dev)) {
    LOG_ERR("I2C device is not ready");
    while (true);
  }

  while (true)
  {
    /* Write then read data from an I2C device */
    ret = i2c_write_read(i2c_dev, PN532_I2C_ADDRESS, &command, 1, response, sizeof(response));
    if (ret == 0) {
      /* Process the response, which contains the firmware version */
      LOG_INF("Firmware version: %x.%x\n", response[1], response[2]);
    } else {
      LOG_ERR("Error reading firmware version: %d", ret);
    }

    k_msleep(1000);
  }
}

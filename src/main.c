#include <stdlib.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#define PN532_I2C_ADDRESS (0x48 >> 1)

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
  const struct device *temperature_dev = DEVICE_DT_GET(DT_NODELABEL(die_temp));

  /* Check if internal temperature sensor is ready to be used */
  if (!device_is_ready(temperature_dev)) {
    LOG_ERR("Temperature device is not ready");
    while (true);
  }

  while (true)
  {
    /* Fetch a sample from the sensor */
    ret = sensor_sample_fetch(temperature_dev);
    if (ret) {
      LOG_ERR("Failed to fetch sample (%d)", ret);
      while (true);
    }

    /* Get a reading from a sensor device */
    ret = sensor_channel_get(temperature_dev, SENSOR_CHAN_DIE_TEMP, &value);
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
  uint8_t version_cmd[] = {0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};
  uint8_t response[10] = {0};
  const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

  /* Check if i2c device is ready to be used */
  if (!device_is_ready(i2c_dev)) {
    LOG_ERR("I2C device is not ready");
    while (true);
  }

  while (true)
  {
    /* Send Get Firmware Version command */
    ret = i2c_write(i2c_dev, version_cmd, sizeof(version_cmd), PN532_I2C_ADDRESS);
    if (ret < 0) {
      LOG_ERR("Failed to send command to PN532");
      while (true);
    }

    /* Read the response from the PN532 */
    ret = i2c_read(i2c_dev, response, sizeof(response), PN532_I2C_ADDRESS);
    if (ret < 0) {
      LOG_ERR("Failed to read response from PN532");
      while (true);
    }

    /* Process the response (assuming success) */
    if (response[6] == 0x32) {
      LOG_INF("PN532 detected, Firmware Version: %02x.%02x", response[7], response[8]);
    } else {
      LOG_WRN("No PN532 detected or wrong response");
    }

    k_msleep(1000);
  }
}

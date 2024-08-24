#include <stdlib.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(die_temp_example);

static void temperature_thread_handler(void);

K_THREAD_DEFINE(temperature_thread, 1024, temperature_thread_handler, NULL, NULL, NULL, 7, 0, 0);

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

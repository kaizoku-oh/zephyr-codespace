#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "pn532.h"

LOG_MODULE_REGISTER(pn532_example);

static void nfc_thread_handler(void);
static int pn532_irq_setup(void);
static void pn532_irq_handler_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

static struct gpio_callback pn532_irq_cb_data;

K_THREAD_DEFINE(nfc_thread, 1536, nfc_thread_handler, NULL, NULL, NULL, 7, 0, 0);

static void nfc_thread_handler(void)
{
  int ret = 0;
  uint32_t version = 0;
  uint8_t uid[7] = {0};
  uint8_t uid_length = 0;
  const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

  if (!device_is_ready(i2c_dev)) {
    LOG_ERR("I2C device is not ready");
    while (true) {k_msleep(1000);}
  }

  ret = pn532_get_firmware_version(i2c_dev, &version);
  if (ret == 0)
  {
    LOG_INF("Found chip PN5%x", (version >> 24) & 0xFF);
    LOG_INF("Firmware version %d.%d", (version >> 16) & 0xFF, (version >> 8) & 0xFF);
  }
  else
  {
    LOG_ERR("Error reading firmware version: %d", ret);
    while (true) {k_msleep(1000);}
  }

  pn532_start_passive_target_id_detection(i2c_dev, PN532_MIFARE_ISO14443A);
  pn532_irq_setup();
  while (true)
  {
    pn532_start_passive_target_id_detection(i2c_dev, PN532_MIFARE_ISO14443A);
    ret = pn532_read_passive_target_id(i2c_dev, uid, &uid_length, 1000);
    if (ret == 0)
    {
      LOG_HEXDUMP_INF(uid, uid_length, "Card UID:");
    }
    else
    {
      LOG_WRN("Looking for a card...");
    }
    k_msleep(1000);
  }
}

static int pn532_irq_setup(void)
{
  int ret;
  const struct gpio_dt_spec pn532_irq_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(user_button), gpios);

  if (!gpio_is_ready_dt(&pn532_irq_gpio)) {
    LOG_ERR("Error: pn532_irq_gpio device %s is not ready", pn532_irq_gpio.port->name);
    return 0;
  }

  ret = gpio_pin_configure_dt(&pn532_irq_gpio, GPIO_INPUT);
  if (ret != 0) {
    LOG_ERR("Error %d: failed to configure %s pin %d", ret, pn532_irq_gpio.port->name, pn532_irq_gpio.pin);
    return 0;
  }

  ret = gpio_pin_interrupt_configure_dt(&pn532_irq_gpio, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret != 0) {
    LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", ret, pn532_irq_gpio.port->name, pn532_irq_gpio.pin);
    return 0;
  }

  gpio_init_callback(&pn532_irq_cb_data, pn532_irq_handler_cb, BIT(pn532_irq_gpio.pin));
  gpio_add_callback(pn532_irq_gpio.port, &pn532_irq_cb_data);
  LOG_INF("Set up pn532_irq_gpio at %s pin %d", pn532_irq_gpio.port->name, pn532_irq_gpio.pin);

  return 0;
}

static void pn532_irq_handler_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
  LOG_INF("PN532 IRQ triggered at %" PRIu32, k_cycle_get_32());
}

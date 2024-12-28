// Lib C
#include <assert.h>

// Zephyr includes
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Button);

// User C++ class headers
#include "Button.h"

Button::Button(const struct gpio_dt_spec *gpio) {
  assert(gpio);

  this->gpio = gpio;

  if (!gpio_is_ready_dt(gpio)) {
    LOG_ERR("Error: button device %s is not ready", gpio->port->name);
    return;
  }

  if (gpio_pin_configure_dt(gpio, GPIO_INPUT) != 0) {
    LOG_ERR("Error: Failed to configure %s pin %d", gpio->port->name, gpio->pin);
    return;
  }
}

Button::~Button() {
  // Destructor is automatically called when the object goes out of scope or is explicitly deleted
}

bool Button::isPressed(ButtonPolarity polarity) {
  bool result = false;

  if (polarity == ButtonPolarity::NORMAL) {
    result = (gpio_pin_get_dt(this->gpio) == 0) ? true : false;
  } else {
    result = (gpio_pin_get_dt(this->gpio) == 1) ? true : false;
  }

  return result;
}

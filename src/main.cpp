/*-----------------------------------------------------------------------------------------------*/
/* Includes                                                                                      */
/*-----------------------------------------------------------------------------------------------*/
// Lib C includes
#include <stdlib.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

// User C++ class headers
#include "EventManager.h"
#include "Network.h"
#include "Button.h"

/*-----------------------------------------------------------------------------------------------*/
/* Public functions                                                                              */
/*-----------------------------------------------------------------------------------------------*/
/**
  * @brief  Program entry point
  * @param  None
  * @retval None
  */
int main(void) {
  const struct gpio_dt_spec buttonGpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
  Button button(&buttonGpio);
  event_t eventToPublish = {.id = EVENT_BUTTON_PRESSED};

  Network::getInstance().onGotIP([](const char *ipAddress) {
    event_t eventToPublish = {.id = EVENT_NETWORK_AVAILABLE};

    LOG_INF("Got IP address: %s", ipAddress);
    publishEvent(&eventToPublish, K_NO_WAIT);
  });

  LOG_INF("Waiting for network connection...");
  Network::getInstance().start();

  while (true) {
    if (button.isPressed()) {
      LOG_INF("Button is pressed");
      publishEvent(&eventToPublish, K_NO_WAIT);
    }
    k_msleep(300);
  }

  return EXIT_FAILURE;
}

/**
  * @brief  Fatal error handler callback
  * @param  reason Reason for the fatal error
  * @param  context Exception context, with details and partial or full register
  * @retval None
  */
void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *context) {
  struct k_thread *faultingThread = NULL;

  switch (reason) {
    case K_ERR_CPU_EXCEPTION: {
      LOG_ERR("Generic CPU exception, not covered by other codes");
      break;
    }
    case K_ERR_SPURIOUS_IRQ: {
      LOG_ERR("Unhandled hardware interrupt");
      break;
    }
    case K_ERR_STACK_CHK_FAIL: {
      LOG_ERR("Faulting context overflowed its stack buffer");
      /* Get the current thread that caused the fault */
      faultingThread = k_current_get();
      if (faultingThread) {
        LOG_ERR("Fault occurred in thread: %s", k_thread_name_get(faultingThread));
        LOG_ERR("Thread ID: %p", (void *)faultingThread);
        LOG_ERR("Stack start: %p, size: %zu",
                (void *)faultingThread->stack_info.start,
                faultingThread->stack_info.size);
      } else {
        LOG_ERR("Could not determine faulting thread");
      }
      break;
    }
    case K_ERR_KERNEL_OOPS: {
      LOG_ERR("High severity software error");
      break;
    }
    case K_ERR_ARCH_START: {
      LOG_ERR("Arch specific fatal errors");
      break;
    }
    default: {
      LOG_ERR("Unknow reason for fatal error (%d)", reason);
      break;
    }
  }
}

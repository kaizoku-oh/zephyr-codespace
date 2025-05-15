/*-----------------------------------------------------------------------------------------------*/
/* Includes                                                                                      */
/*-----------------------------------------------------------------------------------------------*/
// Lib C includes
#include <stdlib.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <pn532.h>

/*-----------------------------------------------------------------------------------------------*/
/* Private function prototypes                                                                   */
/*-----------------------------------------------------------------------------------------------*/
static void pn532_example(void);

/*-----------------------------------------------------------------------------------------------*/
/* Public functions                                                                              */
/*-----------------------------------------------------------------------------------------------*/
/**
  * @brief  Program entry point
  * @param  None
  * @retval None
  */
int main(void)
{
  pn532_example();

  while (true) {
    k_msleep(500);
  }

  return EXIT_FAILURE;
}

/**
  * @brief  Fatal error handler callback
  * @param  reason Reason for the fatal error
  * @param  context Exception context, with details and partial or full register
  * @retval None
  */
void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *context)
{
  struct k_thread *faulting_thread = NULL;

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
      faulting_thread = k_current_get();
      if (faulting_thread) {
        LOG_ERR("Fault occurred in thread: %s", k_thread_name_get(faulting_thread));
        LOG_ERR("Thread ID: %p", (void *)faulting_thread);
        LOG_ERR("Stack start: %p, size: %zu",
                (void *)faulting_thread->stack_info.start,
                faulting_thread->stack_info.size);
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

/*-----------------------------------------------------------------------------------------------*/
/* Private functions                                                                             */
/*-----------------------------------------------------------------------------------------------*/
/**
  * @brief  Example of using the PN532 NFC driver
  * @retval None
  */
static void pn532_example(void)
{
  const struct device *dev = DEVICE_DT_GET_ONE(nxp_pn532);
  uint32_t version = 0;

  if (!device_is_ready(dev)) {
    LOG_INF("PN532 device not ready");
    return;
  }

  if (pn532_get_firmware_version(dev, &version) == 0) {
    LOG_INF("PN532 Firmware Version: %02X.%02X", (version >> 8) & 0xFF, version & 0xFF);
  } else {
    LOG_ERR("Failed to get PN532 firmware version");
  }
}

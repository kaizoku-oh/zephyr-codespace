/*-----------------------------------------------------------------------------------------------*/
/* Includes                                                                                      */
/*-----------------------------------------------------------------------------------------------*/
// Lib C includes
#include <stdlib.h>
#include <stdbool.h>

// Zephyr includes
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/crypto/crypto.h>
#include <zephyr/crypto/hash.h>
LOG_MODULE_REGISTER(hash_example);

#if DT_HAS_COMPAT_STATUS_OKAY(st_stm32_hash)
#define HASH_DEV_COMPAT st_stm32_hash
#else
#error "You need to enable one hash device"
#endif

/*-----------------------------------------------------------------------------------------------*/
/* Public functions                                                                              */
/*-----------------------------------------------------------------------------------------------*/
/**
  * @brief  Program entry point
  * @param  None
  * @retval None
  */
int main(void) {
  const struct device *const dev = DEVICE_DT_GET_ONE(HASH_DEV_COMPAT);
  struct hash_ctx ctx = {0};

  if (!device_is_ready(dev)) {
    LOG_ERR("Hash device is not ready");
    return 0;
  }

  if (!hash_begin_session(dev, &ctx, CRYPTO_HASH_ALGO_SHA256)) {
    LOG_ERR("Failed to setup a hash session");
    return 0;
  }

  LOG_INF("Hashing...");
  k_msleep(3000);

  if (!hash_free_session(dev, &ctx)) {
    LOG_ERR("Failed to free hash session");
    return 0;
  }

  while (true) {
    LOG_INF("Running...");
    k_msleep(1000);
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

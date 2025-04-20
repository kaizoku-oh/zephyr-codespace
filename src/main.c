/*-----------------------------------------------------------------------------------------------*/
/* Includes                                                                                      */
/*-----------------------------------------------------------------------------------------------*/
/* Lib C includes */
#include <stdlib.h>
#include <stdbool.h>

/* Zephyr includes */
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

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
  int ret;
  const struct gpio_dt_spec led_gpio = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

  if (!gpio_is_ready_dt(&led_gpio))
  {
    return EXIT_FAILURE;
  }

  ret = gpio_pin_configure_dt(&led_gpio, GPIO_OUTPUT_ACTIVE);
  if (ret < 0)
  {
    return EXIT_FAILURE;
  }

  while (true)
  {
    ret = gpio_pin_toggle_dt(&led_gpio);
    if (ret < 0)
    {
      return EXIT_FAILURE;
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
 void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *context)
 {
  struct k_thread *faulting_thread = NULL;

  switch (reason)
  {
    case K_ERR_CPU_EXCEPTION:
    {
      LOG_ERR("Generic CPU exception, not covered by other codes");
      break;
    }
    case K_ERR_SPURIOUS_IRQ:
    {
      LOG_ERR("Unhandled hardware interrupt");
      break;
    }
    case K_ERR_STACK_CHK_FAIL:
    {
      LOG_ERR("Faulting context overflowed its stack buffer");
      /* Get the current thread that caused the fault */
      faulting_thread = k_current_get();
      if (faulting_thread) {
        LOG_ERR("Fault occurred in thread: %s", k_thread_name_get(faulting_thread));
        LOG_ERR("Thread ID: %p", (void *)faulting_thread);
        LOG_ERR("Stack start: %p, size: %zu",
                (void *)faulting_thread->stack_info.start,
                faulting_thread->stack_info.size);
      }
      else
      {
        LOG_ERR("Could not determine faulting thread");
      }
      break;
    }
    case K_ERR_KERNEL_OOPS:
    {
      LOG_ERR("High severity software error");
      break;
    }
    case K_ERR_ARCH_START:
    {
      LOG_ERR("Arch specific fatal errors");
      break;
    }
    default:
    {
      LOG_ERR("Unknow reason for fatal error (%d)", reason);
      break;
    }
  }
}

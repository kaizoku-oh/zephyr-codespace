#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

int main(void)
{
  LOG_INF("Started...");

  while (true)
  {
    LOG_INF("Running...");
    k_msleep(1000);
  }

  return EXIT_FAILURE;
}

void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *context)
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
      if (faulting_thread)
      {
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
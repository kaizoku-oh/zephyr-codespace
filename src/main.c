#include <stdlib.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

int main(void)
{
  while (true)
  {
    LOG_INF("Running...");
    k_msleep(1000);
  }

  return EXIT_FAILURE;
}

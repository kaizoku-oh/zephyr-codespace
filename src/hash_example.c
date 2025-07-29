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

/*-----------------------------------------------------------------------------------------------*/
/* Macros                                                                                        */
/*-----------------------------------------------------------------------------------------------*/
#if DT_HAS_COMPAT_STATUS_OKAY(st_stm32_hash)
#define HASH_DEV_COMPAT st_stm32_hash
#else
#error "You need to enable one hash device"
#endif /* DT_HAS_COMPAT_STATUS_OKAY(st_stm32_hash) */

/*-----------------------------------------------------------------------------------------------*/
/* Public functions                                                                              */
/*-----------------------------------------------------------------------------------------------*/
/**
  * @brief  Program entry point
  * @param  None
  * @retval None
  */
int main(void) {
{
  const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(hash));
  struct hash_ctx ctx = {0};
  int ret;

  if (!device_is_ready(dev)) {
    LOG_ERR("Hash device is not ready");
    return EXIT_FAILURE;
  }

  ret = hash_begin_session(dev, &ctx, CRYPTO_HASH_ALGO_SHA256);
  if (ret != 0) {
    LOG_ERR("Failed to setup hash session (err=%d)", ret);
    return EXIT_FAILURE;
  }

  LOG_INF("Hash session started successfully");

  k_msleep(3000);

  ret = hash_free_session(dev, &ctx);
  if (ret != 0) {
    LOG_ERR("Failed to free hash session (err=%d)", ret);
    return EXIT_FAILURE;
  }

  while (true) {
    LOG_INF("Running...");
    k_msleep(1000);
  }

  return EXIT_SUCCESS;
}

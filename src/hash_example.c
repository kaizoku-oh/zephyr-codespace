#include <stdlib.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/crypto/crypto.h>
#include <zephyr/crypto/hash.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hash_example);

#define SHA256_DIGEST_LEN (32)

int main(void)
{
  int ret;
  const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(hash));
  struct hash_ctx ctx = {0};
  struct hash_pkt pkt = {0};

  /* String to hash */
  const uint8_t plain_text[] = "Hello Zephyr!";

  /* Buffer to store the output of the hashing operation */
  uint8_t digest[SHA256_DIGEST_LEN] = {0};

  /* Expected hash for the string "Hello Zephyr!" (13 ASCII bytes: no null terminator) */
  static const uint8_t expected_digest[32] = {
    0xe9, 0xfa, 0x10, 0xe3, 0x3b, 0x48, 0x9b, 0x7c,
    0xe9, 0x35, 0x9f, 0x15, 0x25, 0xb5, 0xf0, 0x0f,
    0xc6, 0x6b, 0x69, 0xfa, 0x52, 0xb5, 0x15, 0xfd,
    0x20, 0x9a, 0x8f, 0x26, 0x67, 0xfa, 0xd1, 0x8a
  };

  if (!device_is_ready(dev)) {
    LOG_ERR("Hash device is not ready (err=%d)", ret);
    return EXIT_FAILURE;
  }

  ret = hash_begin_session(dev, &ctx, CRYPTO_HASH_ALGO_SHA256);
  if (ret != 0) {
    LOG_ERR("Failed to setup hash session (err=%d)", ret);
    return EXIT_FAILURE;
  }

  pkt.in_buf = (uint8_t *)plain_text;
  pkt.in_len = sizeof(plain_text) - 1;
  pkt.out_buf = digest;

  ret = hash_compute(&ctx, &pkt);
  if (ret != 0) {
    LOG_ERR("Hash computation failed (err=%d)", ret);
    hash_free_session(dev, &ctx);
    return EXIT_FAILURE;
  }

  LOG_INF("Hash computed successfully");

  if (memcmp(digest, expected_digest, SHA256_DIGEST_LEN) == 0) {
    LOG_INF("Digest matches expected value");
  } else {
    LOG_ERR("Digest mismatch");
  }
  LOG_HEXDUMP_INF(digest, SHA256_DIGEST_LEN, "Computed SHA-256 Digest:");
  LOG_HEXDUMP_INF(expected_digest, SHA256_DIGEST_LEN, "Expected SHA-256 Digest:");

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

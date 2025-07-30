#include <stdlib.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/crypto/crypto.h>
#include <zephyr/crypto/hash.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hash_example);

#define SHA256_DIGEST_LEN (32)
#define SHA224_DIGEST_LEN (28)

/* String to hash */
static const uint8_t plain_text[] = "Hello Zephyr!";

/* Expected hash for the string "Hello Zephyr!" (13 ASCII bytes: no null terminator) */
static const uint8_t sha256_expected_digest[SHA256_DIGEST_LEN] = {
  0x04, 0xef, 0x6a, 0x62, 0x87, 0xf5, 0x35, 0x74,
  0x89, 0x8d, 0xd9, 0x88, 0xdc, 0x84, 0x94, 0xa1,
  0x6b, 0x6a, 0x86, 0x83, 0x1d, 0x21, 0x15, 0x5d,
  0x52, 0xcb, 0x1c, 0xe7, 0xb4, 0xd8, 0x3a, 0x19
};

static const uint8_t sha224_expected_digest[SHA224_DIGEST_LEN] = {
  0x48, 0xed, 0xbf, 0x58, 0xea, 0x7e, 0xf7, 0xc9,
  0x1e, 0x4e, 0x18, 0x5c, 0xd2, 0xa9, 0x44, 0x93,
  0x72, 0x5d, 0x1d, 0xed, 0xa6, 0xff, 0x99, 0x3b,
  0xb5, 0xc8, 0x84, 0xa7
};

/* Buffers to store the output of the hashing operation */
static uint8_t sha256_digest[SHA256_DIGEST_LEN] = {0};
static uint8_t sha224_digest[SHA224_DIGEST_LEN] = {0};

int main(void)
{
  int ret;
  const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(hash));
  struct hash_ctx ctx = {0};
  struct hash_pkt pkt = {0};

  if (!device_is_ready(dev)) {
    LOG_ERR("Hash device is not ready");
    return EXIT_FAILURE;
  }

  /* ======================================================== */
  /* ======================== SHA256 ======================== */
  /* ======================================================== */

  LOG_INF("=================================================================");

  ret = hash_begin_session(dev, &ctx, CRYPTO_HASH_ALGO_SHA256);
  if (ret != 0) {
    LOG_ERR("Failed to setup hash session (err=%d)", ret);
    return EXIT_FAILURE;
  }

  pkt.in_buf = (uint8_t *)plain_text;
  pkt.in_len = sizeof(plain_text) - 1;
  pkt.out_buf = sha256_digest;

  ret = hash_compute(&ctx, &pkt);
  if (ret != 0) {
    LOG_ERR("Hash computation failed (err=%d)", ret);
    hash_free_session(dev, &ctx);
    return EXIT_FAILURE;
  }

  if (memcmp(sha256_digest, sha256_expected_digest, SHA256_DIGEST_LEN) == 0) {
    LOG_INF("Digest matches expected value");
  } else {
    LOG_ERR("Digest mismatch");
  }
  LOG_HEXDUMP_INF(sha256_digest, SHA256_DIGEST_LEN, "Computed SHA-256 Digest:");
  LOG_HEXDUMP_INF(sha256_expected_digest, SHA256_DIGEST_LEN, "Expected SHA-256 Digest:");

  ret = hash_free_session(dev, &ctx);
  if (ret != 0) {
    LOG_ERR("Failed to free hash session (err=%d)", ret);
    return EXIT_FAILURE;
  }

  LOG_INF("=================================================================");

  /* ======================================================== */
  /* ======================== SHA224 ======================== */
  /* ======================================================== */

  ret = hash_begin_session(dev, &ctx, CRYPTO_HASH_ALGO_SHA224);
  if (ret != 0) {
    LOG_ERR("Failed to setup hash session (err=%d)", ret);
    return EXIT_FAILURE;
  }

  pkt.in_buf = (uint8_t *)plain_text;
  pkt.in_len = sizeof(plain_text) - 1;
  pkt.out_buf = sha224_digest;

  ret = hash_compute(&ctx, &pkt);
  if (ret != 0) {
    LOG_ERR("Hash computation failed (err=%d)", ret);
    hash_free_session(dev, &ctx);
    return EXIT_FAILURE;
  }

  if (memcmp(sha224_digest, sha224_expected_digest, SHA224_DIGEST_LEN) == 0) {
    LOG_INF("Digest matches expected value");
  } else {
    LOG_ERR("Digest mismatch");
  }
  LOG_HEXDUMP_INF(sha224_digest, SHA224_DIGEST_LEN, "Computed SHA-224 Digest:");
  LOG_HEXDUMP_INF(sha224_expected_digest, SHA224_DIGEST_LEN, "Expected SHA-224 Digest:");

  ret = hash_free_session(dev, &ctx);
  if (ret != 0) {
    LOG_ERR("Failed to free hash session (err=%d)", ret);
    return EXIT_FAILURE;
  }

  LOG_INF("=================================================================");

  return EXIT_SUCCESS;
}

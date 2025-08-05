#include <zephyr/crypto/crypto.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hash_example);

#define SHA256_DIGEST_LEN (32)
#define SHA224_DIGEST_LEN (28)

static const uint8_t test_data_256[] = "Thread1 SHA256 data";
static const uint8_t test_data_224[] = "Thread2 SHA224 data";

static const uint8_t expected_digest_256[SHA256_DIGEST_LEN] = {
    0x1f, 0x1b, 0x55, 0x79, 0x3e, 0x5a, 0x48, 0x42, 0x0a, 0x28, 0xfe,
    0xc7, 0xeb, 0xee, 0x94, 0xcd, 0x7e, 0x25, 0xf9, 0x26, 0x18, 0xe8,
    0x6a, 0xaa, 0xf7, 0x1d, 0x2f, 0x02, 0x86, 0x1c, 0x67, 0xd4};

static const uint8_t expected_digest_224[SHA224_DIGEST_LEN] = {
    0x14, 0xca, 0xee, 0x76, 0xf6, 0x54, 0x95, 0xf3, 0xe9, 0xed,
    0xa9, 0x10, 0xe2, 0x9b, 0x0b, 0x4a, 0x62, 0xe8, 0xf5, 0x67,
    0x12, 0x27, 0xcf, 0x97, 0x1a, 0x4f, 0xe8, 0x8b};

static void do_hashing(enum hash_algo algo, const uint8_t *input, size_t in_len,
                       const uint8_t *expected_digest, size_t digest_len,
                       const char *label) {
  int ret;
  const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(hash));
  struct hash_ctx ctx = {0};
  struct hash_pkt pkt = {0};
  uint8_t digest[SHA256_DIGEST_LEN] = {0};

  ret = hash_begin_session(dev, &ctx, algo);
  if (ret != 0) {
    LOG_ERR("%s: begin_session failed (%d)", label, ret);
    return;
  }

  pkt.in_buf = (uint8_t *)input;
  pkt.in_len = in_len;
  pkt.out_buf = digest;

  ret = hash_compute(&ctx, &pkt);
  if (ret != 0) {
    LOG_ERR("%s: compute failed (%d)", label, ret);
    hash_free_session(dev, &ctx);
    return;
  }

  if (memcmp(digest, expected_digest, digest_len) == 0) {
    LOG_INF("%s: Digest OK", label);
  } else {
    LOG_ERR("%s: Digest MISMATCH", label);
  }

  LOG_HEXDUMP_DBG(digest, digest_len, label);

  ret = hash_free_session(dev, &ctx);
  if (ret != 0) {
    LOG_ERR("%s: free_session failed (%d)", label, ret);
    return;
  }
}

static void thread_sha256(void) {
  do_hashing(CRYPTO_HASH_ALGO_SHA256, test_data_256, sizeof(test_data_256) - 1,
             expected_digest_256, SHA256_DIGEST_LEN, "SHA256-Thread1");
}

static void thread_sha224(void) {
  do_hashing(CRYPTO_HASH_ALGO_SHA224, test_data_224, sizeof(test_data_224) - 1,
             expected_digest_224, SHA224_DIGEST_LEN, "SHA224-Thread2");
}

K_THREAD_DEFINE(sha256_thread, 2048, thread_sha256, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(sha224_thread, 2048, thread_sha224, NULL, NULL, NULL, 5, 0, 0);

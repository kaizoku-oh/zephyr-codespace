// Lib C includes
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// Zephyr includes
#include <app_version.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/dfu/flash_img.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Updater);

// User C++ class headers
#include "EventManager.h"
#include "HttpClient.h"

// Function declarations
static void updaterThreadHandler();
static void onNetworkAvailableAction();
static void startOtaUpdateAction();
static void downloadImage(const char *host, const char *endpoint);
static bool confirmCurrentImage();
static int shellUpdateCommandHandler(const struct shell *shell, size_t argc, char **argv);
static void drawProgressBar(uint32_t total, uint32_t progress);

// ZBUS subscribers definition
ZBUS_SUBSCRIBER_DEFINE(updaterSubscriber, 4);

// Add a subscriber observer to ZBUS events channel
ZBUS_CHAN_ADD_OBS(eventsChannel, updaterSubscriber, 4);

// Thread definition
K_THREAD_DEFINE(updaterThread, 1024*8, updaterThreadHandler, NULL, NULL, NULL, 7, 0, 0);

// Shell command registration
SHELL_CMD_ARG_REGISTER(update, NULL, "Start OTA update process", shellUpdateCommandHandler, 1, 0);

// Event-Action pairs
static const event_action_pair_t eventActionList[] {
  {EVENT_OTA_UPDATE_SHELL_CMD, startOtaUpdateAction    },
  {EVENT_BUTTON_PRESSED,       startOtaUpdateAction    },
  {EVENT_NETWORK_AVAILABLE,    onNetworkAvailableAction},
};

static volatile bool networkIsAvailable = false;
static struct flash_img_context flashContext = {0};
static size_t totalDownloadSize = 0;
static size_t currentDownloadedSize = 0;
#ifdef VERIFY_DOWNLOADED_IMAGE_HASH
static struct flash_img_check flashImageCheck = {0};
#endif // VERIFY_DOWNLOADED_IMAGE_HASH

static void updaterThreadHandler() {
  int ret = 0;
  event_t event = {.id = EVENT_INITIAL_VALUE};

  if (confirmCurrentImage() == false) {
    LOG_ERR("Failed to confirm current image");
    while (true) { k_msleep(1000); }
  }
  while (true) {
    ret = waitForEvent(&updaterSubscriber, &event, K_FOREVER);
    if (ret == 0) {
      processEvent(&event, eventActionList, EVENT_ACTION_LIST_SIZE(eventActionList));
    }
  }
}

static void onNetworkAvailableAction() {
  LOG_INF("Network is now available");
  networkIsAvailable = true;
}

static void startOtaUpdateAction() {
  if (networkIsAvailable) {
    downloadImage("192.168.1.25", "/zephyr.signed.bin");
    if (boot_request_upgrade(BOOT_UPGRADE_TEST)) {
      LOG_ERR("Failed to mark the image in slot 1 as pending");
      return;
    }
    LOG_INF("You need to reboot your system to apply the new update");
  } else {
    LOG_WRN("Network is not available, cannot start update");
  }
}

static void downloadImage(const char *host, const char *endpoint) {
  int ret = 0;

  assert(host);
  assert(endpoint);

  HttpClient client((char *)host);

  // Initialize context needed for writing the image to the flash
  ret = flash_img_init(&flashContext);
  if (ret < 0) {
    LOG_ERR("Flash context init error: %d", ret);
    return;
  }

  // Download image
  client.get(endpoint, [](HttpResponse *response) {
    int ret = 0;
    size_t totalSizeWrittenToFlash = 0;

    if (totalDownloadSize == 0) {
      totalDownloadSize = response->totalSize;
      LOG_INF("Image size to download: %.3f kb", (float)totalDownloadSize / 1024);
    }

    ret = flash_img_buffered_write(&flashContext,
                                   response->body,
                                   response->bodyLength,
                                   (response->isComplete));
    if (ret < 0) {
      LOG_ERR("Flash write error: %d", ret);
      totalDownloadSize = 0;
      currentDownloadedSize = 0;
      return;
    }

    currentDownloadedSize += response->bodyLength;
    drawProgressBar(totalDownloadSize, currentDownloadedSize);

    if (response->isComplete) {
      totalSizeWrittenToFlash = flash_img_bytes_written(&flashContext);
      if ((currentDownloadedSize == totalDownloadSize) &&
          (totalDownloadSize == totalSizeWrittenToFlash)) {
          printk("✅\r\n");
        LOG_INF("Download completed successfully");
        totalDownloadSize = 0;
        currentDownloadedSize = 0;
#ifdef VERIFY_DOWNLOADED_IMAGE_HASH
        // Verify the hash of the stored firmware
        flashImageCheck.match = fileHash;
        flashImageCheck.clen = totalDownloadSize;
        if (flash_img_check(&flashContext, &flashImageCheck, FIXED_PARTITION_ID(slot1_partition))) {
          LOG_ERR("Firmware - flash validation has failed");
          return;
        }
#endif // VERIFY_DOWNLOADED_IMAGE_HASH
      } else {
        printk("❌\r\n");
        LOG_ERR("The size written to flash is different than the one downloaded");
        LOG_INF("totalDownloadSize=%d", totalDownloadSize);
        LOG_INF("currentDownloadedSize=%d", currentDownloadedSize);
        LOG_INF("totalSizeWrittenToFlash=%d", totalSizeWrittenToFlash);
        totalDownloadSize = 0;
        currentDownloadedSize = 0;
      }
    }
  });
}

static bool confirmCurrentImage() {
  int ret = 0;
  bool imageIsConfirmed = false;
  struct mcuboot_img_header header = {0};

  if (boot_read_bank_header(FIXED_PARTITION_ID(slot0_partition), &header, sizeof(header)) != 0) {
    LOG_ERR("Failed to read slot0_partition header");
    return false;
  }

  LOG_INF("Bootloader version: %d.x.y", header.mcuboot_version);
  LOG_INF("Application version: %d.%d.%d",
          header.h.v1.sem_ver.major,
          header.h.v1.sem_ver.minor,
          header.h.v1.sem_ver.revision);

  // On boot verify if current image is confirmed, if not confirm it
  imageIsConfirmed = boot_is_img_confirmed();
  LOG_INF("Image is%s confirmed", imageIsConfirmed ? "" : " not");
  if (!imageIsConfirmed) {
    LOG_INF("Marking image as confirmed...");
    ret = boot_write_img_confirmed();
    if (ret < 0) {
      LOG_ERR("Couldn't confirm this image: %d", ret);
      return false;
    }

    LOG_INF("Marked image as confirmed");
    ret = boot_erase_img_bank(FIXED_PARTITION_ID(slot1_partition));
    if (ret) {
      LOG_ERR("Failed to erase second slot: %d", ret);
      return false;
    }
  }
  return true;
}

static int shellUpdateCommandHandler(const struct shell *shell, size_t argc, char **argv) {
  event_t eventToPublish = {.id = EVENT_OTA_UPDATE_SHELL_CMD};

  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  shell_print(shell, "Starting OTA update...");
  if (networkIsAvailable) {
    publishEvent(&eventToPublish, K_NO_WAIT);
  } else {
    shell_error(shell, "Network is not available. Please ensure connectivity.");
  }

  return 0;
}

static void drawProgressBar(uint32_t total, uint32_t progress) {
  uint32_t percent  = 0;
  uint32_t filledBlocks  = 0;
  uint32_t index  = 0;

  // Calculate the percentage
  percent = (progress * 100) / total;

  // Calculate the number of blocks to fill
  // 5% per block, so there will be 20 blocks in total
  filledBlocks = (percent / 5);

  // Print the percentage and the progress bar
  printk("\r%%%-3d [", percent);

  // Print the filled blocks
  for (index = 0; index < filledBlocks; index++) { printk("█"); }

  // Print the empty blocks
  for (index = filledBlocks; index < 20; index++) { printk(" "); }

  printk("]");
}

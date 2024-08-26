#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pn532);

#define PN532_I2C_ADDR 0x24
#define PN532_I2C_RDY_BYTE ((uint8_t)0x01)
#define PN532_PREAMBLE 0x00
#define PN532_STARTCODE1 0x00
#define PN532_STARTCODE2 0xFF
#define PN532_POSTAMBLE 0x00
#define PN532_DIRECTION_HOST_TO_PN532 0xD4
#define PN532_DIRECTION_PN532_TO_HOST 0xD5
#define PN532_COMMAND_GET_FIRMWARE_VERSION 0x02
#define PN532_COMMAND_ENLIST_PASSIVE_TARGET 0x4A
#define PN532_COMMAND_MAX_SIZE (255U)
#define PN532_PACKET_MAX_SIZE (PN532_COMMAND_MAX_SIZE) + (8U)
#define PN532_ACK_PACKET_MAX_SIZE (6U)
#define DETECT_ONE_CARD_AT_A_TIME (1U)
#define PN532_COMMAND_SAMCONFIGURATION (0x14)      ///< SAM configuration

#define TWOS_COMPLEMENT(val) (~(val) + (1U))

static const uint8_t PN532_ACK_PACKET[PN532_ACK_PACKET_MAX_SIZE] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

/**
 * @brief Checks if the PN532 device is ready for communication.
 *
 * This function polls the PN532 device to check if it is ready by reading
 * a specific byte from the device and comparing it with a predefined
 * ready byte value. It waits up to the specified timeout period for the
 * device to indicate readiness.
 *
 * @param i2c_dev Initialized I2C device
 * @param timeout Maximum time to wait for the device to be ready, in milliseconds.
 *
 * @return `true` if the device is ready, `false` otherwise.
 */
static bool _pn532_is_ready(const struct device *i2c_dev, uint16_t timeout)
{
  int ret;
  uint8_t rdy_byte = 0;
  uint16_t time_elapsed = 0;
  bool device_is_ready = false;

  while (time_elapsed < timeout)
  {
    ret = i2c_read(i2c_dev, &rdy_byte, 1, PN532_I2C_ADDR);
    if ((ret == 0) && (rdy_byte == PN532_I2C_RDY_BYTE))
    {
      device_is_ready = true;
      break;
    }
    else
    {
      k_msleep(10);
      time_elapsed += 10;
    }
  }

  return device_is_ready;
}

/**
 * @brief Builds and sends a command packet to the PN532 device.
 *
 * @param i2c_dev Initialized I2C device
 * @param command Pointer to the command data.
 * @param command_length Length of the command data.
 * @param timeout Maximum time to wait for the device to be ready.
 *
 * @return 0 on success, negative error code on failure.
 */
static int _pn532_send_command(const struct device *i2c_dev, uint8_t *command, uint8_t command_length, uint16_t timeout)
{
  int ret;
  uint8_t command_data_sum = 0;
  uint8_t direction_byte = PN532_DIRECTION_HOST_TO_PN532;
  uint8_t packet_length = 0;
  uint8_t packet[PN532_PACKET_MAX_SIZE] = {0};
  uint8_t acknowledgement[PN532_ACK_PACKET_MAX_SIZE + sizeof(PN532_I2C_RDY_BYTE)] = {0};

  /* 0. Check arguments validity */
  if (i2c_dev && command && command_length)
  {
    /* 1. Build the packet that encapsulate the command to send:
     *  ---------------------- ---------------------------------------------------- ------------------------------------------
     * | Field                | Description                                        |    Value                                 |
     * |----------------------|----------------------------------------------------|------------------------------------------|
     * | Preamble             | Start of the packet                                | 0x00                                     |
     * | Start Code           | Start code to identify the beginning of the packet | 0x00, 0xFF                               |
     * | Length               | Length of the packet (data field + TFI byte)       | Number of bytes in Data Field + 1        |
     * | Length Checksum      | Checksum for Length field                          | ~Length + 1                              |
     * | Direction Byte (TFI) | Type Field Identifier (TFI) or Direction Byte      | 0xD4                                     |
     * | Data Field           | The command and its parameters                     | Command data                             |
     * | Data Checksum        | Checksum for the Data Field                        | ~(Direction Byte + Data Field bytes) + 1 |
     * | Postamble            | End of the packet                                  | 0x00                                     |
     *  ----------------------------------------------------------------------------------------------------------------------
     */
    packet[packet_length++] = PN532_PREAMBLE;
    packet[packet_length++] = PN532_STARTCODE1;
    packet[packet_length++] = PN532_STARTCODE2;
    packet[packet_length++] = command_length + sizeof(direction_byte);
    packet[packet_length++] = TWOS_COMPLEMENT(command_length + sizeof(direction_byte));
    packet[packet_length++] = direction_byte;

    memcpy(&packet[packet_length], command, command_length);
    packet_length += command_length;

    for (uint8_t i = 0; i < command_length; i++) {command_data_sum += command[i];}
    packet[packet_length++] = TWOS_COMPLEMENT(direction_byte + command_data_sum);

    packet[packet_length++] = PN532_POSTAMBLE;

    /* 2. Send the packet */
    ret = i2c_write(i2c_dev, packet, packet_length, PN532_I2C_ADDR);
    if (ret == 0)
    {
      /* 3. Wait for device to be ready */
      if (_pn532_is_ready(i2c_dev, timeout))
      {
        /* 4. Read and verify the received acknowledgment for the sent command */
        ret = i2c_read(i2c_dev, acknowledgement, PN532_ACK_PACKET_MAX_SIZE + 1, PN532_I2C_ADDR);
        if (ret == 0)
        {
          /* When comparing ignore the first byte (RDY_BYTE) */
          ret = (memcmp(&acknowledgement[1], PN532_ACK_PACKET, PN532_ACK_PACKET_MAX_SIZE) == 0) ? 0 : -EIO;
        }
        else
        {
          ret = -EIO;
        }
      }
      else
      {
        ret = -ETIMEDOUT;
      }
    }
    else
    {
      ret = -EIO;
    }
  }
  else
  {
    ret = -EINVAL;
  }

  return ret;
}

/**
 * @brief Reads a response packet from the PN532 device.
 *
 * @param i2c_dev Initialized I2C device
 * @param response Pointer to the buffer where the response will be stored.
 * @param length Number of bytes to read from the device.
 * @param timeout Maximum time to wait for the device to be ready.
 *
 * @return 0 on success, negative error code on failure.
 */
static int _pn532_read_response(const struct device *i2c_dev, uint8_t *response, uint8_t length, uint16_t timeout)
{
  int ret;
  uint8_t temporary_buffer[PN532_PACKET_MAX_SIZE] = {0};

  /* 0. Check arguments */
  if ((i2c_dev != NULL) && (response != NULL) && (length > 0) && (length + 1 < PN532_PACKET_MAX_SIZE))
  {
    /* 1. Wait for device to be ready */
    if (_pn532_is_ready(i2c_dev, timeout))
    {
      /* 2. Read packet from the device */
      ret = i2c_read(i2c_dev, temporary_buffer, length + 1, PN532_I2C_ADDR);
      if (ret == 0)
      {
        /* 3. Copy the actual response data to the provided buffer and ignore the first byte (RDY_BYTE) */
        memcpy(response, &temporary_buffer[1], length);
        ret = 0;
      }
      else
      {
        ret = -EIO;
      }
    }
    else
    {
      ret = -ETIMEDOUT;
    }
  }
  else
  {
    ret = -EINVAL;
  }

  return ret;
}

/**
 * @brief Retrieves the firmware version from the PN532 device.
 *
 * Sends a command to the PN532 device to get the firmware version and processes the response.
 *
 * @param i2c_dev Initialized I2C device
 * @param version Pointer to a variable where the firmware version will be stored.
 * @return 0 on success, or a negative error code on failure.
 */
int pn532_get_firmware_version(const struct device *i2c_dev, uint32_t *version)
{
  int ret;
  const uint8_t expected_version[] = { 0x00, 0x00, 0xFF, 0x06, 0xFA, 0xD5};
  uint8_t buffer[13] = {0};
  uint8_t version_offset = 7;

  if (i2c_dev && version)
  {
    buffer[0] = PN532_COMMAND_GET_FIRMWARE_VERSION;
    ret = _pn532_send_command(i2c_dev, buffer, 1, 1000);
    if (ret == 0)
    {
      ret = _pn532_read_response(i2c_dev, buffer, 13, 1000);
      if (ret == 0)
      {
        if (memcmp(buffer, expected_version, sizeof(expected_version)) == 0)
        {
          *version = buffer[version_offset++];
          *version <<= 8;
          *version |= buffer[version_offset++];
          *version <<= 8;
          *version |= buffer[version_offset++];
          *version <<= 8;
          *version |= buffer[version_offset++];
          ret = 0;
        }
        else
        {
          ret = -EINVAL;
        }
      }
      else
      {
        ret = -EIO;
      }
    }
    else
    {
      ret = -EIO;
    }
  }
  else
  {
    ret = -EINVAL;
  }

  return ret;
}

/**
 * @brief Starts the detection of a passive target (e.g., an NFC card or tag).
 *
 * This function initiates the detection of a passive target by sending the 
 * `ENLIST_PASSIVE_TARGET` command to the PN532 device. It operates in non-blocking 
 * mode, so interrupts must be enabled to handle the detection results. The function 
 * can be used to detect a single passive target at a specified baud rate.
 *
 * @param i2c_dev Initialized I2C device.
 * @param card_baudrate The baud rate to use for detecting the card. Typical values are:
 *        - 0x00 for 106 kbps (ISO14443A/MiFare)
 *        - 0x01 for 212 kbps (FeliCa)
 *        - 0x02 for 424 kbps (FeliCa)
 *
 * @return 0 on success, or a negative error code on failure.
 *         - -EINVAL if the i2c_dev parameter is invalid.
 *         - Negative error codes from `_pn532_send_command()` if sending the command fails.
 */
int pn532_start_passive_target_id_detection(const struct device *i2c_dev, uint8_t card_baudrate)
{
  int ret;
  uint8_t buffer[16] = {0};

  if (i2c_dev)
  {
    /* Prepare command buffer */
    buffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    buffer[1] = 0x01; // normal mode;
    buffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    buffer[3] = 0x01; // use IRQ pin!
    /* Send command */
    ret = _pn532_send_command(i2c_dev, buffer, 3, 1000);
    /* Reuse the same buffer for reading the response */
    ret = _pn532_read_response(i2c_dev, buffer, 9, 1000);
    if (buffer[6] == 0x15)
    {
      ret = 0;
    }
    else
    {
      ret = -EIO;
    }

    /* Prepare command buffer */
    buffer[0] = PN532_COMMAND_ENLIST_PASSIVE_TARGET;
    buffer[1] = DETECT_ONE_CARD_AT_A_TIME;
    buffer[2] = card_baudrate;
    /* Send command */
    ret = _pn532_send_command(i2c_dev, buffer, 3, 1000);
  }
  else
  {
    ret = -EINVAL;
  }

  return ret;
}

/**
 * @brief Reads the UID of a passive target (card/tag) in the field.
 *
 * This function sends a command to detect a passive target and reads its UID.
 *
 * @param i2c_dev Initialized I2C device
 * @param uid Pointer to the buffer where the UID will be stored (up to 7 bytes)
 * @param uid_length Pointer to the variable where the length of the UID will be stored
 * @param timeout Maximum time to wait for the device to be ready, in milliseconds.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int pn532_read_passive_target_id(const struct device *i2c_dev, uint8_t *uid, uint8_t *uid_length, uint16_t timeout)
{
  int ret;
  uint8_t buffer[20] = {0};

  if (i2c_dev && uid && uid_length)
  {
    /* Reuse the same buffer for reading the response */
    ret = _pn532_read_response(i2c_dev, buffer, 20, timeout);
    if (ret == 0)
    {
      /* Make sure we've detected only one card */
      if (buffer[7] == 1)
      {
        /* Extract UID length and UID */
        *uid_length = buffer[12];
        memcpy(uid, &buffer[13], *uid_length);
        ret = 0;
      }
      else
      {
        ret = -EIO;
      }
    }
  }
  else
  {
    ret = -EINVAL;
  }

  return ret;
}

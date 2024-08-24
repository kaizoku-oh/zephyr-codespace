#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <string.h>

#define PN532_I2C_ADDR 0x24 // Adjust to the correct I2C address for PN532
#define PN532_PREAMBLE 0x00
#define PN532_STARTCODE1 0x00
#define PN532_STARTCODE2 0xFF
#define PN532_POSTAMBLE 0x00
#define PN532_HOSTTOPN532 0xD4
#define PN532_PN532TOHOST 0xD5
#define PN532_COMMAND_GETFIRMWAREVERSION 0x02

LOG_MODULE_REGISTER(pn532_example);

const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
uint8_t pn532_packetbuffer[13] = {0};

bool sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout);
void readdata(uint8_t *buffer, uint8_t length);
void writecommand(uint8_t *cmd, uint8_t cmdlen);
bool waitready(uint16_t timeout);
bool readack();
bool isready();
static void nfc_thread_handler(void);

K_THREAD_DEFINE(nfc_thread, 1024, nfc_thread_handler, NULL, NULL, NULL, 7, 0, 0);

/*!
    @brief  Gets the firmware version of the PN532 chip via I2C.

    @returns  The chip's firmware version and ID, or 0 if there's an error.
*/
uint32_t getFirmwareVersion(void) {
    uint32_t response;
    ///< Expected firmware version message from PN532
    uint8_t pn532response_firmwarevers[] = { 0x00, 0x00, 0xFF, 0x06, 0xFA, 0xD5};

    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

    if (!sendCommandCheckAck(pn532_packetbuffer, 1, 1000)) {
        return 0; // Command failed
    }

    // Read data packet
    readdata(pn532_packetbuffer, 13);

    // Check some basic stuff
    if (memcmp(pn532_packetbuffer, pn532response_firmwarevers, 6) != 0) {
        LOG_ERR("Firmware doesn't match!");
        return 0;
    }

    int offset = 7;
    response = pn532_packetbuffer[offset++];
    response <<= 8;
    response |= pn532_packetbuffer[offset++];
    response <<= 8;
    response |= pn532_packetbuffer[offset++];
    response <<= 8;
    response |= pn532_packetbuffer[offset++];

    return response;
}

void writecommand(uint8_t *cmd, uint8_t cmdlen) {
    uint8_t packet[8 + cmdlen];
    uint8_t LEN = cmdlen + 1;
    uint8_t sum = 0;

    packet[0] = PN532_PREAMBLE;
    packet[1] = PN532_STARTCODE1;
    packet[2] = PN532_STARTCODE2;
    packet[3] = LEN;
    packet[4] = ~LEN + 1;
    packet[5] = PN532_HOSTTOPN532;

    for (uint8_t i = 0; i < cmdlen; i++) {
        packet[6 + i] = cmd[i];
        sum += cmd[i];
    }

    packet[6 + cmdlen] = ~(PN532_HOSTTOPN532 + sum) + 1;
    packet[7 + cmdlen] = PN532_POSTAMBLE;

    // Write the packet to the I2C bus
    i2c_write(i2c_dev, packet, sizeof(packet), PN532_I2C_ADDR);
}

bool sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout) {
    writecommand(cmd, cmdlen);

    // Wait for chip to be ready
    if (!waitready(timeout)) {
        return false;
    }

    // Read acknowledgment
    if (!readack()) {
        LOG_ERR("No ACK frame received!");
        return false;
    }

    // Wait for chip to be ready
    if (!waitready(timeout)) {
        return false;
    }

    return true; // Command acknowledged
}

void readdata(uint8_t *buffer, uint8_t length) {
    uint8_t temp_buffer[length + 1];
    i2c_read(i2c_dev, temp_buffer, length + 1, PN532_I2C_ADDR);
    memcpy(buffer, &temp_buffer[1], length);
}

bool waitready(uint16_t timeout) {
    uint16_t timer = 0;
    while (!isready()) {
        if (timeout != 0) {
            timer += 10;
            if (timer > timeout) {
                LOG_ERR("Timeout waiting for device to be ready");
                return false;
            }
        }
        k_sleep(K_MSEC(10));
    }
    return true;
}

bool isready() {
    uint8_t rdy;
    i2c_read(i2c_dev, &rdy, 1, PN532_I2C_ADDR);
    return (rdy == 0x01);
}

bool readack() {
    uint8_t ackbuff[6];
    readdata(ackbuff, 6);
    uint8_t pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    return (memcmp(ackbuff, pn532ack, 6) == 0);
}


static void nfc_thread_handler(void)
{
  int ret = 0;
  uint32_t version = 0;

  /* Check if i2c device is ready to be used */
  if (!device_is_ready(i2c_dev)) {
    LOG_ERR("I2C device is not ready");
    while (true);
  }

  while (true)
  {
    version = getFirmwareVersion();
    if (ret == 0) {
        // Print chip module and firmware version
        LOG_INF("Found chip PN5%x", (version >> 24) & 0xFF);
        LOG_INF("Firmware version %d.%d", (version >> 16) & 0xFF, (version >> 8) & 0xFF);
    } else {
      LOG_ERR("Error reading firmware version: %d", ret);
    }

    k_msleep(1000);
  }
}

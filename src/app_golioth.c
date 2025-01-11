/*
 * Copyright (c) 2023 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_golioth, LOG_LEVEL_DBG);

#include <golioth/client.h>
#include <samples/common/sample_credentials.h>
#include <golioth/lightdb_state.h>

#include <samples/common/net_connect.h>

const struct device *temperature_dev = DEVICE_DT_GET(DT_NODELABEL(die_temp));
struct golioth_client *client = NULL;
static K_SEM_DEFINE(connected, 0, 1);

static void on_client_event(struct golioth_client *client,
                            enum golioth_client_event event,
                            void *arg)
{
    bool is_connected = (event == GOLIOTH_CLIENT_EVENT_CONNECTED);

    if (is_connected)
    {
        k_sem_give(&connected);
    }
    LOG_INF("Golioth client %s", is_connected ? "connected" : "disconnected");
}

int main(void)
{
    int ret = 0;
    double temperature = 0;
    struct sensor_value value = {0};
    /* Note: In production, you would provision unique credentials onto each
     * device. For simplicity, we provide a utility to hardcode credentials as
     * kconfig options in the samples.
     */
    const struct golioth_client_config *client_config = golioth_sample_credentials_get();

    if (!device_is_ready(temperature_dev))
    {
        LOG_ERR("Error: Device is not ready");
        return -1;
    }

    net_connect();

    client = golioth_client_create(client_config);

    golioth_client_register_event_callback(client, on_client_event, NULL);

    k_sem_take(&connected, K_FOREVER);

    while (true)
    {
        ret = sensor_sample_fetch(temperature_dev);
        if (ret)
        {
            LOG_ERR("Failed to fetch sample (%d)\n", ret);
            return -1;
        }

        ret = sensor_channel_get(temperature_dev, SENSOR_CHAN_DIE_TEMP, &value);
        if (ret)
        {
            LOG_ERR("Failed to get data (%d)\n", ret);
            return -1;
        }

        temperature = sensor_value_to_double(&value);
        LOG_INF("CPU temperature: %.2f °C", temperature);

        ret = golioth_lightdb_set_int_sync(client, "temperature", sensor_value_to_double(&value), 3);
        if (ret)
        {
            LOG_WRN("Failed to set temperature: %d", ret);
            // return -1;
        }

        k_sleep(K_SECONDS(5));
    }

    return 0;
}

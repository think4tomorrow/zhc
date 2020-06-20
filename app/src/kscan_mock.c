/*
 * Copyright (c) 2020 Peter Johanson <peter@peterjohanson.com>
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_kscan_mock

#include <device.h>
#include <drivers/kscan.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/kscan-mock.h>

struct kscan_mock_data
{
    kscan_callback_t callback;

    u8_t event_index;
    struct k_delayed_work work;
    struct device *dev;
};

static int kscan_mock_disable_callback(struct device *dev)
{
    struct kscan_mock_data *data = dev->driver_data;

    k_delayed_work_cancel(&data->work);
    return 0;
}

static int kscan_mock_configure(struct device *dev, kscan_callback_t callback)
{
    struct kscan_mock_data *data = dev->driver_data;

    if (!callback)
    {
        return -EINVAL;
    }

    data->event_index = 0;
    data->callback = callback;

    return 0;
}

#define MOCK_INST_INIT(n)                                                          \
    struct kscan_mock_config_##n                                                   \
    {                                                                              \
        u32_t events[DT_INST_PROP_LEN(n, events)];                                 \
    };                                                                             \
    static void kscan_mock_schedule_next_event_##n(struct device *dev)             \
    {                                                                              \
        struct kscan_mock_data *data = dev->driver_data;                           \
        const struct kscan_mock_config_##n *cfg = dev->config_info;                \
        if (data->event_index < DT_INST_PROP_LEN(n, events))                       \
        {                                                                          \
            u32_t ev = cfg->events[data->event_index];                             \
            LOG_DBG("delaying next keypress: %d", ZMK_MOCK_MSEC(ev));              \
            k_delayed_work_submit(&data->work, K_MSEC(ZMK_MOCK_MSEC(ev)));         \
        }                                                                          \
    }                                                                              \
    static void kscan_mock_work_handler_##n(struct k_work *work)                   \
    {                                                                              \
        struct kscan_mock_data *data =                                             \
            CONTAINER_OF(work, struct kscan_mock_data, work);                      \
        const struct kscan_mock_config_##n *cfg = data->dev->config_info;          \
        u32_t ev = cfg->events[data->event_index++];                               \
        LOG_DBG("ev %u row %d column %d state %d\n", ev,                           \
                ZMK_MOCK_ROW(ev), ZMK_MOCK_COL(ev), ZMK_MOCK_IS_PRESS(ev));        \
        data->callback(data->dev,                                                  \
                       ZMK_MOCK_ROW(ev), ZMK_MOCK_COL(ev), ZMK_MOCK_IS_PRESS(ev)); \
        kscan_mock_schedule_next_event_##n(data->dev);                             \
    }                                                                              \
    static int kscan_mock_init_##n(struct device *dev)                             \
    {                                                                              \
        struct kscan_mock_data *data = dev->driver_data;                           \
        data->dev = dev;                                                           \
        k_delayed_work_init(&data->work, kscan_mock_work_handler_##n);             \
        return 0;                                                                  \
    }                                                                              \
    static int kscan_mock_enable_callback_##n(struct device *dev)                  \
    {                                                                              \
        kscan_mock_schedule_next_event_##n(dev);                                   \
        return 0;                                                                  \
    }                                                                              \
    static const struct kscan_driver_api mock_driver_api_##n = {                   \
        .config = kscan_mock_configure,                                            \
        .enable_callback = kscan_mock_enable_callback_##n,                         \
        .disable_callback = kscan_mock_disable_callback,                           \
    };                                                                             \
    static struct kscan_mock_data kscan_mock_data_##n;                             \
    static const struct kscan_mock_config_##n kscan_mock_config_##n = {            \
        .events = DT_INST_PROP(n, events)};                                        \
    DEVICE_AND_API_INIT(kscan_mock_##n, DT_INST_LABEL(n), kscan_mock_init_##n,     \
                        &kscan_mock_data_##n,                                      \
                        &kscan_mock_config_##n,                                    \
                        APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,          \
                        &mock_driver_api_##n);

DT_INST_FOREACH_STATUS_OKAY(MOCK_INST_INIT)
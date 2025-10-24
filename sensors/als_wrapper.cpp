/*
 * SPDX-FileCopyrightText: 2019 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "sensors.als_wrapper"

#include <fcntl.h>
#include <hardware/sensors.h>
#include <log/log.h>

#include "AlsCorrection.h"

static const int SENSOR_TYPE_ANDROID_WISE_LIGHT = 65627;

struct als_wrapper_context_t {
    sensors_poll_device_1_t device;

    struct sensors_module_t* real_module;
    sensors_poll_device_1_t* real_device;
};

static struct als_wrapper_context_t* ctx;

static int als_wrapper_activate(struct sensors_poll_device_t* /* dev */, int handle, int enabled) {
    if (!ctx->real_device->activate) {
        return -EINVAL;
    }

    return ctx->real_device->activate((struct sensors_poll_device_t*)ctx->real_device, handle,
                                      enabled);
}

static int als_wrapper_setDelay(struct sensors_poll_device_t* /* dev */, int handle,
                                int64_t sampling_period_ns) {
    if (!ctx->real_device->setDelay) {
        return -EINVAL;
    }

    return ctx->real_device->setDelay((struct sensors_poll_device_t*)ctx->real_device, handle,
                                      sampling_period_ns);
}

static int als_wrapper_poll(struct sensors_poll_device_t* /* dev */, sensors_event_t* data,
                            int count) {
    int rc;

    if (!ctx->real_device->poll) {
        return 0;
    }

    rc = ctx->real_device->poll((struct sensors_poll_device_t*)ctx->real_device, data, count);
    if (rc < 0) {
        return 0;
    }

    for (int i = 0; i < rc; i++) {
        sensors_event_t* event = &data[i];
        if (event->type == SENSOR_TYPE_ANDROID_WISE_LIGHT) {
            AlsCorrection::process(*event);
        }
    }

    return rc;
}

static int als_wrapper_batch(struct sensors_poll_device_1* /* dev */, int handle, int flags,
                             int64_t period_ns, int64_t max_ns) {
    if (!ctx->real_device->batch) {
        return -EINVAL;
    }

    return ctx->real_device->batch(ctx->real_device, handle, flags, period_ns, max_ns);
}

static int als_wrapper_flush(struct sensors_poll_device_1* /* dev */, int handle) {
    if (!ctx->real_device->flush) {
        return -EINVAL;
    }

    return ctx->real_device->flush(ctx->real_device, handle);
}

static int als_wrapper_inject_sensor_data(struct sensors_poll_device_1* /* dev */,
                                          const sensors_event_t* data) {
    if (!ctx->real_device->inject_sensor_data) {
        return -EINVAL;
    }

    return ctx->real_device->inject_sensor_data(ctx->real_device, data);
}

static int als_wrapper_register_direct_channel(struct sensors_poll_device_1* /* dev */,
                                               const struct sensors_direct_mem_t* mem,
                                               int channel_handle) {
    if (!ctx->real_device->register_direct_channel) {
        return -EINVAL;
    }

    return ctx->real_device->register_direct_channel(ctx->real_device, mem, channel_handle);
}

static int als_wrapper_config_direct_report(struct sensors_poll_device_1* /* dev */,
                                            int sensor_handle, int channel_handle,
                                            const struct sensors_direct_cfg_t* config) {
    if (!ctx->real_device->config_direct_report) {
        return -EINVAL;
    }

    return ctx->real_device->config_direct_report(ctx->real_device, sensor_handle, channel_handle,
                                                  config);
}

static int open_sensors(const struct hw_module_t* module, const char* name,
                        struct hw_device_t** device) {
    int rc;

    ctx = (struct als_wrapper_context_t*)calloc(1, sizeof(als_wrapper_context_t));
    if (!ctx) {
        ALOGE("Failed to allocate memory!");
        return -ENOMEM;
    }

    rc = hw_get_module(SENSORS_HARDWARE_MODULE_ID, (const hw_module_t**)&ctx->real_module);
    if (rc < 0) {
        ALOGE("Could not get sensors hw module: %d", rc);
        return rc;
    }

    rc = ctx->real_module->common.methods->open(&ctx->real_module->common, name,
                                                (struct hw_device_t**)&ctx->real_device);
    if (rc < 0) {
        ALOGE("Failed to open sensors HAL: %d", rc);
        return rc;
    }

    ctx->device.common.tag = ctx->real_device->common.tag;
    ctx->device.common.version = ctx->real_device->common.version;
    ctx->device.common.module = const_cast<hw_module_t*>(module);
    ctx->device.common.close = ctx->real_device->common.close;

    ctx->device.activate = als_wrapper_activate;
    ctx->device.setDelay = als_wrapper_setDelay;
    ctx->device.poll = als_wrapper_poll;
    ctx->device.batch = als_wrapper_batch;
    ctx->device.flush = als_wrapper_flush;
    ctx->device.inject_sensor_data = als_wrapper_inject_sensor_data;
    ctx->device.register_direct_channel = als_wrapper_register_direct_channel;
    ctx->device.config_direct_report = als_wrapper_config_direct_report;

    *device = &ctx->device.common;
    return 0;
}

static struct hw_module_methods_t als_wrapper_module_methods = {
        .open = open_sensors,
};

static int als_wrapper_get_sensors_list(struct sensors_module_t* /* module */,
                                        struct sensor_t const** list) {
    int rc;
    const struct sensor_t* sensors;

    if (!ctx || !ctx->real_module) {
        return -ENODEV;
    }

    rc = ctx->real_module->get_sensors_list(ctx->real_module, &sensors);
    if (rc < 0) {
        ALOGE("Failed to get sensors list: %d", rc);
        return rc;
    }

    for (int i = 0; i < rc; i++) {
        struct sensor_t* sensor = (struct sensor_t*)&sensors[i];
        if (sensor->type == SENSOR_TYPE_ANDROID_WISE_LIGHT) {
            ALOGD("Patching Oplus Wise Light sensor.");
            sensor->type = SENSOR_TYPE_LIGHT;
            AlsCorrection::init();
            break;
        }
    }

    *list = sensors;
    return rc;
}

static int als_wrapper_set_operation_mode(unsigned int mode) {
    if (!ctx || !ctx->real_module) {
        return -ENODEV;
    }

    return ctx->real_module->set_operation_mode(mode);
}

struct sensors_module_t HAL_MODULE_INFO_SYM = {
        .common = {.tag = HARDWARE_MODULE_TAG,
                   .version_major = 1,
                   .version_minor = 0,
                   .id = SENSORS_HARDWARE_MODULE_ID,
                   .name = "Oplus ALS Sensors Wrapper Module",
                   .author = "bengris32",
                   .methods = &als_wrapper_module_methods,
                   .dso = NULL,
                   .reserved = {0}},
        .get_sensors_list = als_wrapper_get_sensors_list,
        .set_operation_mode = als_wrapper_set_operation_mode,
};

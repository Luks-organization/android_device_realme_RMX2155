/*
 * SPDX-FileCopyrightText: 2019 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define PRIVATE_SENSOR_TYPE(type) (SENSOR_TYPE_DEVICE_PRIVATE_BASE + type)

#define VIRTUAL_SENSORS_DEVICE "/dev/m_virtual_sensor_misc"
#define VIRTUAL_SENSORS_SYSFS "/sys/class/oplus_sensor/m_virtual_sensor_misc/virtual_sensor"

#define VIRTUAL_SENSORS_ACTIVE VIRTUAL_SENSORS_SYSFS "active"
#define VIRTUAL_SENSORS_BATCH VIRTUAL_SENSORS_SYSFS "batch"
#define VIRTUAL_SENSORS_DEVNUM VIRTUAL_SENSORS_SYSFS "devnum"
#define VIRTUAL_SENSORS_FLUSH VIRTUAL_SENSORS_SYSFS "flush"

enum {
    SENSOR_PICKUP_DETECT,
};

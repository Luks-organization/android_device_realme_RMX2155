/*
 * SPDX-FileCopyrightText: 2019 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "init_RMX2155.h"

#include <vector>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "vendor_init.h"
#include "property_service.h"

#include <fs_mgr_dm_linear.h>

using android::base::ReadFileToString;
using android::base::Trim;
using std::string;

std::vector<string> ro_props_default_source_order = {
    "",
    "bootimage.",
    "odm.",
    "odm_dlkm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
    "vendor_dlkm.",
};

void property_override(const char* prop, const char* value, bool add) {
    prop_info* pi = (prop_info*)__system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else if (add)
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void set_ro_build_prop(const string& prop, const string& value, bool product) {
    string prop_name;

    for (const auto& source : ro_props_default_source_order) {
        if (product)
            prop_name = "ro.product." + source + prop;
        else
            prop_name = "ro." + source + "build." + prop;

        property_override(prop_name.c_str(), value.c_str());
    }
}

void load_dalvik_properties(void) {
    char const *heapstartsize;
    char const *heapgrowthlimit;
    char const *heapsize;
    char const *heapminfree;
    char const *heapmaxfree;
    char const *heaptargetutilization;
    struct sysinfo sys;

    sysinfo(&sys);

    if (sys.totalram >= 7ull * 1024 * 1024 * 1024) {
        heapstartsize = "24m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.46";
        heapminfree = "8m";
        heapmaxfree = "48m";
    } else if (sys.totalram >= 5ull * 1024 * 1024 * 1024) {
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "32m";
    } else {
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "8m";
        heapmaxfree = "16m";
    }

    property_override("dalvik.vm.heapstartsize", heapstartsize);
    property_override("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    property_override("dalvik.vm.heapsize", heapsize);
    property_override("dalvik.vm.heaptargetutilization", heaptargetutilization);
    property_override("dalvik.vm.heapminfree", heapminfree);
    property_override("dalvik.vm.heapmaxfree", heapmaxfree);
}

void set_device_props(void) {
    char const *operator_code_file = "/proc/oplusVersion/operatorName";
    std::string operator_code_raw, device, model, marketname, fingerprint;

    if (ReadFileToString(operator_code_file, &operator_code_raw)) {
        int operator_code = stoi(operator_code_raw);
        switch (operator_code) {
            case 140:
            case 141:
            case 146:
            case 149:
                device = "RMX2151L1";
                model = "RMX2151";
                marketname = "realme 7";
                fingerprint = "realme/RMX2151/RMX2151L1:12/SP1A.210812.016/Q.bf75e7-1:user/release-keys";
                break;
            case 94:
            case 148:
                device = "RMX2155L1";
                model = "RMX2155";
                marketname = "realme 7";
                fingerprint = "realme/RMX2155EEA/RMX2155L1:12/SP1A.210812.016/Q.GDPR.bf75e7-1:user/release-keys";
                break;
            default:
                LOG(ERROR) << "Unknown operator found: " << operator_code;
                device = "";
                model = "";
                marketname = "";
                fingerprint = "";
        }
    }

    if (!fingerprint.empty()) {
        property_override("ro.build.fingerprint", fingerprint.c_str());
        for (const auto& source : ro_props_default_source_order) {
            string prop = "ro." + source + "build.fingerprint";
            property_override(prop.c_str(), fingerprint.c_str());
        }
    }

    set_ro_build_prop("device", device);
    set_ro_build_prop("model", model);
    set_ro_build_prop("name", model);
    set_ro_build_prop("product", model, false);
    set_ro_build_prop("marketname", marketname);

    property_override("ro.vendor.device", device.c_str());
    property_override("ro.product.device", device.c_str());
    property_override("bluetooth.device.default_name", marketname.c_str());
    property_override("vendor.usb.product_string", marketname.c_str());
    property_override("ro.oplus.market.name", marketname.c_str());
    property_override("ro.vendor.oplus.market.name", marketname.c_str());
}

void vendor_load_properties(void) {
#ifndef __ANDROID_RECOVERY__
    set_device_props();
#endif
    load_dalvik_properties();
}

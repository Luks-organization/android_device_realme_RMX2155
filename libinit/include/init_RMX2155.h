/*
 * SPDX-FileCopyrightText: 2019 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifndef INIT_RMX2155_H
#define INIT_RMX2155_H

#include <string>
#include <vector>

extern std::vector<std::string> ro_props_default_source_order;

void property_override(const char* prop, const char* value, bool add = true);
void set_ro_build_prop(const std::string& prop, const std::string& value, bool product = true);
void load_dalvik_properties(void);
void set_device_props(void);
void vendor_load_properties(void);

#endif // INIT_RMX2155_H

/*
 * SPDX-FileCopyrightText: The Android Open Source Project
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package com.realme.realmeparts;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import androidx.preference.PreferenceManager;

public class Startup extends BroadcastReceiver {
    private static final String TAG = "BootReceiver";
    private static final String ONE_TIME_TUNABLE_RESTORE = "hardware_tunable_restored";
    
    // Predefined switch configurations
    private static final SwitchConfig[] SWITCH_CONFIGS = {
        new SwitchConfig(DCModeSwitch.getFile(), DeviceSettings.KEY_DC_SWITCH),
        new SwitchConfig(SRGBModeSwitch.getFile(), DeviceSettings.KEY_SRGB_SWITCH),
        new SwitchConfig(OTGModeSwitch.getFile(), DeviceSettings.KEY_OTG_SWITCH)
    };

    @Override
    public void onReceive(Context context, Intent bootIntent) {
        final SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        
        restoreSwitches(context, prefs);
    }

    private void restoreSwitches(Context context, SharedPreferences prefs) {
        for (SwitchConfig config : SWITCH_CONFIGS) {
            if (config.filePath != null) {
                Utils.writeValue(config.filePath, 
                    prefs.getBoolean(config.prefKey, false) ? "1" : "0");
            }
        }
    }

    private boolean hasRestoredTunable(Context context) {
        return PreferenceManager.getDefaultSharedPreferences(context)
            .getBoolean(ONE_TIME_TUNABLE_RESTORE, false);
    }

    private void setRestoredTunable(Context context) {
        PreferenceManager.getDefaultSharedPreferences(context)
            .edit()
            .putBoolean(ONE_TIME_TUNABLE_RESTORE, true)
            .apply();
    }

    // Immutable configuration class
    private static final class SwitchConfig {
        final String filePath;
        final String prefKey;

        SwitchConfig(String filePath, String prefKey) {
            this.filePath = filePath;
            this.prefKey = prefKey;
        }
    }
}

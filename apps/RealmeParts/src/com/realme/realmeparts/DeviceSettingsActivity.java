/*
 * SPDX-FileCopyrightText: The Android Open Source Project
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package com.realme.realmeparts;

import android.app.Fragment;
import android.os.Bundle;
import com.android.settingslib.collapsingtoolbar.CollapsingToolbarBaseActivity;

public class DeviceSettingsActivity extends CollapsingToolbarBaseActivity {

    private static final int CONTENT_FRAME_ID = com.android.settingslib.collapsingtoolbar.R.id.content_frame;
    private DeviceSettings mDeviceSettingsFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if (savedInstanceState == null) {
            initializeFragment();
        } else {
            restoreFragment();
        }
    }

    private void initializeFragment() {
        mDeviceSettingsFragment = new DeviceSettings();
        getFragmentManager().beginTransaction()
                .replace(CONTENT_FRAME_ID, mDeviceSettingsFragment)
                .commitNow();
    }

    private void restoreFragment() {
        Fragment fragment = getFragmentManager().findFragmentById(CONTENT_FRAME_ID);
        if (fragment instanceof DeviceSettings) {
            mDeviceSettingsFragment = (DeviceSettings) fragment;
        } else {
            initializeFragment();
        }
    }
}

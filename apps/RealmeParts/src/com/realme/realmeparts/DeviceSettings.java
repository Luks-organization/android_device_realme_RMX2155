/*
 * SPDX-FileCopyrightText: The Android Open Source Project
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package com.realme.realmeparts;

import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.hardware.display.DisplayManager;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.Display;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceFragment;
import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreference;
import androidx.preference.TwoStatePreference;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import java.io.IOException;
import java.text.DecimalFormat;

import com.realme.realmeparts.speaker.ClearSpeakerActivity;

public class DeviceSettings extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    // Preference Keys
    public static final String KEY_SRGB_SWITCH = "srgb";
    public static final String KEY_DC_SWITCH = "dc";
    public static final String KEY_OTG_SWITCH = "otg";
    public static final String KEY_CABC = "cabc";
    public static final String KEY_SETTINGS_PREFIX = "device_setting_";

    // System Properties
    public static final String CABC_SYSTEM_PROPERTY = "persist.cabc_profile";

    // Key categories
    private static final String KEY_CATEGORY_GRAPHICS = "graphics";
    private static final String PREF_CLEAR_SPEAKER = "clear_speaker_settings";
    private static final String KEY_CATEGORY_MTK_ENG = "mtk_engineer";
    private final String ProductName = Utils.ProductName();  // Get product name

    // Preference components
    private PreferenceCategory mPreferenceCategory;
    public static DisplayManager mDisplayManager;
    private static NotificationManager mNotificationManager;
    private TwoStatePreference mDCModeSwitch;
    private TwoStatePreference mSRGBModeSwitch;
    private TwoStatePreference mOTGModeSwitch;
    private SecureSettingListPreference mCABC;
    private Preference mClearSpeakerPref;
    private Preference mEngineerMode;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        // Get context and preferences
        Context context = getContext();
        if (context == null) {
            return;  // Exit early if context is null
        }

        final SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        prefs.edit().putString("ProductName", ProductName).apply();

        // Load preferences from XML
        addPreferencesFromResource(R.xml.main);

        // Initialize DC Mode Switch
        mDCModeSwitch = findPreference(KEY_DC_SWITCH);
        if (mDCModeSwitch != null) {
            mDCModeSwitch.setEnabled(DCModeSwitch.isSupported());
            mDCModeSwitch.setChecked(DCModeSwitch.isCurrentlyEnabled(context));
            mDCModeSwitch.setOnPreferenceChangeListener(new DCModeSwitch());
        }

        // Initialize SRGB Mode Switch
        mSRGBModeSwitch = findPreference(KEY_SRGB_SWITCH);
        if (mSRGBModeSwitch != null) {
            mSRGBModeSwitch.setEnabled(SRGBModeSwitch.isSupported());
            mSRGBModeSwitch.setChecked(SRGBModeSwitch.isCurrentlyEnabled(context));
            mSRGBModeSwitch.setOnPreferenceChangeListener(new SRGBModeSwitch());
        }

        // Initialize OTG Mode Switch
        mOTGModeSwitch = findPreference(KEY_OTG_SWITCH);
        if (mOTGModeSwitch != null) {
            mOTGModeSwitch.setEnabled(OTGModeSwitch.isSupported());
            mOTGModeSwitch.setChecked(OTGModeSwitch.isCurrentlyEnabled(context));
            mOTGModeSwitch.setOnPreferenceChangeListener(new OTGModeSwitch());
        }

        // CABC (Content Adaptive Backlight Control) Preference
        mCABC = findPreference(KEY_CABC);
        if (mCABC != null) {
            try {
                ParseJson(); // Check if device supports CABC
                if (prefs.getBoolean("CABC_DeviceMatched", false)) {
                    mCABC.setValue(Utils.getStringProp(CABC_SYSTEM_PROPERTY, "0"));
                    mCABC.setSummary(mCABC.getEntry());
                    mCABC.setOnPreferenceChangeListener(this);
                } else {
                    removePreference(KEY_CABC);
                }
            } catch (JSONException e) {
                Log.e("DeviceSettings", "Error parsing JSON", e);
                removePreference(KEY_CABC);
            }
        }

        // Clear Speaker Preference
        mClearSpeakerPref = findPreference(PREF_CLEAR_SPEAKER);
        if (mClearSpeakerPref != null) {
            mClearSpeakerPref.setOnPreferenceClickListener(preference -> {
                Intent intent = new Intent(getActivity(), ClearSpeakerActivity.class);
                startActivity(intent);
                return true;
            });
        }

        // Engineer Mode Preference (only available if Developer Options are enabled)
        mEngineerMode = findPreference(KEY_CATEGORY_MTK_ENG);
        if (mEngineerMode != null) {
            boolean isDevOptionsEnabled = Settings.Global.getInt(context.getContentResolver(), 
                Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0) != 0;
            if (!isDevOptionsEnabled) {
                removePreference(KEY_CATEGORY_MTK_ENG);
            }
        }
    }

    private void removePreference(String key) {
        Preference pref = findPreference(key);
        if (pref != null) {
            getPreferenceScreen().removePreference(pref);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference == mCABC) {
            String newCABCValue = (String) newValue;
            mCABC.setValue(newCABCValue);
            mCABC.setSummary(mCABC.getEntry());
            Utils.setStringProp(CABC_SYSTEM_PROPERTY, newCABCValue);
            return true;
        }
        return false;
    }

    private void ParseJson() throws JSONException {
        final SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
        mPreferenceCategory = findPreference(KEY_CATEGORY_GRAPHICS);

        // Read the JSON file as string
        String features_json = Utils.InputStreamToString(getResources().openRawResource(R.raw.realmeparts_features));
        if (features_json == null || features_json.isEmpty()) {
            throw new JSONException("Failed to load JSON data");
        }

        // Parse JSON string into JSONObject
        JSONObject jsonOB = new JSONObject(features_json);

        // Get the CABC array from the JSON object
        JSONArray CABC = jsonOB.optJSONArray(KEY_CABC);
        if (CABC == null) {
            throw new JSONException("CABC array not found in JSON");
        }

        // Check if ProductName contains any entry in the CABC array
        boolean CABC_DeviceMatched = false;
        String productNameUpper = ProductName.toUpperCase();
        for (int i = 0; i < CABC.length(); i++) {
            if (productNameUpper.contains(CABC.getString(i).toUpperCase())) {
                CABC_DeviceMatched = true;
                break;
            }
        }

        prefs.edit().putBoolean("CABC_DeviceMatched", CABC_DeviceMatched).apply();
    }
}

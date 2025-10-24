#
# SPDX-FileCopyrightText: 2019 The Android Open Source Project
# SPDX-FileCopyrightText: 2025 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Unzip bloobs
unzip vendor/realme/RMX2155/radio/md1img.zip -d vendor/realme/RMX2155/radio && rm vendor/realme/RMX2155/radio/md1img.zip
unzip vendor/realme/RMX2155/proprietary/odm/lib64/libstfaceunlockppl.zip -d vendor/realme/RMX2155/proprietary/odm/lib64 && rm vendor/realme/RMX2155/proprietary/odm/lib64/libstfaceunlockppl.zip

#echo "- Applying fenrir compatiblity patches"
#cd system/core
#curl https://github.com/Nothing-2A/android_system_core/commit/8ff6e7a68523c3b870d8dcd5713c71ea15b43dd2.patch | git am
#curl https://github.com/Nothing-2A/android_system_core/commit/0d5990a96c5e6a404887f5575c5d00bcbbaaef74.patch | git am
#cd ../../

echo "- Applying Aperture Mediatek HFPS Mode Patch"
cd packages/apps/Aperture
curl https://github.com/Nothing-2A/android_packages_apps_Aperture/commit/9509277efc852ad8bdcce204e0d9cfe104b6d190.patch | git am
cd ../../../

# Apply patch
cd device/realme/RMX2155/patches && ./apply.sh && cd && cd evo

# Make the build faster using ccache
#export CCACHE_EXEC=/usr/bin/ccache

export USE_CCACHE=1
export CCACHE_DIR=~/.ccache
export CCACHE_COMPRESS=1
export CCACHE_COMPRESSLEVEL=6

ccache -M 40G
ccache -o compression=true
ccache -o hard_link=true
ccache -o max_files=200000
ccache -o depend_mode=true

# Disable and stop systemd-oomd service.
systemctl disable --now systemd-oomd && sudo apt-get purge systemd-oomd -y

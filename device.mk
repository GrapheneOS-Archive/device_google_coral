#
# Copyright (C) 2018 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := device/google/coral

PRODUCT_VENDOR_MOVE_ENABLED := true

PRODUCT_SOONG_NAMESPACES += \
    hardware/google/av \
    hardware/google/camera \
    hardware/google/interfaces \
    hardware/google/pixel \
    device/google/coral \
    hardware/qcom/sm8150/display \
    hardware/qcom/sm8150/data/ipacfg-mgr \
    hardware/qcom/sm8150/gps \
    vendor/google/airbrush/floral \
    vendor/google/biometrics/face/florence \
    vendor/google/camera \
    vendor/google/darwinn \
    vendor/google_devices/common/proprietary/confirmatioui_hal \
    vendor/google_nos/host/android \
    vendor/qcom/sm8150 \
    vendor/qcom/sm8150/proprietary/commonsys/telephony-apps/DataStatusNotification \
    vendor/qcom/sm8150/proprietary/gps \
    vendor/qcom/sm8150/proprietary/qmi \
    vendor/qcom/sm8150/codeaurora/location \
    vendor/google/interfaces \
    vendor/google_nos/test/system-test-harness

# Include sensors soong namespace
PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/sensors \
    vendor/google/tools/sensors

# Single vendor RIL/Telephony/data with SM7250
DEVICE_USES_SM7250_QCRIL_TELEPHONY := true

ifeq ($(DEVICE_USES_SM7250_QCRIL_TELEPHONY), true)
  PRODUCT_SOONG_NAMESPACES += \
      vendor/qcom/sm7250/codeaurora/commonsys/telephony/ims/ims-ext-common \
      vendor/qcom/sm7250/codeaurora/dataservices/rmnetctl \
      vendor/qcom/sm7250/proprietary/commonsys/qcrilOemHook \
      vendor/qcom/sm7250/proprietary/commonsys/telephony-apps/ims \
      vendor/qcom/sm7250/proprietary/commonsys/telephony-apps/QtiTelephonyService \
      vendor/qcom/sm7250/proprietary/commonsys/telephony-apps/xdivert \
      vendor/qcom/sm7250/proprietary/qcril-data-hal \
      vendor/qcom/sm7250/proprietary/qcril-hal \
      vendor/qcom/sm7250/proprietary/data
else
  $(warning DEVICE_USES_SM7250_QCRIL_TELEPHONY is disabled)

  PRODUCT_SOONG_NAMESPACES += \
      vendor/qcom/sm8150/codeaurora/telephony/ims \
      vendor/qcom/sm8150/proprietary/qcril-data-hal \
      vendor/qcom/sm8150/proprietary/qcril-hal
endif

PRODUCT_PROPERTY_OVERRIDES += \
    keyguard.no_require_sim=true

# enable cal by default on accel sensor
PRODUCT_PRODUCT_PROPERTIES += \
    persist.vendor.debug.sensors.accel_cal=1

# The default value of this variable is false and should only be set to true when
# the device allows users to retain eSIM profiles after factory reset of user data.
PRODUCT_PRODUCT_PROPERTIES += \
    masterclear.allow_retain_esim_profiles_after_fdr=true

PRODUCT_COPY_FILES += \
    device/google/coral/default-permissions.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/default-permissions/default-permissions.xml \
    device/google/coral/component-overrides.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sysconfig/component-overrides.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.software.verified_boot.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.verified_boot.xml

# Enforce privapp-permissions whitelist
PRODUCT_PROPERTY_OVERRIDES += \
    ro.control_privapp_permissions=enforce

PRODUCT_PACKAGES += \
    messaging

TARGET_PRODUCT_PROP := $(LOCAL_PATH)/product.prop

$(call inherit-product, $(LOCAL_PATH)/utils.mk)

# Installs gsi keys into ramdisk, to boot a developer GSI with verified boot.
$(call inherit-product, $(SRC_TARGET_DIR)/product/developer_gsi_keys.mk)

ifeq ($(wildcard vendor/google_devices/coral/proprietary/device-vendor-coral.mk),)
    BUILD_WITHOUT_VENDOR := true
endif

ifeq ($(TARGET_PREBUILT_KERNEL),)
    LOCAL_KERNEL := device/google/coral-kernel/Image.lz4
else
    LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif
PRODUCT_VENDOR_KERNEL_HEADERS := device/google/coral-kernel/sm8150/kernel-headers


PRODUCT_CHARACTERISTICS := nosdcard
PRODUCT_SHIPPING_API_LEVEL := 29

# Enforce native interfaces of product partition as VNDK
PRODUCT_PRODUCT_VNDK_VERSION := current

DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay

#
PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel \
    $(LOCAL_PATH)/fstab.hardware:$(TARGET_COPY_OUT_RECOVERY)/root/first_stage_ramdisk/fstab.$(PRODUCT_PLATFORM) \
    $(LOCAL_PATH)/fstab.hardware:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.$(PRODUCT_PLATFORM) \
    $(LOCAL_PATH)/fstab.persist:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.persist \
    $(LOCAL_PATH)/init.hardware.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).rc \
    $(LOCAL_PATH)/init.power.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).power.rc \
    $(LOCAL_PATH)/init.radio.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.radio.sh \
    $(LOCAL_PATH)/init.hardware.usb.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).usb.rc \
    $(LOCAL_PATH)/init.insmod.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.insmod.sh \
    $(LOCAL_PATH)/init.sensors.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.sensors.sh \
    $(LOCAL_PATH)/thermal-engine-$(PRODUCT_HARDWARE)-normal_mode.conf:$(TARGET_COPY_OUT_VENDOR)/etc/thermal-engine-$(PRODUCT_HARDWARE)-normal_mode.conf \
    $(LOCAL_PATH)/thermal-engine-$(PRODUCT_HARDWARE)-retail_mode.conf:$(TARGET_COPY_OUT_VENDOR)/etc/thermal-engine-$(PRODUCT_HARDWARE)-retail_mode.conf \
    $(LOCAL_PATH)/ueventd.rc:$(TARGET_COPY_OUT_VENDOR)/ueventd.rc \
    $(LOCAL_PATH)/init.ramoops.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.ramoops.sh


MSM_VIDC_TARGET_LIST := msmnile # Get the color format from kernel headers
MASTER_SIDE_CP_TARGET_LIST := msmnile # ION specific settings

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.mpssrfs.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).mpssrfs.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.diag.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).diag.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.wlc.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.$(PRODUCT_PLATFORM).wlc.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.userdebug.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.$(PRODUCT_PLATFORM).userdebug.rc
else
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.mpssrfs.rc.user:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).mpssrfs.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.diag.rc.user:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).diag.rc
endif

# Enable CAT by default
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.cat.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.cat.rc
endif

# A/B support
PRODUCT_PACKAGES += \
    otapreopt_script \
    cppreopts.sh \
    update_engine \
    update_verifier

# Use Sdcardfs
PRODUCT_PRODUCT_PROPERTIES += \
    ro.sys.sdcardfs=1

PRODUCT_PACKAGES += \
    bootctrl.msmnile \
    bootctrl.msmnile.recovery

PRODUCT_PROPERTY_OVERRIDES += \
    ro.cp_system_other_odex=1

AB_OTA_POSTINSTALL_CONFIG += \
    RUN_POSTINSTALL_system=true \
    POSTINSTALL_PATH_system=system/bin/otapreopt_script \
    FILESYSTEM_TYPE_system=ext4 \
    POSTINSTALL_OPTIONAL_system=true

AB_OTA_POSTINSTALL_CONFIG += \
    RUN_POSTINSTALL_vendor=true \
    POSTINSTALL_PATH_vendor=bin/checkpoint_gc \
    FILESYSTEM_TYPE_vendor=ext4 \
    POSTINSTALL_OPTIONAL_vendor=true

PRODUCT_PACKAGES += \
    update_engine_sideload

PRODUCT_PACKAGES_DEBUG += \
    sg_write_buffer \
    f2fs_io \
    check_f2fs

PRODUCT_PROPERTY_OVERRIDES += \
    ro.crypto.volume.filenames_mode=aes-256-cts

# Userdata Checkpointing OTA GC
PRODUCT_PACKAGES += \
    checkpoint_gc

# The following modules are included in debuggable builds only.
PRODUCT_PACKAGES_DEBUG += \
    bootctl \
    r.vendor \
    update_engine_client

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml\
    frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml\
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.biometrics.face.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.biometrics.face.xml\
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.assist.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.assist.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.barometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.barometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.sensor.hifi_sensors.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.hifi_sensors.xml \
    frameworks/native/data/etc/android.hardware.context_hub.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.context_hub.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.telephony.ims.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.ims.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.aware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.aware.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.wifi.rtt.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.rtt.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/android.hardware.reboot_escrow.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.reboot_escrow.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.nxp.mifare.xml \
    frameworks/native/data/etc/android.hardware.vulkan.level-1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level.xml \
    frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version.xml \
    frameworks/native/data/etc/android.software.vulkan.deqp.level-2020-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.vulkan.deqp.level.xml \
    frameworks/native/data/etc/android.software.opengles.deqp.level-2020-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.opengles.deqp.level.xml \
    frameworks/native/data/etc/android.hardware.telephony.carrierlock.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.carrierlock.xml \
    frameworks/native/data/etc/android.hardware.strongbox_keystore.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.strongbox_keystore.xml \
    frameworks/native/data/etc/android.hardware.nfc.uicc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.uicc.xml \
    frameworks/native/data/etc/android.hardware.nfc.ese.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.ese.xml \
    frameworks/native/data/etc/android.hardware.se.omapi.ese.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.se.omapi.ese.xml \
    frameworks/native/data/etc/android.hardware.se.omapi.uicc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.se.omapi.uicc.xml \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml \

# Audio fluence, ns, aec property, voice and media volume steps
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.audio.sdk.fluencetype=fluencepro \
    persist.vendor.audio.fluence.voicecall=true \
    persist.vendor.audio.fluence.speaker=true \
    persist.vendor.audio.fluence.voicecomm=true \
    persist.vendor.audio.fluence.voicerec=false \
    ro.config.vc_call_vol_steps=7 \
    ro.config.media_vol_steps=25 \

# Audio Features
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.audio.feature.external_dsp.enable=true \
    vendor.audio.feature.external_speaker.enable=true \
    vendor.audio.feature.concurrent_capture.enable=false \
    vendor.audio.feature.a2dp_offload.enable=true \
    vendor.audio.feature.hfp.enable=true \
    vendor.audio.feature.hwdep_cal.enable=true \
    vendor.audio.feature.incall_music.enable=true \
    vendor.audio.feature.maxx_audio.enable=true \
    vendor.audio.feature.spkr_prot.enable=true \
    vendor.audio.feature.usb_offload.enable=true \
    vendor.audio.feature.audiozoom.enable=true \
    vendor.audio.feature.snd_mon.enable=true \
    vendor.audio.feature.multi_voice_session.enable=true \
    vendor.audio.capture.enforce_legacy_copp_sr=true \
    persist.vendor.audio_hal.dsp_bit_width_enforce_mode=24 \
    vendor.audio.offload.gapless.enabled=true \

# MaxxAudio effect and add rotation monitor
PRODUCT_PROPERTY_OVERRIDES += \
    ro.audio.monitorRotation=true

# Iaxxx streming and factory binary
PRODUCT_PACKAGES += \
    libtunnel \
    libodsp \
    adnc_strm.primary.default
# Add Oslo test for debug rom
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PACKAGES += \
    tunneling_hal_test \
    sensor_param_test \
    oslo_config_test \
    oslo_data_injection_test \
    oslo_get_stats \
    odsp_api_test \
    crash_event_logger \
    dump_debug_info \
    get_pwr_stats \
    crash_trigger_test
endif

# graphics
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=196610

PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.display.foss=1 \
    ro.vendor.display.paneltype=2 \
    ro.vendor.display.sensortype=2 \
    vendor.display.foss.config=1 \
    vendor.display.foss.config_path=/vendor/etc/FOSSConfig.xml \
    vendor.display.qdcm.mode_combine=1

PRODUCT_PROPERTY_OVERRIDES += \
    vendor.display.defer_fps_frame_count=2 \
    vendor.display.primary_vsyncs_rate_change=2

# camera google face detection
PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.googfd.enable=1

# camera hal buffer management
PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.managebuffer.enable=1

# Lets the vendor library that Google Camera HWL is enabled
PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.google_hwl.enabled=true \
    persist.camera.google_hwl.name=libgooglecamerahwl_impl.so

# OEM Unlock reporting
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1

PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.cne.feature=1 \
    persist.vendor.data.iwlan.enable=true \
    persist.vendor.radio.RATE_ADAPT_ENABLE=1 \
    persist.vendor.radio.ROTATION_ENABLE=1 \
    persist.vendor.radio.VT_ENABLE=1 \
    persist.vendor.radio.VT_HYBRID_ENABLE=1 \
    persist.vendor.radio.apm_sim_not_pwdn=1 \
    persist.vendor.radio.custom_ecc=1 \
    persist.vendor.radio.data_ltd_sys_ind=1 \
    persist.vendor.radio.videopause.mode=1 \
    persist.vendor.radio.mt_sms_ack=30 \
    persist.vendor.radio.multisim_switch_support=true \
    persist.vendor.radio.sib16_support=1 \
    persist.vendor.radio.data_con_rprt=true \
    persist.vendor.radio.relay_oprt_change=1 \
    persist.vendor.radio.no_wait_for_card=1 \
    persist.vendor.radio.sap_silent_pin=1 \
    persist.vendor.radio.manual_nw_rej_ct=1 \
    persist.rcs.supported=1 \
    vendor.rild.libpath=/vendor/lib64/libril-qc-hal-qmi.so \
    ro.hardware.keystore_desede=true \
    persist.vendor.radio.procedure_bytes=SKIP \

# Enable reboot free DSDS
PRODUCT_PRODUCT_PROPERTIES += \
    persist.radio.reboot_on_modem_change=false

PRODUCT_PROPERTY_OVERRIDES += \
    telephony.active_modems.max_count=2

# Disable snapshot timer
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.radio.snapshot_enabled=0 \
    persist.vendor.radio.snapshot_timer=0

PRODUCT_PACKAGES += \
    hwcomposer.msmnile \
    android.hardware.graphics.composer@2.4-service-sm8150 \
    gralloc.msmnile \
    android.hardware.graphics.mapper@3.0-impl-qti-display \
    android.hardware.graphics.mapper@4.0-impl-qti-display \
    vendor.qti.hardware.display.allocator-service

# RenderScript HAL
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# Light HAL
PRODUCT_PACKAGES += \
    lights.msmnile \
    hardware.google.light@1.1-service

# Memtrack HAL
PRODUCT_PACKAGES += \
    memtrack.msmnile \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service

# Bluetooth HAL
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0-impl-qti \
    android.hardware.bluetooth@1.0-service-qti

#Bluetooth SAR HAL
PRODUCT_PACKAGES += \
    hardware.google.bluetooth.sar@1.0-impl
PRODUCT_PACKAGES_DEBUG += \
    bluetooth_sar_test

# Bluetooth SoC
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.qcom.bluetooth.soc=cherokee

# Property for loading BDA from device tree
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.bt.bdaddr_path=/proc/device-tree/chosen/cdt/cdb2/bt_addr

# Bluetooth WiPower
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.bluetooth.emb_wp_mode=false \
    ro.vendor.bluetooth.wipower=false

# Bluetooth ftmdaemon needs libbt-hidlclient.so
PRODUCT_SOONG_NAMESPACES += vendor/qcom/proprietary/bluetooth/hidl_client

# DRM HAL
PRODUCT_PACKAGES += \
    android.hardware.drm-service.clearkey \
    android.hardware.drm-service.widevine

# NFC and Secure Element packages
PRODUCT_PACKAGES += \
    NfcNci \
    Tag \
    SecureElement \
    android.hardware.nfc@1.2-service.st \
    android.hardware.secure_element@1.0-service.st

PRODUCT_COPY_FILES += \
    device/google/coral/nfc/com.google.hardware.pixel.japan.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_G020N/com.google.hardware.pixel.japan.xml \
    device/google/coral/nfc/com.google.hardware.pixel.japan.xml:$(TARGET_COPY_OUT_ODM)/etc/permissions/sku_G020Q/com.google.hardware.pixel.japan.xml

PRODUCT_COPY_FILES += \
    device/google/coral/nfc/libnfc-hal-st.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-hal-st.conf \
    device/google/coral/nfc/libese-hal-st.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libese-hal-st.conf \
    device/google/coral/nfc/libnfc-nci.conf:$(TARGET_COPY_OUT_PRODUCT)/etc/libnfc-nci.conf

# USB HAL
PRODUCT_PACKAGES += \
    android.hardware.usb-service.coral
PRODUCT_PACKAGES += \
    android.hardware.usb.gadget-service.coral

PRODUCT_PACKAGES += \
    android.hardware.health@2.1-impl-coral \
    android.hardware.health@2.1-service

# Storage health HAL
PRODUCT_PACKAGES += \
    android.hardware.health.storage@1.0-service

PRODUCT_PACKAGES += \
    libmm-omxcore \
    libOmxCore \
    libstagefrighthw \
    libOmxVdec \
    libOmxVdecHevc \
    libOmxVenc \
    libc2dcolorconvert

# Enable Codec 2.0
PRODUCT_PROPERTY_OVERRIDES += \
    debug.media.codec2=2 \
    debug.stagefright.ccodec=4 \
    debug.stagefright.omx_default_rank=512 \

# Disable OMX
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.media.omx=0 \

# Create input surface on the framework side
PRODUCT_PROPERTY_OVERRIDES += \
    debug.stagefright.c2inputsurface=-1 \

# Transcoding related property.
PRODUCT_PROPERTY_OVERRIDES += \
    debug.media.transcoding.codec_max_operating_rate_720P=480 \
    debug.media.transcoding.codec_max_operating_rate_1080P=240 \
    debug.media.transcoding.codec_max_operating_rate_4k=120 \

# Enable ECO service
QC2_HAVE_ECO_SERVICE := true

PRODUCT_PACKAGES += \
    libqcodec2 \
    vendor.qti.media.c2@1.0-service \
    media_codecs_c2.xml \
    codec2.vendor.ext.policy \
    codec2.vendor.base.policy

PRODUCT_PROPERTY_OVERRIDES += \
    vendor.qc2.venc.avgqp.enable=1

# To reach target bitrate in CBR mode for IMS VT Call
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.ims.mm_minqp=1

PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.7-impl-google \
    android.hardware.camera.provider@2.7-service-google \
    camera.msmnile \
    lib_multicam_dualfov_capture_session \
    libgooglecamerahwl_impl \
    libqomx_core \
    libmmjpeg_interface \
    libmmcamera_interface \
    libcameradepthcalibrator

# Google Camera HAL test libraries in debug builds
PRODUCT_PACKAGES_DEBUG += \
    libgoogle_camera_hal_proprietary_tests \
    libgoogle_camera_hal_tests

PRODUCT_PACKAGES += \
    sensors.$(PRODUCT_HARDWARE) \
    android.hardware.sensors@2.0-service.multihal \
    hals.conf

PRODUCT_PACKAGES += \
    fs_config_dirs \
    fs_config_files

# Context hub HAL
PRODUCT_PACKAGES += \
    android.hardware.contexthub-service.generic

# CHRE tools
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PACKAGES += \
    chre_power_test_client \
    chre_test_client
endif

# Boot control HAL
PRODUCT_PACKAGES += \
    android.hardware.boot@1.2-impl-pixel-legacy \
    android.hardware.boot@1.2-impl-pixel-legacy.recovery \
    android.hardware.boot@1.2-service \

# Thermal HAL config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/thermal_info_config_$(PRODUCT_HARDWARE).json:$(TARGET_COPY_OUT_VENDOR)/etc/thermal_info_config.json \
    $(LOCAL_PATH)/thermal_info_config_$(PRODUCT_HARDWARE)_jaws.json:$(TARGET_COPY_OUT_VENDOR)/etc/thermal_info_config_jaws.json

#GNSS HAL
PRODUCT_PACKAGES += \
    libgps.utils \
    libgnss \
    libloc_core \
    liblocation_api \
    libbatching \
    libgeofencing \
    android.hardware.gnss@2.1-impl-qti \
    android.hardware.gnss@2.1-service-qti

ENABLE_VENDOR_RIL_SERVICE := true

USE_QCRIL_OEMHOOK := true

HOSTAPD := hostapd
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
HOSTAPD += hostapd_cli
endif
PRODUCT_PACKAGES += $(HOSTAPD)

WPA := wpa_supplicant.conf
WPA += wpa_supplicant_wcn.conf
WPA += wpa_supplicant
PRODUCT_PACKAGES += $(WPA)

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PACKAGES += wpa_cli
endif

# Wifi
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service \
    wificond \
    libwpa_client \
    WifiOverlay

# Connectivity
PRODUCT_PACKAGES += \
    ConnectivityOverlay

# WLAN driver configuration files
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf \
    $(LOCAL_PATH)/wifi_concurrency_cfg.txt:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wifi_concurrency_cfg.txt \
    $(LOCAL_PATH)/WCNSS_qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini \

LIB_NL := libnl_2
PRODUCT_PACKAGES += $(LIB_NL)

# Audio effects
PRODUCT_PACKAGES += \
    libvolumelistener \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libqcomvoiceprocessingdescriptors \
    libqcompostprocbundle

PRODUCT_PACKAGES += \
    audio.primary.msmnile \
    audio.usb.default \
    audio.r_submix.default \
    libaudio-resampler \
    audio.bluetooth.default

PRODUCT_PACKAGES += \
    android.hardware.audio@7.0-impl:32 \
    android.hardware.audio.effect@7.0-impl:32 \
    android.hardware.soundtrigger@2.3-impl \
    android.hardware.bluetooth.audio@2.0-impl \
    android.hardware.audio.service

# Modules for Audio HAL
PRODUCT_PACKAGES += \
    libcirrusspkrprot \
    libsndmonitor \
    libmalistener \
    liba2dpoffload \
    btaudio_offload_if \
    libmaxxaudio \
    libaudiozoom

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PACKAGES += \
    tinyplay \
    tinycap \
    tinymix \
    tinypcminfo \
    cplay
endif

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PACKAGES += chre_test_client
endif

# Audio hal xmls
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/mixer_paths_iaxxx_jaws.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_iaxxx_jaws.xml \
    $(LOCAL_PATH)/audio/mixer_paths_iaxxx_coral.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_iaxxx_coral.xml \
    $(LOCAL_PATH)/audio/mixer_paths_iaxxx_c2proto.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_iaxxx_c2proto.xml \
    $(LOCAL_PATH)/audio/mixer_paths_iaxxx_flame.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_iaxxx_flame.xml \
    $(LOCAL_PATH)/audio/mixer_paths_iaxxx_f2proto.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_iaxxx_f2proto.xml \
    $(LOCAL_PATH)/audio/audio_platform_info_iaxxx_jaws.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_iaxxx_jaws.xml \
    $(LOCAL_PATH)/audio/audio_platform_info_iaxxx_coral.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_iaxxx_coral.xml \
    $(LOCAL_PATH)/audio/audio_platform_info_iaxxx_c2proto.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_iaxxx_c2proto.xml \
    $(LOCAL_PATH)/audio/audio_platform_info_iaxxx_flame.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_iaxxx_flame.xml \
    $(LOCAL_PATH)/audio/audio_platform_info_iaxxx_f2proto.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_iaxxx_f2proto.xml \
    $(LOCAL_PATH)/audio/sound_trigger_mixer_paths_default.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths_default.xml \
    $(LOCAL_PATH)/audio/sound_trigger_mixer_paths_jaws.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths_jaws.xml \
    $(LOCAL_PATH)/audio/sound_trigger_mixer_paths_coral.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths_coral.xml \
    $(LOCAL_PATH)/audio/sound_trigger_mixer_paths_c2proto.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths_c2proto.xml \
    $(LOCAL_PATH)/audio/sound_trigger_mixer_paths_flame.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths_flame.xml \
    $(LOCAL_PATH)/audio/sound_trigger_mixer_paths_f2proto.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths_f2proto.xml

# Audio Policy tables
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio/audio_policy_configuration_a2dp_offload_disabled.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration_a2dp_offload_disabled.xml \
    $(LOCAL_PATH)/audio/audio_policy_configuration_bluetooth_legacy_hal.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration_bluetooth_legacy_hal.xml \
    $(LOCAL_PATH)/audio/bluetooth_hearing_aid_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_hearing_aid_audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    $(LOCAL_PATH)/audio/audio_io_policy.conf:$(TARGET_COPY_OUT_VENDOR)/etc/audio_io_policy.conf \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/a2dp_in_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_in_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/hearing_aid_audio_policy_configuration_7_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/hearing_aid_audio_policy_configuration_7_0.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml

# Audio ACDB data
PRODUCT_COPY_FILES += \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Bluetooth_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Bluetooth_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/General_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/General_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Global_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Global_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Handset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Handset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Hdmi_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Hdmi_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Headset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Headset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Speaker_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Speaker_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Codec_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/Codec_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Bluetooth_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Bluetooth_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/General_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/General_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Global_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Global_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Handset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Handset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Hdmi_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Hdmi_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Headset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Headset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Speaker_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Speaker_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Codec_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/Codec_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Bluetooth_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Bluetooth_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/General_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/General_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Global_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Global_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Handset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Handset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Hdmi_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Hdmi_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Headset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Headset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Speaker_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Speaker_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Codec_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/Codec_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Bluetooth_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Bluetooth_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/General_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/General_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Global_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Global_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Handset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Handset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Hdmi_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Hdmi_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Headset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Headset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Speaker_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Speaker_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Codec_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/Codec_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Bluetooth_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Bluetooth_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/General_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/General_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Global_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Global_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Handset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Handset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Hdmi_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Hdmi_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Headset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Headset_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Speaker_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Speaker_cal.acdb \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Codec_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/Codec_cal.acdb \
    device/google/coral/audio/acdbdata/adsp_avs_config.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/adsp_avs_config.acdb


# Audio ACDB workspace files for QACT
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_COPY_FILES += \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/workspaceFile.qwsp:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-jaws-snd-card/workspaceFile.qwsp \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/workspaceFile.qwsp:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-coral-snd-card/workspaceFile.qwsp \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/workspaceFile.qwsp:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-c2proto-snd-card/workspaceFile.qwsp \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/workspaceFile.qwsp:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-flame-snd-card/workspaceFile.qwsp \
    device/google/coral/audio/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/workspaceFile.qwsp:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/OEM/sm8150-iaxxx-f2proto-snd-card/workspaceFile.qwsp
endif

# Audio speaker tunning config data
PRODUCT_COPY_FILES += \
    device/google/coral/audio/data/crus_sp_config_coral_rx.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/crus_sp_config_coral_rx.bin \
    device/google/coral/audio/data/crus_sp_config_coral_tx.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/crus_sp_config_coral_tx.bin \
    device/google/coral/audio/data/crus_sp_config_flame_rx.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/crus_sp_config_flame_rx.bin \
    device/google/coral/audio/data/crus_sp_config_flame_tx.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/crus_sp_config_flame_tx.bin

# Audio audiozoom config data
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audiozoom.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audiozoom.xml

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(LOCAL_PATH)/media_codecs_omx.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_omx.xml \
    $(LOCAL_PATH)/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    $(LOCAL_PATH)/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml


# Vendor seccomp policy files for media components:
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/seccomp_policy/mediacodec.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy

PRODUCT_PROPERTY_OVERRIDES += \
    vendor.audio.snd_card.open.retries=50


ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
# Subsystem ramdump
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.sys.ssr.enable_ramdumps=1
endif

# Subsystem silent restart
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.sys.ssr.restart_level=modem,slpi,adsp

# setup dalvik vm configs
$(call inherit-product, frameworks/native/build/phone-xhdpi-6144-dalvik-heap.mk)

# Use the default charger mode images
PRODUCT_PACKAGES += \
    charger_res_images

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
# b/36703476: Set default log size to 1M
PRODUCT_PROPERTY_OVERRIDES += \
  ro.logd.size=1M
# b/114766334: persist all logs by default rotating on 30 files of 1MiB
PRODUCT_PROPERTY_OVERRIDES += \
  logd.logpersistd=logcatd \
  logd.logpersistd.size=30
endif

# Dumpstate HAL
PRODUCT_PACKAGES += \
    android.hardware.dumpstate@1.1-service.coral

# Storage: for factory reset protection feature
PRODUCT_PROPERTY_OVERRIDES += \
    ro.frp.pst=/dev/block/bootdevice/by-name/frp

PRODUCT_PACKAGES += \
    vndk-sp

# Override heap growth limit due to high display density on device
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapgrowthlimit=256m \

# Use 64-bit dex2oat for better dexopt time.
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.dex2oat64.enabled=true

PRODUCT_PACKAGES += \
    ipacm \
    IPACM_cfg.xml

#Set default CDMA subscription to RUIM
PRODUCT_PROPERTY_OVERRIDES += \
    ro.telephony.default_cdma_sub=0

# Set network mode to Global by default and no DSDS/DSDA
PRODUCT_PROPERTY_OVERRIDES += \
    ro.telephony.default_network=10

# Set display color mode to Adaptive by default
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.sf.color_saturation=1.0 \
    persist.sys.sf.native_mode=2 \
    persist.sys.sf.color_mode=9

# Keymaster configuration
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.device_id_attestation.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.device_id_attestation.xml \
    frameworks/native/data/etc/android.hardware.device_unique_attestation.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.device_unique_attestation.xml

# Enable modem logging
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.radio.log_loc="/data/vendor/modem_dump" \
    ro.vendor.radio.log_prefix="modem_log_"

# Enable modem logging for debug
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.sys.modem.diag.mdlog=true
else
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.sys.modem.diag.mdlog=false
endif
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.sys.modem.diag.mdlog_br_num=5

# Enable tcpdump_logger on userdebug and eng
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
    PRODUCT_PROPERTY_OVERRIDES += \
        persist.vendor.tcpdump.log.alwayson=false \
        persist.vendor.tcpdump.log.br_num=5
endif

# Preopt SystemUI
PRODUCT_DEXPREOPT_SPEED_APPS += SystemUIGoogle  # For internal
PRODUCT_DEXPREOPT_SPEED_APPS += SystemUI  # For AOSP

# Compile SystemUI on device with `speed`.
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.systemuicompilerfilter=speed

# Enable stats logging in LMKD
TARGET_LMKD_STATS_LOG := true

# default usb oem functions
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
  PRODUCT_PROPERTY_OVERRIDES += \
      persist.vendor.usb.usbradio.config=diag
endif

# Enable app/sf phase offset as durations. The numbers below are translated from the existing
# positive offsets by finding the duration app/sf will have with the offsets.
# For SF the previous value was 6ms which under 16.6ms vsync time (60Hz) will leave SF with ~10.5ms
# for each frame. For App the previous value was 2ms which under 16.6ms vsync time will leave the
# App with ~20.5ms (16.6ms * 2 - 10.5ms - 2ms). The other values were calculated similarly.
# Full comparison between the old vs. the new values are captured in
# https://docs.google.com/spreadsheets/d/1a_5cVNY3LUAkeg-yL56rYQNwved6Hy-dvEcKSxp6f8k/edit

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.use_phase_offsets_as_durations=1
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.late.sf.duration=10500000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.late.app.duration=20500000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.early.sf.duration=16000000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.early.app.duration=16500000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.earlyGl.sf.duration=13500000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += debug.sf.earlyGl.app.duration=21000000

# Enable backpressure for GL comp
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.enable_gl_backpressure=1

# Enable EGL image tracking on SF for b/137514000
PRODUCT_PROPERTY_OVERRIDES += debug.sf.enable_egl_image_tracker=1

# Do not skip init trigger by default
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    vendor.skip.init=0

BOARD_USES_QCNE := true

#per device
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/coral/init.coral.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.coral.rc \
    $(LOCAL_PATH)/flame/init.flame.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.flame.rc

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/sec_config:$(TARGET_COPY_OUT_VENDOR)/etc/sec_config

# GPS configuration file
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/gps.conf:$(TARGET_COPY_OUT_VENDOR)/etc/gps.conf

PRODUCT_USE_DYNAMIC_PARTITIONS := true

# insmod files
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/init.insmod.coral.cfg:$(TARGET_COPY_OUT_VENDOR)/etc/init.insmod.coral.cfg \
	$(LOCAL_PATH)/init.insmod.flame.cfg:$(TARGET_COPY_OUT_VENDOR)/etc/init.insmod.flame.cfg

# Use /product/etc/fstab.postinstall to mount system_other
PRODUCT_PRODUCT_PROPERTIES += \
    ro.postinstall.fstab.prefix=/product

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/fstab.postinstall:$(TARGET_COPY_OUT_PRODUCT)/etc/fstab.postinstall

# powerstats HAL
PRODUCT_PACKAGES += \
    android.hardware.power.stats@1.0-service.pixel

PRODUCT_PACKAGES_DEBUG += \
    pwrstats_util

# Recovery
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/init.recovery.device.rc:recovery/root/init.recovery.coral.rc \
    $(LOCAL_PATH)/init.recovery.device.rc:recovery/root/init.recovery.flame.rc

# Oslo feature flag
PRODUCT_PRODUCT_PROPERTIES += \
    ro.vendor.aware_available=true

QTI_TELEPHONY_UTILS := qti-telephony-utils
QTI_TELEPHONY_UTILS += qti_telephony_utils.xml
PRODUCT_PACKAGES += $(QTI_TELEPHONY_UTILS)

HIDL_WRAPPER := qti-telephony-hidl-wrapper
HIDL_WRAPPER += qti_telephony_hidl_wrapper.xml
HIDL_WRAPPER += qti-telephony-hidl-wrapper-prd
HIDL_WRAPPER += qti_telephony_hidl_wrapper_prd.xml
PRODUCT_PACKAGES += $(HIDL_WRAPPER)

# Increment the SVN for any official public releases
PRODUCT_PROPERTY_OVERRIDES += \
	ro.vendor.build.svn=62

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/powerhint.json:$(TARGET_COPY_OUT_VENDOR)/etc/powerhint.json


# Disable SPU usage
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.gatekeeper.disable_spu = true

# Enable iwlan service logging for debug
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
    PRODUCT_PROPERTY_OVERRIDES += persist.vendor.iwlan.logging.logcat=true
endif

# Vendor verbose logging default property
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.verbose_logging_enabled=true
else
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.verbose_logging_enabled=false
endif

# Disable Rescue Party on userdebug & eng build
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PRODUCT_PROPERTIES += \
    persist.sys.disable_rescue=true
endif

# Set support one-handed mode
PRODUCT_PRODUCT_PROPERTIES += \
    ro.support_one_handed_mode=true

# Set system properties identifying the chipset
PRODUCT_VENDOR_PROPERTIES += ro.soc.manufacturer=Qualcomm
PRODUCT_VENDOR_PROPERTIES += ro.soc.model=SM8150

# Security
-include vendor/qcom/sm8150/proprietary/securemsm/config/keymaster_vendor_proprietary_board.mk
-include vendor/qcom/sm8150/proprietary/securemsm/config/keymaster_vendor_proprietary_product.mk

include hardware/google/pixel/common/pixel-common-device.mk
include hardware/google/pixel/vibrator/cs40l25/device.mk
include hardware/google/pixel/pixelstats/device.mk
include hardware/google/pixel/mm/device_legacy.mk
include hardware/google/pixel/thermal/device.mk

# Citadel
include hardware/google/pixel/citadel/citadel.mk

# power HAL
-include hardware/google/pixel/power-libperfmgr/aidl/device.mk

# Pixel Logger
include hardware/google/pixel/PixelLogger/PixelLogger.mk

$(call inherit-product, $(SRC_TARGET_DIR)/product/virtual_ab_ota_retrofit.mk)

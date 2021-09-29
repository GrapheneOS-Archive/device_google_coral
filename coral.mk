# Inherit AOSP product configuration
$(call inherit-product, device/google/coral/aosp_coral.mk)

# Remove AOSP prefix from product name
PRODUCT_NAME := coral
# Tell build system not to bundle sample APNs from AOSP
OEM_APNS := true

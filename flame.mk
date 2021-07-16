# Inherit AOSP product configuration
$(call inherit-product, device/google/coral/aosp_flame.mk)

# Remove AOSP prefix from product name
PRODUCT_NAME := flame

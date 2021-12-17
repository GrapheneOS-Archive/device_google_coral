# Mitigation for Qualcomm apdp debug policy

# "fastboot oem sha1sum apdp" gives a sha1sum of 1ADC95BEBE9EEA8C112D40CD04AB7A8D75C4F961
# Thus we create a file, apdp.img using "dd if=/dev/zero of=apdp.img bs=65536 count=1"
# "sha1sum apdp.img" will produce the same hash
# A zeroed out policy means that no policy is set.

LOCAL_PATH := $(call my-dir)

$(call add-radio-file,radio/apdp.img)

# Mitigation for Qualcomm msadp debug policy

# "fastboot oem sha1sum msadp" gives a sha1sum of 1ADC95BEBE9EEA8C112D40CD04AB7A8D75C4F961
# Thus we create a file, msadp.img using "dd if=/dev/zero of=msadp.img bs=65536 count=1"
# "sha1sum msadp.img" will produce the same hash
# A zeroed out policy means that no policy is set.

$(call add-radio-file,radio/msadp.img)

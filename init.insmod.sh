#! /vendor/bin/sh

#########################################
### init.insmod.cfg format:           ###
### --------------------------------- ###
### [insmod|setprop] [path|prop name] ###
### ...                               ###
#########################################

if [ $# -eq 1 ]; then
  cfg_file=$1
else
  exit 1
fi

if [ -f $cfg_file ]; then
  while IFS=" " read -r action name
  do
    case $action in
      "insmod") insmod $name ;;
      "setprop") setprop $name 1 ;;
    esac
  done < $cfg_file
fi

# set property even if there is no insmod config
# as property value "1" is expected in early-boot trigger
setprop vendor.all.modules.ready 1

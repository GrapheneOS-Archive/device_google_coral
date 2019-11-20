#!/vendor/bin/sh

while true
do
  logline="tz:"
  for f in /sys/class/thermal/thermal*
  do
    temp=`cat $f/temp`
    logline+="|$temp"
  done
  logline+=" cdev:"
  for f in /sys/class/thermal/cooling_device*
  do
    cur_state=`cat $f/cur_state`
    logline+="|$cur_state"
  done
  log -p w -t THERMAL_LOG $logline
  sleep 5
done

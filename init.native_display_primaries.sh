#!/vendor/bin/sh

NATIVE_GAMUT_FILE='/mnt/vendor/persist/display/native_gamut.csv'

get_primary () { grep $1 $NATIVE_GAMUT_FILE | tr '\r' ' '; }
# Data for a primary is stored in a csv string of 6 values: R,G,B,X,Y,Z
# These helper functions provide access these values
get_primary_coords () { echo $1 | cut -d ',' -f 4,5,6; }
get_primary_coord () { echo $1 | cut -d ',' -f $2; }
get_primary_X () { get_primary_coord $1 4; }
get_primary_Y () { get_primary_coord $1 5; }
get_primary_Z () { get_primary_coord $1 6; }

float_ge_0 () { awk -v val=$1 'BEGIN { printf (val >= 0 ? "1" : "0") }'; }

verify_primary () {
	primary=$1
	if [ -z "$primary" ]; then
		return 1
	fi

	X=$(get_primary_X $primary)
	Y=$(get_primary_Y $primary)
	Z=$(get_primary_Z $primary)

	# Check if coordinate is empty
	if [ -z "$X" ] || [ -z "$Y" ] || [ -z "$Z" ]; then
		return 1
	fi

	# Check if coordinate is >= 0
	if [ "$(float_ge_0 $X)" -ne 1 ] || [ "$(float_ge_0 $Y)" -ne 1 ] || [ "$(float_ge_0 $Z)" -ne 1 ]; then
		return 1
	fi

	return 0
}

init_primaries_from_file () {
	red=$(get_primary 255,0,0)
	verify_primary $red
	if [ "$?" -ne 0 ]; then
		return 1
	fi

	green=$(get_primary 0,255,0)
	verify_primary $green
	if [ "$?" -ne 0 ]; then
		return 1
	fi

	blue=$(get_primary 0,0,255)
	verify_primary $blue
	if [ "$?" -ne 0 ]; then
		return 1
	fi

	white=$(get_primary 255,255,255)
	verify_primary $white
	if [ "$?" -ne 0 ]; then
		return 1
	fi

	setprop vendor.display.primary_red $(get_primary_coords $red)
	setprop vendor.display.primary_green $(get_primary_coords $green)
	setprop vendor.display.primary_blue $(get_primary_coords $blue)
	setprop vendor.display.primary_white $(get_primary_coords $white)

	return 0
}

init_primaries_from_defaults () {
	device=$(getprop ro.hardware)

	if [ "$device" = "flame" ]; then
		setprop vendor.display.primary_red 0.571498,0.268889,0.000102
		setprop vendor.display.primary_green 0.264549,0.688889,0.044777
		setprop vendor.display.primary_blue 0.183287,0.073333,0.965525
		setprop vendor.display.primary_white 0.950352,1.0,1.089366

	elif [ "$device" = "coral" ]; then
		setprop vendor.display.primary_red 0.500651,0.235556,0.00009
		setprop vendor.display.primary_green 0.267109,0.695556,0.045211
		setprop vendor.display.primary_blue 0.16107,0.064444,0.848491
		setprop vendor.display.primary_white 0.950352,1.0,1.089366
	fi
	# Leave properties unset if no matching device
}

init_primaries_from_file
if [ "$?" -ne 0 ]; then
	init_primaries_from_defaults
fi
setprop vendor.display.native_display_primaries_ready 1

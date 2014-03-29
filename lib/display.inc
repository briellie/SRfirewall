#!/bin/bash
# By Brielle Bruns <bruns@2mbit.com>
# URL: http://www.sosdg.org/freestuff/firewall
# License: GPLv3
#
#    Copyright (C) 2009 - 2014  Brielle Bruns
#    Copyright (C) 2009 - 2014  The Summit Open Source Development Group
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.


# ANSI color sequences
BLUE="\E[34m"
GREEN="\E[32m"
RED="\E[31m"
YELLOW="\E[33m"
PURPLE="\E[35m"
AQUA="\E[36m"
WHITE="\E[1m"
GREY="\E[37m"
DEFAULT_COLOR="\E[39m"

# display_c $COLOR $TEXT BOOL(YN)
# $COLOR being bash colors
# $TEXT being what to output (make sure to put " " around text)
# BOOL being (Y or N) to do newline at end or not
function display_c {
	unset COLOR_CODE TEXT NEWLINE
	DEFAULT_COLOR="\E[39m"
	COLOR_CODE=`pick_color $1`
	TEXT="$2"
	if [ "$3" == "N" ]; then
		NEWLINE="-n"
	fi
	echo -e ${NEWLINE} "${COLOR_CODE}${TEXT}${DEFAULT_COLOR}"
}

# pick_color $COLOR
# returns appropriate color codes for use in display_c and such
function pick_color {
	case $1 in
		BLUE) COLOR="\E[34m" ;;
		GREEN) COLOR="\E[32m" ;;
		RED) COLOR="\E[31m" ;;
		YELLOW) COLOR="\E[33m" ;;
		PURPLE) COLOR="\E[35m" ;;
		AQUA) COLOR="\E[36m" ;;
		WHITE) COLOR="\E[1m" ;;
		GREY) COLOR="\E[37m" ;;
		*) COLOR="\E[37m" ;;
	esac
	echo "${COLOR}"
}

# reset_color
function reset_color {
	unset NEWLINE
	DEFAULT_COLOR="\E[39m"
	if [ "$1" == "N" ]; then
		NEWLINE="-n"
	fi
	echo ${NEWLINE} -e "${DEFAULT_COLOR}"
}

# show_help
# Show command line options help
function show_help {
	 echo "Firewall/SOSDG ${FW_VERSION} - Brielle Bruns <bruns@2mbit.com>"
 	 echo -e "\t--help\t\tShows this info"
 	 echo -e "\t--flush\t\tFlushes all rules back to default ACCEPT"
 	 echo -e "\t--generate-cache\tGenerate cached rule file"
 }
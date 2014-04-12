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

function load_kernel_modules {
	LOAD_MODULES=$1
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Loading kernel module:" N
	for i in $LOAD_MODULES; do
		${debug} ${DebugColor} "${i} " N
		${MODPROBE} ${i}
	done
	${debug} ${DebugColor} "\n${FUNCNAME}:${DEFAULT_COLOR} done."
}
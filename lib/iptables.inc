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

# iptables_rules_flush (ipv6|ipv4)
# Clear all rules from iptables - be very careful in how this is called as it
# could easily lock out the user from the network.  Best way to be safe, is to
# call iptables_policy_reset first then this function.
function iptables_rules_flush {
	IP_VERSION=$1
	case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES} ; TABLE_NAMES=/proc/net/ip6_tables_names ;;
		ipv4|*) VER_IPTABLES=${IPTABLES} ; TABLE_NAMES=/proc/net/ip_tables_names ;;
	esac
	${display_c} RED "Flushing ${IP_VERSION} rules..."
	${VER_IPTABLES} --flush &>/dev/null
	${VER_IPTABLES} -F OUTPUT &>/dev/null
	${VER_IPTABLES} -F PREROUTING &>/dev/null
	${VER_IPTABLES} -F POSTROUTING &>/dev/null
	for i in `cat $TABLE_NAMES`; do
		${VER_IPTABLES} -F -t $i &>/dev/null
	done
	${VER_IPTABLES} -X
}

# iptables_policy_set (ipv6|ipv4) (ACCEPT|DROP)
# Sets all policy rules to either ACCEPT or DROP for ipv4 or ipv6
# If no policy given, assume ACCEPT
function iptables_policy_reset {
	IP_VERSION=$1
	SET_POLICY=${2=ACCEPT}
	case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES} ;;
		ipv4|*) VER_IPTABLES=${IPTABLES} ;;
	esac
	${display_c} RED "Setting ${IP_VERSION} policies to ${SET_POLICY}..."
	${VER_IPTABLES} --policy INPUT ${SET_POLICY}
	${VER_IPTABLES} --policy OUTPUT ${SET_POLICY}
	${VER_IPTABLES} --policy FORWARD ${SET_POLICY}
}

# setup_iptables_chains (ipv4|ipv6)
# Creates the default chains when called
function setup_uptables_chains {
	IP_VERSION=$1
	case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES} ;;
		ipv4|*) VER_IPTABLES=${IPTABLES} ;;
	esac
	${display_c} GREEN "Setting up default chains for ${IP_VERSION}..."
	${VER_IPTABLES} -N ${InCustomPreRules}
	${VER_IPTABLES} -N ${InPreRules}
	${VER_IPTABLES} -N ${OutCustomPreRules}
	${VER_IPTABLES} -N ${OutPreRules}
	${VER_IPTABLES} -N ${Trusted}
	${VER_IPTABLES} -N ${InEasyBlock}
	${VER_IPTABLES} -N ${OutEasyBlock}
	${VER_IPTABLES} -N ${InCustomFilter}
	${VER_IPTABLES} -N ${InFilter}
	${VER_IPTABLES} -N ${OutCustomFilter}
	${VER_IPTABLES} -N ${OutFilter}
	${VER_IPTABLES} -N ${FwdCustomFilter}
	${VER_IPTABLES} -N ${FwdFilter}
	${VER_IPTABLES} -N ${CustomNAT}
	${VER_IPTABLES} -N ${NAT}
	${VER_IPTABLES} -N ${CustomPortForward}
	${VER_IPTABLES} -N ${PortForward}
	${VER_IPTABLES} -N ${InCustomPostRules}
	${VER_IPTABLES} -N ${InPostRules}
	${VER_IPTABLES} -N ${OutCustomPostRules}
	${VER_IPTABLES} -N ${InPostRules}
}
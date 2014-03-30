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
	${display} GREEN "Flushing ${IP_VERSION} rules..."
	${VER_IPTABLES} -F &>/dev/null
	${VER_IPTABLES} -X &>/dev/null
	${VER_IPTABLES} -F INPUT &>/dev/null
	${VER_IPTABLES} -F OUTPUT &>/dev/null
	${VER_IPTABLES} -F FORWARD &>/dev/null
	${VER_IPTABLES} -t nat -F &>/dev/null
	${VER_IPTABLES} -t nat -X &>/dev/null
	${VER_IPTABLES} -t mangle -F &>/dev/null
	${VER_IPTABLES} -t mangle -X &>/dev/null
	${VER_IPTABLES} -P INPUT ACCEPT &>/dev/null
	${VER_IPTABLES} -P OUTPUT ACCEPT &>/dev/null
	${VER_IPTABLES} -P FORWARD ACCEPT &>/dev/null
	#for i in `cat $TABLE_NAMES`; do
	#	${VER_IPTABLES} -F -t $i &>/dev/null
	#done
	#${VER_IPTABLES} -X
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
function setup_iptables_chains {
	IP_VERSION=$1
	case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	# Create the actual chains
	${display} GREEN "Setting up chains for ${IP_VERSION}..."
	${VER_IPTABLES} -N ${InPreRules}
	${VER_IPTABLES} -N ${OutPreRules}
	${VER_IPTABLES} -N ${Trusted}
	${VER_IPTABLES} -N ${InEasyBlock}
	${VER_IPTABLES} -N ${OutEasyBlock}
	${VER_IPTABLES} -N ${InFilter}
	${VER_IPTABLES} -N ${OutFilter}
	${VER_IPTABLES} -N ${FwdFilter}
	${VER_IPTABLES} -N ${NAT} -t nat
	${VER_IPTABLES} -N ${PortForward} -t nat
	${VER_IPTABLES} -N ${InPostRules}
	${VER_IPTABLES} -N ${OutPostRules}
	
	# Set up rules - the order matters - we do it separately here
	# for easy viewing of order
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/prerun.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/prerun.sh; fi
	${debug} ${DebugColor} "Setting up InPreRules"
	${VER_IPTABLES} -A INPUT -j ${InPreRules}
	${debug} ${DebugColor} "Setting up OutPreRules"
	${VER_IPTABLES} -A OUTPUT -j ${OutPreRules}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/trusted.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/trusted.sh; fi
	${debug} ${DebugColor} "Setting up Trusted"
	${VER_IPTABLES} -A INPUT -j ${Trusted}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/easyblock.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/easyblock.sh; fi
	${debug} ${DebugColor} "Setting up InEasyBlock"
	${VER_IPTABLES} -A INPUT -j ${InEasyBlock}
	${debug} ${DebugColor} "Setting up OutEasyBlock"
	${VER_IPTABLES} -A OUTPUT -j ${OutEasyBlock}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/filter.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/filter.sh; fi
	${debug} ${DebugColor} "Setting up InFilter"
	${VER_IPTABLES} -A INPUT -j ${InFilter}
	${debug} ${DebugColor} "Setting up OutFilter"
	${VER_IPTABLES} -A OUTPUT -j ${OutFilter}
	${debug} ${DebugColor} "Setting up FwdFilter"
	${VER_IPTABLES} -A FORWARD -j ${FwdFilter}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/nat.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/nat.sh; fi
	${debug} ${DebugColor} "Setting up NAT"
	${VER_IPTABLES} -A POSTROUTING -t nat -j ${NAT}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/portfw.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/portfw.sh; fi
	${debug} ${DebugColor} "Setting up PortForward"
	${VER_IPTABLES} -A PREROUTING -t nat -j ${PortForward}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/postrun.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/postrun.sh; fi
	${debug} ${DebugColor} "Setting up InPostRules"
	${VER_IPTABLES} -A INPUT -j ${InPostRules}
	${debug} ${DebugColor} "Setting up OutPostRules"
	${VER_IPTABLES} -A OUTPUT -j ${OutPostRules}
}

function allow_all_loopback {
	IP_VERSION=$1
	case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "allow_all_loopback: loaded"
	${VER_IPTABLES} -A ${InPreRules} -i lo -j ACCEPT
	${VER_IPTABLES} -A ${OutPreRules} -o lo -j ACCEPT
}
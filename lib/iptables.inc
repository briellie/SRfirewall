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
	${VER_IPTABLES} -P INPUT ACCEPT &>/dev/null
	${VER_IPTABLES} -P OUTPUT ACCEPT &>/dev/null
	${VER_IPTABLES} -P FORWARD ACCEPT &>/dev/null
	${VER_IPTABLES} -F &>/dev/null
	${VER_IPTABLES} -X &>/dev/null
	${VER_IPTABLES} -F INPUT &>/dev/null
	${VER_IPTABLES} -F OUTPUT &>/dev/null
	${VER_IPTABLES} -F FORWARD &>/dev/null
	${VER_IPTABLES} -t nat -F &>/dev/null
	${VER_IPTABLES} -t nat -X &>/dev/null
	${VER_IPTABLES} -t mangle -F &>/dev/null
	${VER_IPTABLES} -t mangle -X &>/dev/null
	for i in `cat $TABLE_NAMES`; do
		${VER_IPTABLES} -F -t $i &>/dev/null
	done
}

# iptables_policy_set (ipv6|ipv4) (ACCEPT|DROP)
# Sets all policy rules to either ACCEPT or DROP for ipv4 or ipv6
# If no policy given, assume ACCEPT
function default_policy_set {
	IP_VERSION=$1
	INPOLICY=${2=ACCEPT}
	OUTPOLICY=${3=ACCEPT}
	FWDPOLICY=${4=ACCEPT}
	case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES} ;;
		ipv4|*) VER_IPTABLES=${IPTABLES} ;;
	esac
	${display} RED "Setting ${IP_VERSION} policies to INPUT:${INPOLICY} OUTPUT:${OUTPOLICY} FORWARD:${FWDPOLICY}..."
	${VER_IPTABLES} --policy INPUT ${INPOLICY}
	${VER_IPTABLES} --policy OUTPUT ${OUTPOLICY}
	${VER_IPTABLES} --policy FORWARD ${FWDPOLICY}
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
	${VER_IPTABLES} -N ${InEasyBlock}
	${VER_IPTABLES} -N ${OutEasyBlock}
	${VER_IPTABLES} -N ${InFilter}
	${VER_IPTABLES} -N ${OutFilter}
	${VER_IPTABLES} -N ${FwdFilter}
	([[ ${IPVER} == "4" ]] && [[ ${Enablev4NAT} == "yes" ]]) && ${VER_IPTABLES} -N ${NAT} -t nat
	([[ ${IPVER} == "6" ]] && [[ ${Enablev6NAT} == "yes" ]]) && ${VER_IPTABLES} -N ${NAT} -t nat
	([[ ${IPVER} == "4" ]] && [[ ${Enablev4NAT} == "yes" ]]) && ${VER_IPTABLES} -N ${PortForward} -t nat
	([[ ${IPVER} == "6" ]] && [[ ${Enablev6NAT} == "yes" ]]) && ${VER_IPTABLES} -N ${PortForward} -t nat
	[[ ${IPVER} == "6" ]] && ${VER_IPTABLES} -N ${v6ICMP}
	${VER_IPTABLES} -N ${InPostRules}
	${VER_IPTABLES} -N ${OutPostRules}
	
	# Set up rules - the order matters - we do it separately here
	# for easy viewing of order
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/prerun.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/prerun.sh; fi
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up InPreRules"
	${VER_IPTABLES} -A INPUT -j ${InPreRules}
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up OutPreRules"
	${VER_IPTABLES} -A OUTPUT -j ${OutPreRules}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/easyblock.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/easyblock.sh; fi
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up InEasyBlock"
	${VER_IPTABLES} -A INPUT -j ${InEasyBlock}
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up OutEasyBlock"
	${VER_IPTABLES} -A OUTPUT -j ${OutEasyBlock}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/filter.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/filter.sh; fi
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up InFilter"
	${VER_IPTABLES} -A INPUT -j ${InFilter}
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up OutFilter"
	${VER_IPTABLES} -A OUTPUT -j ${OutFilter}
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up FwdFilter"
	${VER_IPTABLES} -A FORWARD -j ${FwdFilter}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/nat.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/nat.sh; fi
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up NAT"
	([[ ${IPVER} == "4" ]] && [[ ${Enablev4NAT} == "yes" ]]) && ${VER_IPTABLES} -A POSTROUTING -t nat -j ${NAT}
	([[ ${IPVER} == "6" ]] && [[ ${Enablev6NAT} == "yes" ]]) && ${VER_IPTABLES} -A POSTROUTING -t nat -j ${NAT}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/portfw.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/portfw.sh; fi
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up PortForward"
	([[ ${IPVER} == "4" ]] && [[ ${Enablev4NAT} == "yes" ]]) && ${VER_IPTABLES} -A PREROUTING -t nat -j ${PortForward}
	([[ ${IPVER} == "6" ]] && [[ ${Enablev6NAT} == "yes" ]]) && ${VER_IPTABLES} -A PREROUTING -t nat -j ${PortForward}
	if [ -x ${FWCONFIGDIR}/ipv${IPVER}/custom/postrun.sh ]; then . ${FWCONFIGDIR}/ipv${IPVER}/custom/postrun.sh; fi
	[[ ${IPVER} == "6" ]] && ${VER_IPTABLES} -A INPUT -j ${v6ICMP}
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up InPostRules"
	${VER_IPTABLES} -A INPUT -j ${InPostRules}
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Setting up OutPostRules"
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
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loaded"
	${VER_IPTABLES} -A ${InPreRules} -i lo -j ACCEPT
	${VER_IPTABLES} -A ${OutPreRules} -o lo -j ACCEPT
}

function allow_trusted_hosts {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	if [ -e "${FWCONFIGDIR}/ipv${IPVER}/trusted.conf" ]; then
		for i in `grep -v "\#" "${FWCONFIGDIR}/ipv${IPVER}/trusted.conf"`; do
			${VER_IPTABLES} -A ${InPreRules} -s $i -j ACCEPT
			${VER_IPTABLES} -A ${OutPreRules} -d $i -j ACCEPT
		done
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
	else
		${display} RED "File Missing: ${FWCONFIGDIR}/ipv${IPVER}/trusted.conf"
		${display} RED "Error: can not load trusted hosts file."
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} failed"
	fi
}
function enable_mss_clamp {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	if [ -e "${FWCONFIGDIR}/ipv${IPVER}/mss-clamp.conf" ]; then
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} read ${FWCONFIGDIR}/ipv${IPVER}/mss-clamp.conf successful"
		while read -r interface mss type msssize; do
			[[ ${interface} = \#* ]] && continue
			[[ ${interface} = "" ]] && continue
			#[[ -z ${mss} ]] && mss="-"
			[[ ${mss} != "-" ]] && mss="-m tcpmss --mss ${mss}"
			[[ ${mss} == "-" ]] && mss=""
			[[ -z ${type} ]] && type="-"
			[[ ${type} == "-" ]] && type="out"
			[[ ${type} == "out" ]] && type="${OutFilter}"
			[[ ${type} == "fwd" ]] && type="${FwdFilter}"
			[[ ${type} == "in" ]] && type="${InFilter}"
			[[ -z ${msssize} ]] && msssize="-"
			[[ ${msssize} != "-" ]] && msssize="--set-mss ${msssize}"
			[[ ${msssize} == "-" ]] && msssize="--clamp-mss-to-pmtu"
			#[[ ${interface} != "all" ]] && interface="-o ${interface}"
			[[ ${type} == "${InFilter}" ]] && interface="-i ${interface}"
			[[ ${type} == "${OutFilter}" ]] && interface="-o ${interface}"
			[[ ${interface} == "all" ]] && interface=""
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Read: ${interface} ${mss} ${type} ${msssize}"
			${VER_IPTABLES} -A ${type} -p tcp --tcp-flags SYN,RST SYN -j TCPMSS \
				 ${interface} ${mss} ${msssize}
			unset interface mss type msssize
		done < "${FWCONFIGDIR}/ipv${IPVER}/mss-clamp.conf"

		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
	else
		${display} RED "File Missing: ${FWCONFIGDIR}/ipv${IPVER}/mss-clamp.conf"
		${display} RED "Error: can not load mss clamp file."
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} failed"
	fi
}

function allow_resolvconf_servers {
		IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	[[ ${IP_VERSION} == "ipv4" ]] && ResolvConfFile="${ResolvConfv4File}"
	[[ ${IP_VERSION} == "ipv6" ]] && ResolvConfFile="${ResolvConfv6File}"
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Using ${ResolvConfFile} as resolv.conf"
	while read -r type server; do
		[[ ${type} != "nameserver" ]] && continue
		[[ ${type} = "" ]] && continue
		([[ ${server} =~ ":" ]] && [[ ${IP_VERSION} = "ipv4" ]]) && continue
		([[ ! ${server} =~ ":" ]] && [[ ${IP_VERSION} = "ipv6" ]]) && continue
		
		use_conntrack="no"
		([[ ${IP_VERSION} == "ipv4" ]] && [[ ${Enablev4ConnectionTracking} == "yes" ]]) && use_conntrack="yes"
		([[ ${IP_VERSION} == "ipv6" ]] && [[ ${Enablev6ConnectionTracking} == "yes" ]]) && use_conntrack="yes"

		if [[ ${use_conntrack} == "yes" ]]; then
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Added ${server} to conntrack list for DNS traffic"
			${VER_IPTABLES} -A ${OutPreRules} -p udp -d ${server} --dport 53 ${M_STATE} ${C_STATE} NEW,ESTABLISHED -j ACCEPT
			${VER_IPTABLES} -A ${InPreRules} -p udp -s ${server} --sport 53 ${M_STATE} ${C_STATE} ESTABLISHED,RELATED -j ACCEPT
		else
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Added ${server} to DNS client trusted list"
			${VER_IPTABLES} -A ${OutPreRules} -p udp -s ${server} --sport 1024:65535 --dport 53 -j ACCEPT
			${VER_IPTABLES} -A ${InPreRules} -p udp -d ${server} --dport 1024:65535 --sport 53 -j ACCEPT
			#${VER_IPTABLES} -A ${OutPreRules} -p tcp -s ${server} --sport 1024:65535 --dport 53 -j ACCEPT
			#${VER_IPTABLES} -A ${InPreRules} -p tcp -d ${server} --dport 1024:65535 --sport 53 -j ACCEPT
		fi
		done < "${ResolvConfFile}"
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
}

function allow_dnsclient_manual {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	DNS_SERVERS="$2"
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	use_conntrack="no"
	([[ ${IP_VERSION} == "ipv4" ]] && [[ ${Enablev4ConnectionTracking} == "yes" ]]) && use_conntrack="yes"
	([[ ${IP_VERSION} == "ipv6" ]] && [[ ${Enablev6ConnectionTracking} == "yes" ]]) && use_conntrack="yes"
	for i in ${DNS_SERVERS}; do
		if [[ ${use_conntrack} == "yes" ]]; then
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Added ${server} to conntrack list for DNS traffic"
			${VER_IPTABLES} -A ${OutPreRules} -p udp -d ${i} --dport 53 ${M_STATE} ${C_STATE} NEW,ESTABLISHED -j ACCEPT
			${VER_IPTABLES} -A ${InPreRules} -p udp -s ${i} --sport 53 ${M_STATE} ${C_STATE} ESTABLISHED,RELATED -j ACCEPT
		else
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Added ${i} to DNS client trusted list"
			${VER_IPTABLES} -A ${OutPreRules} -p udp -s ${i} --sport 1024:65535 --dport 53 -j ACCEPT
			${VER_IPTABLES} -A ${InPreRules} -p udp -d ${i} --dport 1024:65535 --sport 53 -j ACCEPT
			#${VER_IPTABLES} -A ${OutPreRules} -p tcp -s ${i} --sport 1024:65535 --dport 53 -j ACCEPT
			#${VER_IPTABLES} -A ${InPreRules} -p tcp -d ${i} --dport 1024:65535 --sport 53 -j ACCEPT
		fi
	done
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
}

function enable_easyblock {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	if [ -e "${FWCONFIGDIR}/ipv${IPVER}/easyblock.conf" ]; then
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} read ${FWCONFIGDIR}/ipv${IPVER}/easyblock.conf successful"
		while read -r direction interface address port protocol; do
			[[ ${direction} = \#* ]] && continue
			[[ ${direction} = "" ]] && continue
			([[ ${direction} != "IN" ]] && [[ ${direction} != "OUT" ]]) \
					&& ${display} RED "easyblock.conf: Error - must begin with IN/OUT: ${DEFAULT_COLOR}${direction} ${interface} ${address} ${port} ${protocol}" && continue

			# Do some creative work with variables to make building the iptables rules fairly painless
			[[ ${port} != "-" ]] && port="--dport ${port}"
			([[ ${address} != "-" ]] && [[ ${direction} == "IN" ]]) && address="-s ${address}"
			([[ ${address} != "-" ]] && [[ ${direction} == "OUT" ]]) && address="-d ${address}"
			([[ ${interface} != "-" ]] && [[ ${direction} == "IN" ]]) && interface="-i ${interface}"
			([[ ${interface} != "-" ]] && [[ ${direction} == "OUT" ]]) && interface="-o ${interface}"
			[[ ${direction} == "OUT" ]] && chain="${OutEasyBlock}"
			[[ ${direction} == "IN" ]] && chain="${InEasyBlock}"
			[[ ${protocol} != "-" ]] && protocol="-p ${protocol}"
			
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Read: ${direction} ${interface} ${address} ${port} ${protocol}"
			
			# Blank variables that we're not going to use.
			[[ ${interface} == "-" ]] && interface=""
			[[ ${port} == "-" ]] && port=""
			[[ ${address} == "-" ]] && address=""
			[[ ${protocol} == "-" ]] && protocol=""
			
			${VER_IPTABLES} -A ${chain} ${interface} ${address} ${protocol} ${port} -j DROP
			
		done < "${FWCONFIGDIR}/ipv${IPVER}/easyblock.conf"
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
	fi
}

function enable_filtering {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	if [ -e "${FWCONFIGDIR}/ipv${IPVER}/acl.conf" ]; then
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} read ${FWCONFIGDIR}/ipv${IPVER}/acl.conf successful"
		while read -r direction action interface srcaddress srcport dstaddress dstport protocol syn state custom; do
			[[ ${direction} = \#* ]] && continue
			[[ ${direction} = "" ]] && continue
			([[ ${direction} != "IN" ]] && [[ ${direction} != "OUT" ]]) \
					&& ${display} RED "acl.conf: Error - must begin with IN/OUT: ${DEFAULT_COLOR}${direction} ${action} ${interface} ${srcaddress} ${srcport} ${dstaddress} ${dstport} ${protocol} ${syn} ${state}" && continue
			([[ ${action} != "ACCEPT" ]] && [[ ${action} != "DROP" ]] && [[ ${action} != "REJECT" ]]) \
					&& ${display} RED "acl.conf: Error - action must be either ACCEPT, DROP, or REJECT : ${DEFAULT_COLOR}${direction} ${action} ${interface} ${srcaddress} ${srcport} ${dstaddress} ${dstport} ${protocol} ${syn} ${state}" && continue

			# Do some creative work with variables to make building the iptables rules fairly painless
			
			[[ -z ${state} ]] && state="-"
			([[ ${IP_VERSION} == "ipv4" ]] && [[ ${Enablev4ConnectionTracking} == "yes" ]] && [[ ! ${state} == "-" ]]) && conntrack_state="${M_STATE} ${C_STATE} ${state}"
			([[ ${IP_VERSION} == "ipv6" ]] && [[ ${Enablev6ConnectionTracking} == "yes" ]] && [[ ! ${state} == "-" ]]) && conntrack_state="${M_STATE} ${C_STATE} ${state}"
			[[ ${dstport} != "-" ]] && dstport="--dport ${dstport}"
			[[ ${srcport} != "-" ]] && srcport="--sport ${srcport}"
			[[ ${srcaddress} != "-" ]] && srcaddress="-s ${srcaddress}"
			[[ ${dstaddress} != "-" ]] && dstaddress="-d ${dstaddress}"
			([[ ${interface} != "-" ]] && [[ ${direction} == "IN" ]]) && interface="-i ${interface}"
			([[ ${interface} != "-" ]] && [[ ${direction} == "OUT" ]]) && interface="-o ${interface}"
			[[ ${direction} == "OUT" ]] && chain="${OutFilter}"
			[[ ${direction} == "IN" ]] && chain="${InFilter}"
			[[ ${protocol} != "-" ]] && protocol="-p ${protocol}"
			[[ ${action} == "REJECT" ]] && action="REJECT --reject-with tcp-reset"
			[[ ${syn} == "syn" ]] && syn="--syn"
			[[ ${syn} == "notsyn" ]] && syn="! --syn"
			
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR}${direction} ${action} ${interface} ${srcaddress} ${srcport} ${dstaddress} ${dstport} ${protocol} ${syn} ${custom}"
			
			# Blank variables that we're not going to use.
			[[ ${interface} == "-" ]] && interface=""
			[[ ${dstport} == "-" ]] && dstport=""
			[[ ${srcport} == "-" ]] && srcport=""
			[[ ${dstaddress} == "-" ]] && dstaddress=""
			[[ ${srcaddress} == "-" ]] && srcaddress=""
			[[ ${protocol} == "-" ]] && protocol=""
			[[ ${syn} == "-" ]] && syn=""
			[[ ${custom} == "-" ]] && custom=""
			
			${VER_IPTABLES} -A ${chain} ${interface} ${protocol} ${srcaddress} ${srcport} ${syn} ${dstaddress} ${dstport} ${conntrack_state} ${custom} -j ${action}
			unset direction action interface srcaddress srcport dstaddress dstport protocol syn state custom conntrack_state
		done < "${FWCONFIGDIR}/ipv${IPVER}/acl.conf"
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
	fi
}

function enable_forwarding {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	if [ -e "${FWCONFIGDIR}/ipv${IPVER}/forward.conf" ]; then
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} read ${FWCONFIGDIR}/ipv${IPVER}/forward.conf successful"
		while read -r action srcinterface srcaddress dstinterface dstaddress bidirectional srcport dstport protocol syn state custom; do
			unset conntrack_state conntrack_udp_new revsrcaddress revdstaddress revdstinterface revsrcinterface revsrcport revdstport
			[[ ${action} = \#* ]] && continue
			[[ -z ${action} ]] && continue
			([[ ${action} != "ACCEPT" ]] && [[ ${action} != "DROP" ]]) \
					&& ${display} RED "forward.conf: Error - action must be either ACCEPT or DROP : ${DEFAULT_COLOR}${action} ${srcinterface} ${srcaddress} ${dstinterface} ${dstaddress} ${bidirectional} ${srcport} ${dstport} ${protocol} ${syn} ${state}" && continue

			# Do some creative work with variables to make building the iptables rules fairly painless
			
			# Although these next few rules seems like they duplicate some work, they
			# actually make handling later rules simpler even if we end up blanking
			# them yet again.
			[[ -z ${dstport} ]] && dstport="-"
			[[ -z ${srcport} ]] && srcport="-"
			[[ -z ${protocol} ]] && protocol="-"
			[[ -z ${syn} ]] && syn="-"
			[[ -z ${state} ]] && state="-"
			
			#([[ ${IP_VERSION} == "ipv4" ]] && [[ ${Enablev4ConnectionTracking} == "yes" ]] && [[ ${state} == "-" ]]) && conntrack_state="${M_STATE} ${C_STATE} ESTABLISHED,RELATED"
			#([[ ${IP_VERSION} == "ipv6" ]] && [[ ${Enablev6ConnectionTracking} == "yes" ]] && [[ ${state} == "-" ]]) && conntrack_state="${M_STATE} ${C_STATE} ESTABLISHED,RELATED"
			([[ ${IP_VERSION} == "ipv4" ]] && [[ ${Enablev4ConnectionTracking} == "yes" ]] && [[ ! ${state} == "-" ]]) && conntrack_state="${M_STATE} ${C_STATE} ${state}"
			([[ ${IP_VERSION} == "ipv6" ]] && [[ ${Enablev6ConnectionTracking} == "yes" ]] && [[ ! ${state} == "-" ]]) && conntrack_state="${M_STATE} ${C_STATE} ${state}"
			
			([[ ${bidirectional} == "yes" ]] && [[ ${srcaddress} != "-" ]]) && revsrcaddress="-d ${srcaddress}"
			([[ ${bidirectional} == "yes" ]] && [[ ${dstaddress} != "-" ]]) && revdstaddress="-s ${dstaddress}"
			([[ ${bidirectional} == "yes" ]] && [[ ${dstinterface} != "-" ]]) && revdstinterface="-i ${dstinterface}"
			([[ ${bidirectional} == "yes" ]] && [[ ${srcinterface} != "-" ]]) && revsrcinterface="-o ${srcinterface}"
			[[ ${srcaddress} != "-" ]] && srcaddress="-s ${srcaddress}"
			[[ ${dstaddress} != "-" ]] && dstaddress="-d ${dstaddress}"
			[[ ${srcinterface} != "-" ]] && srcinterface="-i ${srcinterface}"
			[[ ${dstinterface} != "-" ]] && dstinterface="-o ${dstinterface}"
			([[ ${syn} == "syn" ]] && [[ ! -z ${conntrack_state} ]]) && conntrack_udp_new=",NEW" 
			([[ ${syn} == "syn" ]] && [[ ${protocol} == "udp" ]]) && syn="-"
			[[ ${syn} == "syn" ]] && syn="--syn"
			[[ ${syn} == "notsyn" ]] && syn="! --syn"
			[[ ${dstport} != "-" ]] && dstport="--dport ${dstport}"
			[[ ${srcport} != "-" ]] && srcport="--sport ${srcport}"
			([[ ${bidirectional} == "yes" ]] && [[ ${srcport} != "-" ]]) && revsrcport="--dport ${srcport}"
			([[ ${bidirectional} == "yes" ]] && [[ ${dstport} != "-" ]]) && revdstport="--sport ${dstport}"
			[[ ${protocol} != "-" ]] && protocol="-p ${protocol}"
			
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR}${action} ${srcinterface} ${srcaddress} ${dstinterface} ${dstaddress} ${bidirectional} ${srcport} ${dstport} ${protocol} ${syn} ${state}"
			
			# Blank variables that we're not going to use.
			[[ ${srcinterface} == "-" ]] && srcinterface=""
			[[ ${dstinterface} == "-" ]] && dstinterface=""
			[[ ${dstaddress} == "-" ]] && dstaddress=""
			[[ ${srcaddress} == "-" ]] && srcaddress=""
			[[ ${dstport} == "-" ]] && dstport=""
			[[ ${srcport} == "-" ]] && srcport=""
			[[ ${syn} == "-" ]] && syn=""
			[[ ${state} == "-" ]] && state=""
			[[ ${protocol} == "-" ]] && protocol=""
			[[ ${bidirectional} == "-" ]] && bidirectional="no"
			[[ ${custom} == "-" ]] && custom=""

			${VER_IPTABLES} -A ${FwdFilter} ${protocol} ${srcinterface} ${srcaddress} ${srcport} ${syn} ${dstinterface} ${dstaddress} ${dstport} ${conntrack_state} ${custom} -j ${action}
			[[ ${bidirectional} == "yes" ]] && ${VER_IPTABLES} -A ${FwdFilter} ${protocol} ${revsrcinterface} ${revsrcaddress} ${revsrcport} ${syn} ${revdstinterface} ${revdstaddress} ${revdstport} ${conntrack_state} ${custom} -j ${action}
			unset action srcinterface srcaddress dstinterface dstaddress bidirectional srcport dstport protocol syn state custom conntrack_state
		done < "${FWCONFIGDIR}/ipv${IPVER}/forward.conf"
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
	fi
}

function enable_nat {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	([[ ${IPVER} == "4" ]] && [[ ${Enablev4ConnectionTracking} != "yes" ]]) && ${display} RED "${FUNCNAME}: ERROR:${DEFAULT_COLOR} Unable to load NAT rules if Enablev4ConnectionTracking=no" && return 1
	([[ ${IPVER} == "6" ]] && [[ ${Enablev6ConnectionTracking} != "yes" ]]) && ${display} RED "${FUNCNAME}: ERROR:${DEFAULT_COLOR} Unable to load NAT rules if Enablev6ConnectionTracking=no" && return 1

	if [ -e "${FWCONFIGDIR}/ipv${IPVER}/nat.conf" ]; then
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} read ${FWCONFIGDIR}/ipv${IPVER}/nat.conf successful"
		while read -r type srcinterface srcaddress dstinterface dstaddress custom; do
			[[ ${type} = \#* ]] && continue
			[[ ${type} = "" ]] && continue
			([[ ${type} != "SNAT" ]] && [[ ${type} != "MASQ" ]] && [[ ${type} != "NETMAP" ]] && [[ ${type} != "ACCEPT" ]]) \
					&& ${display} RED "nat.conf: Error - must begin with SNAT/MASQ/NETMAP/ACCEPT: ${DEFAULT_COLOR}${type} ${srcinterface} ${srcaddress} ${dstinterface} ${dstaddress} ${custom}" && continue

			# Do some creative work with variables to make building the iptables rules fairly painless
			#[[ ${srcaddress} != "-" ]] && revsrcaddress="-d ${srcaddress}"
			#[[ ${dstinterface} != "-" ]] && revdstinterface="-i ${dstinterface}"
			#[[ ${srcinterface} != "-" ]] && revsrcinterface="-o ${srcinterface}"
			[[ ${srcinterface} != "-" ]] && srcinterface="-i ${srcinterface}"
			[[ ${dstinterface} != "-" ]] && dstinterface="-o ${dstinterface}"
			([[ ${srcaddress} != "-" ]] && [[ ${type} != "NETMAP" ]]) && srcaddress="-s ${srcaddress}"
			
			([[ ${dstinterface} != "-" ]] && [[ ${type} == "MASQ" ]]) && action="-j MASQUERADE" 
			([[ ${dstinterface} == "-" ]] && [[ ${type} == "MASQ" ]]) && \
					${display} RED "nat.conf: Error - MASQ rule can not have empty destination interface: ${DEFAULT_COLOR}${type} ${srcinterface} ${srcaddress} ${dstinterface} ${dstaddress}" \
					&& continue

			([[ ${dstaddress} != "-" ]] && [[ ${type} == "ACCEPT" ]]) && action="-j ACCEPT" && dstaddress="-d ${dstaddress}"
			([[ ${dstaddress} != "-" ]] && [[ ${type} == "SNAT" ]]) && action="-j SNAT" && dstaddress="--to-source ${dstaddress}"
			([[ ${dstaddress} == "-" ]] && [[ ${type} == "SNAT" ]]) && \
					${display} RED "nat.conf: Error - SNAT rule can not have empty destination address: ${DEFAULT_COLOR}${type} ${srcinterface} ${srcaddress} ${dstinterface} ${dstaddress}" \
					&& continue

			([[ ${srcaddress} != "-" ]] && [[ ${dstaddress} != "-" ]] && [[ ${type} == "NETMAP" ]]) && action="-j NETMAP" && srcaddress="-d ${srcaddress}" && dstaddress="--to ${dstaddress}"
			

			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR}${direction} ${action} ${srcinterface} ${srcaddress} ${srcport} ${dstinterface} ${dstaddress} ${dstport} ${protocol} ${custom}"
			
			# Blank variables that we're not going to use.
			[[ ${srcinterface} == "-" ]] && srcinterface=""
			[[ ${dstinterface} == "-" ]] && dstinterface=""
			[[ ${dstaddress} == "-" ]] && dstaddress=""
			[[ ${srcaddress} == "-" ]] && srcaddress=""
			[[ ${custom} == "-" ]] && custom=""

			${VER_IPTABLES} -A ${NAT} -t nat ${srcaddress} ${action} ${dstinterface} ${dstaddress} ${custom}
			#${VER_IPTABLES} -A ${FwdFilter} ${M_STATE} ${C_STATE} RELATED,ESTABLISHED,NEW ${srcinterface} ${srcaddress} ${dstinterface} -j ACCEPT
			#${VER_IPTABLES} -A ${FwdFilter} ${M_STATE} ${C_STATE} RELATED,ESTABLISHED ${revsrcinterface} ${revsrcaddress} ${revdstinterface} -j ACCEPT
			unset type srcinterface srcaddress dstinterface dstaddress custom
			done < "${FWCONFIGDIR}/ipv${IPVER}/nat.conf"
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
	fi
		
}

function enable_services {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	if [ -e "${FWCONFIGDIR}/ipv${IPVER}/services.conf" ]; then
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} read ${FWCONFIGDIR}/ipv${IPVER}/services.conf successful"
		use_conntrack="no"
		([[ ${IP_VERSION} == "ipv4" ]] && [[ ${Enablev4ConnectionTracking} == "yes" ]]) && conntrack_state="${M_STATE} ${C_STATE} NEW"
		([[ ${IP_VERSION} == "ipv6" ]] && [[ ${Enablev6ConnectionTracking} == "yes" ]]) && conntrack_state="${M_STATE} ${C_STATE} NEW"
		while read -r service protocol interface address srcaddress; do
			multiport="no"
			[[ ${service} = \#* ]] && continue
			[[ -z ${service} ]] && continue
			[[ ${service} == "-" ]] \
					&& ${display} RED "service.conf: Error - must begin with service name or port number: ${DEFAULT_COLOR}${service} ${protocol} ${interface} ${address} ${srcaddress}" && continue
			[[ ${protocol} == "-" ]] \
					&& ${display} RED "service.conf: Error - protocol can not be empty: ${DEFAULT_COLOR}${service} ${protocol} ${interface} ${address} ${srcaddress}" && continue
			[[ ${service} =~ "," ]] && multiport="yes"
			# Do some creative work with variables to make building the iptables rules fairly painless
			([[ ${service} != "-" ]] && [[ ${multiport} != "yes" ]]) && service="--dport ${service}"
			([[ ${service} != "-" ]] && [[ ${multiport} == "yes" ]]) && service="-m multiport --dports ${service}"
			[[ ${protocol} != "-" ]] && protocol="-p ${protocol}"
			[[ ${interface} != "-" ]] && interface="-i ${interface}"
			[[ ${address} != "-" ]] && srcaddress="-d ${address}"
			[[ ${srcaddress} != "-" ]] && srcaddress="-s ${srcaddress}"
			
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Read: ${service} ${protocol} ${interface} ${address} ${srcaddress}"
			
			# Blank variables that we're not going to use.
			[[ ${interface} == "-" ]] && interface=""
			[[ ${address} == "-" ]] && address=""
			[[ ${srcaddress} == "-" ]] && srcaddress=""
			
			${VER_IPTABLES} -A ${InFilter} ${protocol} ${service} ${interface} ${address} ${srcaddress} ${conntrack_state} -j ACCEPT
			unset service protocol interface address srcaddress conntrack_state
		done < "${FWCONFIGDIR}/ipv${IPVER}/services.conf"
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
		unset service protocol interface address srcaddress
	fi
}

function enable_conntrack_int {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	conntrack_int="$2"
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	if [[ ${conntrack_int} == "all" ]]; then
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Enabling conntrack on all interfaces"
		${VER_IPTABLES} -A ${OutPreRules} ${M_STATE} ${C_STATE} ESTABLISHED,RELATED -j ACCEPT
		${VER_IPTABLES} -A ${InPreRules} ${M_STATE} ${C_STATE} ESTABLISHED,RELATED -j ACCEPT
		${VER_IPTABLES} -A ${OutPreRules} ${M_STATE} ${C_STATE} INVALID -j DROP
		${VER_IPTABLES} -A ${InPreRules} ${M_STATE} ${C_STATE} INVALID -j DROP
	else
		for i in ${conntrack_int}; do
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Enabling conntrack on ${i}"
			${VER_IPTABLES} -A ${OutPreRules} -o ${i} ${M_STATE} ${C_STATE} ESTABLISHED,RELATED -j ACCEPT
			${VER_IPTABLES} -A ${InPreRules} -i ${i} ${M_STATE} ${C_STATE} ESTABLISHED,RELATED -j ACCEPT
			${VER_IPTABLES} -A ${OutPreRules} -o ${i} ${M_STATE} ${C_STATE} INVALID -j DROP
			${VER_IPTABLES} -A ${InPreRules} -i ${i} ${M_STATE} ${C_STATE} INVALID -j DROP
		done
	fi
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
}

function enable_portfw {
	IP_VERSION=$1
		case $IP_VERSION in
		ipv6) VER_IPTABLES=${IP6TABLES};
				IPVER="6" ;;
		ipv4|*) VER_IPTABLES=${IPTABLES}
				IPVER="4" ;;
	esac
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	if [ -e "${FWCONFIGDIR}/ipv${IPVER}/portfw.conf" ]; then
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} read ${FWCONFIGDIR}/ipv${IPVER}/portfw.conf successful"
		use_conntrack="no"
		([[ ${IP_VERSION} == "ipv4" ]] && [[ ${Enablev4ConnectionTracking} == "yes" ]]) && conntrack_state="${M_STATE} ${C_STATE} NEW"
		([[ ${IP_VERSION} == "ipv6" ]] && [[ ${Enablev6ConnectionTracking} == "yes" ]]) && conntrack_state="${M_STATE} ${C_STATE} NEW"
		while read -r service protocol intip intport interface address srcaddress; do
			[[ ${service} = \#* ]] && continue
			[[ -z ${service} ]] && continue
			[[ ${service} == "-" ]] \
					&& ${display} RED "service.conf: Error - must begin with service name or port number: ${DEFAULT_COLOR}${service} ${intip} ${intport} ${protocol} ${interface} ${address} ${srcaddress}" && continue
			[[ ${protocol} == "-" ]] \
					&& ${display} RED "service.conf: Error - protocol can not be empty: ${DEFAULT_COLOR}${service} ${intip} ${intport} ${protocol} ${interface} ${address} ${srcaddress}" && continue

			# Do some creative work with variables to make building the iptables rules fairly painless

			# Although these next few rules seems like they duplicate some work, they
			# actually make handling later rules simpler even if we end up blanking
			# them yet again.
			[[ -z ${interface} ]] && interface="-"
			[[ -z ${address} ]] && address="-"
			[[ -z ${srcaddress} ]] && srcaddress="-"

			[[ ${service} != "-" ]]  && service="--dport ${service}"
			[[ ${protocol} != "-" ]] && protocol="-p ${protocol}"
			[[ ${intip} != "-" ]] && intdest="--to-destination ${intip}:${intport}"
			[[ ${interface} != "-" ]] && interface="-i ${interface}"
			[[ ${intip} != "-" ]] && intip="-d ${intip}"
			[[ ${intport} != "-" ]] && intport="--dport ${intport}"
			[[ ${address} != "-" ]] && address="-d ${address}"
			[[ ${srcaddress} != "-" ]] && srcaddress="-s ${srcaddress}"
			
			${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} Read: ${service} ${protocol} ${intip} ${intport} ${interface} ${address} ${srcaddress}"
			
			# Blank variables that we're not going to use.
			[[ ${interface} == "-" ]] && interface=""
			[[ ${address} == "-" ]] && address=""
			[[ ${srcaddress} == "-" ]] && srcaddress=""
			
			${VER_IPTABLES} -A ${PortForward} -t nat ${protocol} ${service} ${interface} ${address} ${srcaddress} -j DNAT ${intdest}
			${VER_IPTABLES} -A ${FwdFilter} ${interface} ${intip} ${protocol} ${intport} ${srcaddress} ${conntrack_state} -j ACCEPT
			unset service protocol intip intport interface address srcaddress conntrack_state
		done < "${FWCONFIGDIR}/ipv${IPVER}/portfw.conf"
		${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} done"
	fi
}

function enable_v6_critical_icmp {
	VER_IPTABLES=${IP6TABLES}
	${debug} ${DebugColor} "${FUNCNAME}:${DEFAULT_COLOR} loading"
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 1 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 2 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 3 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 4 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 133 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 134 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 135 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 136 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 137 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 141 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 142 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -s fe80::/10 -p ipv6-icmp --icmpv6-type 130 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -s fe80::/10 -p ipv6-icmp --icmpv6-type 131 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -s fe80::/10 -p ipv6-icmp --icmpv6-type 132 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -s fe80::/10 -p ipv6-icmp --icmpv6-type 143 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 148 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -p ipv6-icmp --icmpv6-type 149 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -s fe80::/10 -p ipv6-icmp --icmpv6-type 151 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -s fe80::/10 -p ipv6-icmp --icmpv6-type 152 -j ACCEPT
	${VER_IPTABLES} -A ${v6ICMP} -s fe80::/10 -p ipv6-icmp --icmpv6-type 153 -j ACCEPT
}

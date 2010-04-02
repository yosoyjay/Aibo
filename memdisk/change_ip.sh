#!/bin/sh -

NET_CONF_FILE="./OPEN-R/SYSTEM/CONF/WLANDFLT.TXT"

if [ -z $1 ]
then
	echo "usage: $0 <IP ADDRESS>"
else
	sed -e "s/ETHER_IP=.*$/ETHER_IP=$1/" < $NET_CONF_FILE > $NET_CONF_FILE.TMP
	mv $NET_CONF_FILE.TMP $NET_CONF_FILE
	echo "Aibo successfully configured with IP address: $1"
fi

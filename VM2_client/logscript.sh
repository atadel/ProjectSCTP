#!/bin/bash
CONFIG_FILE="plik.conf"
BRYSIA_CLIENT="./brysiaklient"
LOG_FILE="/var/log/local7"

IP_ADDRESS=$(grep -Eo 'server_ip\s*=\s*([0-9]{1,3}\.){3}[0-9]{1,3}' "$CONFIG_FILE" | awk -F= '{print $2}' | tr -d '[:space:]')
"$BRYSIA_CLIENT" "$IP_ADDRESS"

#tail -n0 -F "$LOG_FILE" | while read -r line; do
	#IP_ADDRESS=$(cat "$CONFIG_FILE" | grep 'server_ip' | grep "(\d+\.){3}\d+")
	#IP_ADDRESS=$(grep -Eo 'server_ip\s*=\s*([0-9]{1,3}\.){3}[0-9]{1,3}' "$CONFIG_FILE" | awk -F= '{print $2}' | tr -d '[:space:]')
    	#echo "$line" | "$BRYSIA_CLIENT" "$IP_ADDRESS" 
#	echo "$line"
#	echo  "moje ip: $IP_ADDRESS" 
#done

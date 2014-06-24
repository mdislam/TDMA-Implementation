#!/bin/bash

clear

gcc -o udpclient udpclient.c
#gcc -o myclient myclient.c

# echo "researcher123" | sudo -S tcpdump -i wlan0 port 9930 -w client_data.pcap

# number of packets 600

#sleep 10s
./udpclient 10.0.2.1 50 100
#sleep 5s
./udpclient 10.0.2.1 50 500
#sleep 5s
./udpclient 10.0.2.1 50 1000
#sleep 5s
./udpclient 10.0.2.1 50 5000
#sleep 5s
./udpclient 10.0.2.1 50 8000
#sleep 5s
./udpclient 10.0.2.1 50 10000
#sleep 5s
./udpclient 10.0.2.1 50 50000
#sleep 5s
./udpclient 10.0.2.1 50 80000
#sleep 5s
./udpclient 10.0.2.1 50 100000
#sleep 5s
./udpclient 10.0.2.1 50 500000
#sleep 5s
./udpclient 10.0.2.1 50 800000
#sleep 5s
./udpclient 10.0.2.1 50 1000000

# \x03

#scp client_data.pcap mdislam@hpf-19.cs.helsinki.fi:experiment/client_data.pcap

#todate=$(date +"%H%M%S%3N")

#while [ $todate -lt $1 ]
#do
#	todate=$(date +"%H%M%S%3N");
#done
#desired=`expr $1 + $2`
#./myclient 10.0.0.1 $desired 0 100 > client01.txt

echo "Done!"

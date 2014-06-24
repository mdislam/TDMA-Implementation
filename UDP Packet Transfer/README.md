UDP-Packet-Transfer
===================

A simple Server and Client communication by transferring UDP packet has been implemented in the same host machine. The purpose of this implementation is to record different time-stamps such as UDP packet sending time-stamp, receiving time-stamp and measure packet transfer time and delay overheads from these records.

Experimental Data
===================

log_data.txt file records all time stamps and calculated transfer time.

log.txt file records the sleep overhead

The excel file plots the data into graph. There are two sheets, one is for graph and another is for sleep delay overhead.

Command
==================

command for server: ./udpserver

command for client: ./udpclient 127.0.0.1 50 10000

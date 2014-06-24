TDMA-Implementation
===================

This is a Network program in C. There are two clients and One TDMA server.

Implementation
==============

In the server there are two instances of server run simultaniously, one is broadcast server - broadcast beacon packet over ethernet and another is dataserver - waiting for data from wireless medium.

At first the broadcast server sends beacon packets for clients periodically (after 100 milliseconds). Clients are waiting for their beacon packet. When a client gets it's own beacon packet, it starts uploading data to the dataserver for his time slot duration, which is 100 milliseconds. Each client sends data for 94 milliseconds and then waiting for next beacon packet. Here the guard time is 6 millisecods, which is minimum

I am using harware access point and the hardware access point is acting as the servers.

Observations
============

Data (packets) is uploading in the server as desired. Fairness achieved. Each client sends data for same slot duration and the number of packets recieved from each client is almost same. There are some duplicate packet counts which is around 3 or 4 packets.

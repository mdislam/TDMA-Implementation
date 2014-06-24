#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFLEN 600
#define PORT 9930
#define MAX 255
#define SERVER_IP "10.0.2.1"

// packet structure
struct UdpPacket {
	int pck_id;
	long send_time_sec;
	long send_time_usec;
	long sleep_time;
};

/* this function used for error handling */
void diep(char *s) {
	perror(s);
	exit(1);
}

int main() {
	int sfd, n, i;
	socklen_t len;
	char line[BUFLEN];
	struct UdpPacket *pck;
	struct timeval end;
	
	int pack_count = 0;
	int pack_id[BUFLEN];
	long send_ts_sec[BUFLEN];
	long send_ts_usec[BUFLEN];
	long recv_ts_sec[BUFLEN];
	long recv_ts_usec[BUFLEN];
	// long double tx_time[BUFLEN];
	long sleep_interval[BUFLEN];
	// long double sleep_interval_server[BUFLEN];
	
	long double curr_rx_ts, prev_rx_ts, pck_tx_time, sleep_t_server_cal;
	
	FILE *file; // file for recording packet information
	
	/* 
	 * socketaddr_in is a structure containing an Internet socket address.
	 * It contains: an address family, a port number, an IP address
	 * server will listen in 'saddr' socket and client socket is 'caddr'
	 */
	struct sockaddr_in saddr, caddr;
	
	/* 
	 * Create a socket.
	 * AF_INET says that it will be an Internet socket.
	 * SOCK_DGRAM says that it will use datagram delivery instead of virtual circuits.
	 * IPPROTO_UDP says that it will use the UDP protocol
	 */
	if((sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		diep("socket");
	
	// Initialize saddr strucure, filling with binary zeros
	bzero(&saddr, sizeof(saddr));
	
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT); // htons() ensures that the byte order is correct (Host TO Network order/Short integer)
	saddr.sin_addr.s_addr = htonl(INADDR_ANY); // ??
	
	// the socket sfd should be bound to the address in saddr.
	if(bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
		diep("bind");

	// printf("Server Running... ...\n");
	
	file = fopen("log_data.txt", "w");
	fprintf(file, "ID\tSlp_T\tSend_TS\t\t\t\tRECV_TS\n");
	fclose(file);

	len = sizeof(caddr);
	curr_rx_ts = prev_rx_ts = 0;
	
	pck  = malloc(sizeof(struct UdpPacket));

	printf("Server is Running... ...\n");
	
	/*
	 * Receive a packet from sfd, that the data should be put into line
	 * line can store at most BUFLEN characters
	 * The zero parameter says that no special flags should be used
	 * Data about the sender should be stored in caddr, which has room for len byte
	 */
	
	long double packet_ts;
	while(pack_count < BUFLEN){
	
		if(recvfrom(sfd, line, sizeof(struct UdpPacket), 0, (struct sockaddr *)&caddr, &len) == -1)
			diep("recvfrom()");
		
		// getting the receiveing timestamp
		gettimeofday(&end, NULL);
		
		curr_rx_ts = end.tv_usec / 1000000 + end.tv_sec;

		
		pck = (struct UdpPacket *)line;
		
		
		// calculating the transfer time of a packet
		//packet_ts = pck->send_time_usec / 1000000 + pck->send_time_sec;
		//pck_tx_time = curr_rx_ts - packet_ts;
		//pck_tx_time = ((end.tv_sec * 1000000 + end.tv_usec) - pck->send_time);
		//if(prev_rx_ts == 0)
		  //sleep_t_server_cal = pck->sleep_time;
		//else
		  //sleep_t_server_cal = curr_rx_ts - prev_rx_ts - pck_tx_time;
		
		// information storing in array
		pack_id[pack_count] = pck->pck_id;
		send_ts_sec[pack_count] = pck->send_time_sec;
		send_ts_usec[pack_count] = pck->send_time_usec;
		recv_ts_sec[pack_count] = end.tv_sec;
		recv_ts_usec[pack_count] = end.tv_usec;
		//tx_time[pack_count] = pck_tx_time;
		sleep_interval[pack_count] = pck->sleep_time;
		//sleep_interval_server[pack_count] = sleep_t_server_cal;
		
		printf("Received packet %d from %s:%d\n", pck->pck_id, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
		//printf("Packet ID: %d, Sending TS: %ld, Transfer time: %ld, sleep time: %ld\n", pck->pck_id, pck->send_time, pck_tx_time, sleep_t_server_cal);
		
		prev_rx_ts = curr_rx_ts;
		
		pack_count++;
	}
	
	int count;
	
	if((file = fopen("log_data.txt", "a")) == NULL){
	  printf("\nUnable to open file log_data.txt");
	  exit(1);
	}
    
	//fwrite(pack_id, sizeof(pack_id), 1, file);

	printf("\nLoop Complete\n");
    
	for(count = 0; count < BUFLEN; count++) {
		fprintf(file, "%d\t%ld\t%ld,%ld\t%ld,%ld\n", pack_id[count], sleep_interval[count], send_ts_sec[count], send_ts_usec[count], recv_ts_sec[count], recv_ts_usec[count]);
	}
	
	printf("File write complete!\n");	
	//free(pck);
	
	fclose(file);
	close(sfd);
	return 0;
}

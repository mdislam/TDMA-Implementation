#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFLEN 8192
#define PORT 4690
#define SERVER_IP "10.0.2.1"

// packet structure
struct UdpPacket {
	int pck_id;
	int client_id;
	long send_time_sec;
	long send_time_usec;
	//char dt_arr[1024];
};

struct TDMAControlPacket {
	int client_id;
	//int offset;
	//int slot_duration;
	//int round_time;
};

/* this function used for error handling */
void diep(char *s) {
	perror(s);
	exit(1);
}

int main() {
	int sfd;
	socklen_t len;
	char line[BUFLEN];
	struct UdpPacket *pck;
	struct timeval end;
	
	int pack_count = 0;
	int pack_id[BUFLEN];
	int client_id[BUFLEN];
	long send_ts_sec[BUFLEN];
	long send_ts_usec[BUFLEN];
	long recv_ts_sec[BUFLEN];
	long recv_ts_usec[BUFLEN];
	
	long double curr_rx_ts, prev_rx_ts;
	
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
	
	if (inet_aton(SERVER_IP, &caddr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed.\n");
		exit(1);
	}
	
	// the socket sfd should be bound to the address in saddr.
	if(bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
		diep("bind");
	
	file = fopen("log_data.txt", "w");
	fprintf(file, "Client\tPCKNO\tSend_TS\t\t\t\tRECV_TS\n");
	fclose(file);

	len = sizeof(caddr);
	curr_rx_ts = prev_rx_ts = 0;
	
	pck  = malloc(sizeof(struct UdpPacket));

	printf("Server is waiting for data... ...\n");
	
		
	while(pack_count < BUFLEN){
		/*
		* Receive a packet from sfd, that the data should be put into line
		* line can store at most BUFLEN characters
		* The zero parameter says that no special flags should be used
		* Data about the sender should be stored in caddr, which has room for len byte
		*/
		printf("I am here \n");
		if(recvfrom(sfd, line, sizeof(struct UdpPacket), 0, (struct sockaddr *)&caddr, &len) == -1)
			diep("recvfrom()");
		
		// getting the receiveing timestamp
		gettimeofday(&end, NULL);
		
		curr_rx_ts = end.tv_usec / 1000000 + end.tv_sec;
		
		pck = (struct UdpPacket *)line;
		
		// information storing in array
		pack_id[pack_count] = pck->pck_id;
		client_id[pack_count] = pck->client_id;
		send_ts_sec[pack_count] = pck->send_time_sec;
		send_ts_usec[pack_count] = pck->send_time_usec;
		recv_ts_sec[pack_count] = end.tv_sec;
		recv_ts_usec[pack_count] = end.tv_usec;
		
		printf("Received packet %d from Client %d - %s:%d\n", pck->pck_id, pck->client_id, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
		
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
		fprintf(file, "%d\t%d\t%ld,%ld\t%ld,%ld\n", client_id[count], pack_id[count], send_ts_sec[count], send_ts_usec[count], recv_ts_sec[count], recv_ts_usec[count]);
	}
	
	printf("File write complete!\n");	
	
	fclose(file);
	close(sfd);
	return 0;
}

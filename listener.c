#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#define BUFLEN 4096
#define PORT 9930 // broadcast port
#define DTPORT 4690 // port for upload data
#define SERVER_IP "10.0.2.1"

struct TDMAControlPacket {
	int client_id;
	//int offset;
	//int slot_duration;
	//int round_time;
};

struct UdpPacket {
	int pck_id;
	int client_id;
	long send_time_sec;
	long send_time_usec;
};

/* this function used for error handling */
void diep(char *s) {
	perror(s);
	exit(1);
}

int main() {
	int sfd, dtsfd;
	socklen_t len;
	char line[BUFLEN];
	struct TDMAControlPacket *pck;
	
	int pack_count = 0;
	int numbytes;
	
	long proc_time;
	struct timeval start, end, val1, val2;
	
	/* 
	 * socketaddr_in is a structure containing an Internet socket address.
	 * It contains: an address family, a port number, an IP address
	 * client will listen in 'caddr' socket and server socket is 'saddr'
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
	
	// Initialize caddr strucure, filling with binary zeros
	bzero(&caddr, sizeof(caddr));
	
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(PORT); // htons() ensures that the byte order is correct (Host TO Network order/Short integer)
	caddr.sin_addr.s_addr = htonl(INADDR_ANY); // ??
	
	// the socket sfd should be bound to the address in caddr.
	if(bind(sfd, (struct sockaddr *)&caddr, sizeof(caddr)) == -1)
		diep("bind");

	len = sizeof(caddr);
	
	pck  = malloc(sizeof(struct TDMAControlPacket));

	printf("listener: waiting to recvfrom...\n");
	
	int pck_no = 1;
	
	while(1) {
		/*
		* Receive a packet from sfd, that the data should be put into line
		* line can store at most BUFLEN characters
		* The zero parameter says that no special flags should be used
		* Data about the sender should be stored in saddr, which has room for len byte
		*/
		if ((numbytes = recvfrom(sfd, line, sizeof(struct TDMAControlPacket), 0, (struct sockaddr *)&saddr, &len)) == -1) {
			diep("recvfrom()");
		}

		gettimeofday(&val2, NULL);
		
		pck = (struct TDMAControlPacket *)line;
		
		printf("Received from %s:%d\n", inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));
		printf("listener: packet is %d bytes long\n", numbytes);
		line[numbytes] = '\0';
		
		// if for client 02 then change following if condition
		if(pck->client_id == 1) {
			printf("YAHOO! It's My Turn :-), %ld, %ld .\n\n", val2.tv_sec, val2.tv_usec);
			
			struct sockaddr_in dtaddr;
			
			if((dtsfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
			      diep("socket");
			
			bzero(&dtaddr, sizeof(dtaddr));	
			dtaddr.sin_family = AF_INET;	
			dtaddr.sin_port = htons(DTPORT);
			
			if (inet_aton(SERVER_IP, &dtaddr.sin_addr) == 0)
			      diep("data socket");
			
			gettimeofday(&start, NULL);
			
			proc_time = 0;
			
			while(proc_time < 94000) {
			    struct UdpPacket data;
			    bzero(&data, sizeof(data));
			    
			    gettimeofday(&val1, NULL);
			    
			    data.pck_id = pck_no;
			    data.client_id = pck->client_id;
			    data.send_time_sec = val1.tv_sec;
			    data.send_time_usec = val1.tv_usec;
			    
			    if(sendto(dtsfd, (const char *)&data, sizeof(data), 0, (struct sockaddr *)&dtaddr, sizeof dtaddr) == -1) {
				    perror("sendto");
				    exit(1);
			    }
			    
			    gettimeofday(&end, NULL);
			    proc_time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
			    
			    printf("Client %d: Sending packet %d to %s:%d\n", data.client_id, data.pck_id,  inet_ntoa(dtaddr.sin_addr), ntohs(dtaddr.sin_port));
			    
			    pck_no++;
			}
			
			close(dtsfd);
		}
		else if(pck->client_id == 2)
		  printf("Waiting for Beacon.\n\n");
		else
		  printf("Unknown Value \"%s\"\n\n", line);		
	}

	close(sfd);

	return 0;
}

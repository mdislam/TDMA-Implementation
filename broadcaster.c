#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#define PORT 9930 // broadcast port

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

// command line parameter should be num of packets and sleep time
int main(int argc, char **argv) {
	int sfd, n, i;
	int broadcast = 1;
	int numbytes;
	socklen_t len;
	long proc_time;
	struct timeval start, end, val1;
	
	/* 
	 * socketaddr_in is a structure containing an Internet socket address.
	 * It contains: an address family, a port number, an IP address
	 */
	struct sockaddr_in caddr;

	
	if(argc != 2) {
		printf("Usage: %s HostAddress\n", argv[0]);
		return -1;
	}
	
	/* 
	 * Create a socket.
	 * AF_INET says that it will be an Internet socket.
	 * SOCK_DGRAM says that it will use datagram delivery instead of virtual circuits.
	 * IPPROTO_UDP says that it will use the UDP protocol
	 */
	if((sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		diep("socket");
	}
	
	// this call is what allows broadcast packets to be sent:
	if (setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
		diep("broadcast");
	}
	
	// Initialize caddr strucure, filling with binary zeros
	bzero(&caddr, sizeof(caddr));
	
	caddr.sin_family = AF_INET;	
	caddr.sin_port = htons(PORT); // htons() ensures that the byte order is correct (Host TO Network order/Short integer)

	if (inet_aton(argv[1], &caddr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed.\n");
		exit(1);
	}
	
	int client_id = 2;
	
	// broadcasting becaon packet
	while(1) {
		struct TDMAControlPacket pck;
		bzero(&pck, sizeof(pck));
		
		len = sizeof(caddr);		
		
		if(client_id == 2) {
		  client_id = 1;
		  pck.client_id = client_id;
		}
		else {
		  client_id = 2;
		  pck.client_id = client_id;
		}
		
		/*
		 * Send sizeof(pck) bytes from pck to sfd, with no flags (0).
		 * The receiver is specified in caddr, which contains len byte. 
		 */
		if ((numbytes = sendto(sfd, (const char *)&pck, sizeof(pck), 0, (struct sockaddr *)&caddr, len)) == -1) {
			diep("sendto()");
		}
		gettimeofday(&start, NULL);
		
		printf("Sending Control to Client %d, time: %ld,%ld\n", pck.client_id, start.tv_sec, start.tv_usec);
		
		usleep(100000); // sleep for 100 millisecs
	}

	

	printf("sent %d bytes to %s\n", numbytes, inet_ntoa(caddr.sin_addr));
	close(sfd);

	return 0;
}

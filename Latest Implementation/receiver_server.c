#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define BUFLEN 4196
#define BLCK_LEN 512
#define DTPORT 4690 // port for upload data
#define SERVER_IP "10.0.0.250"

int sfd;

// packet structure
struct TDMAControlPacket {
    int type;
    char id[18];
    int slot_duration;
};

struct UdpPacket {
    int pck_id;
    char id[18];
    char dtbuf[BLCK_LEN + 1];
    long send_time_sec;
    long send_time_usec;
    int end_of_transmission;
};

/* this function used for error handling */
void diep(char *s) {
    perror(s);
    close(sfd);
    exit(1);
}

// this function creates a socket and return the file descriptor
int create_socket(int domain, int type, int protocol, int isBroadcast) {
    int sfd;
    int broadcast = 1;
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 500000;
    if((sfd = socket(domain, type, protocol)) == -1)
    diep("socket");
    
    if(setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        diep("timeout");
    }
    
    if(isBroadcast) {
        if (setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1)
        diep("broadcast");
    }
    
    return sfd;
}

struct sockaddr_in manage_sockaddr_in_recv(int port) {
    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr)); //initialize
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    return saddr;
}

int main() {
    //int sfd;
    socklen_t len;
    char line[BUFLEN];
    struct UdpPacket *pck;
    struct timeval end;
    
    int pack_count = 0;
    
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
    sfd = create_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0);
    
    saddr = manage_sockaddr_in_recv(DTPORT);
    
    // the socket sfd should be bound to the address in saddr.
    if(bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    diep("bind");
    
    
    len = sizeof(caddr);
    curr_rx_ts = prev_rx_ts = 0;
    
    pck  = malloc(sizeof(struct UdpPacket));
    
    printf("Server is waiting for data... ...n");
    
    char recvbuf[BLCK_LEN + 1];
    
    int success = 0;
    int pck_recv_count = 0;
    int pck_recv_total_count = 0;
    
    while(success == 0){
        bzero(recvbuf, BLCK_LEN + 1);
        int fr_block_sz, i = 0;
        int new_id, old_id = -1;
        
        while(1) {
            if((fr_block_sz = recvfrom(sfd, line, sizeof(struct UdpPacket), 0, (struct sockaddr *)&caddr, &len)) == -1)
            diep("recvfrom()");
            
            // getting the receiveing timestamp
            gettimeofday(&end, NULL);
            
            pck = (struct UdpPacket *)line;
            
            fr_block_sz = sizeof(pck->dtbuf);
            
            if(fr_block_sz == 0) break;
            new_id = pck->pck_id;
            if(new_id != old_id && pck->end_of_transmission == 0) {
                pck_recv_count++;
                old_id = new_id;
            }
            else if(new_id == old_id && pck->end_of_transmission == 0) {
                //printf("DUP | %s | %d | %ld,%ld | %ld,%ldn", pck->id, pck->pck_id, pck->send_time_sec, pck->send_time_usec, end.tv_sec, end.tv_usec);
                ;
            }
            
            else if(pck->end_of_transmission == 9) {
                //printf("%s | %d | %d | %ld,%ld | %ld,%ldn", pck->id, pack_count, pck_recv_count, pck->send_time_sec, pck->send_time_usec, end.tv_sec, end.tv_usec);
                printf("%s | %d | %d | %dn", pck->id, pack_count, pck_recv_count, pck_recv_total_count - pck_recv_count);
                pck_recv_count = 0;
                pck_recv_total_count = 0;
            }
            i++;
            pack_count++;
            pck_recv_total_count++;
        }
        if(fr_block_sz < 0) {
            if (errno == EAGAIN) printf("recv() timed out.n");
            else diep("recv failed:");
        }
        printf("OK recieved from client!n");
        success = 1;
        
    }
    close(sfd);
    return 0;
}
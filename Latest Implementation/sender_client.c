#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <linux/if.h>

#define BUFLEN 4196
#define BLCK_LEN 512
#define TOTAL_DATA_SIZE 100

#define BPORT 9930 // use for broadcasting
#define JPORT 9090 // server is listening the join request
#define DTPORT 4690 // port for upload data
#define SERVER_IP "10.0.0.250"
#define FILE_NAME "experiment_data.txt"

#define GUARD 0

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

// place some possible error checking here !!!
struct UdpPacket set_tdma_packet(int pck_no, char *databuffer, char *id, int end_tx, struct timeval ts) {
    struct UdpPacket pck;
    bzero(&pck, sizeof(pck));
    
    pck.pck_id = pck_no;
    strcpy(pck.id, id);
    strcpy(pck.dtbuf, databuffer);
    pck.send_time_sec = ts.tv_sec;
    pck.send_time_usec = ts.tv_usec;
    pck.end_of_transmission = end_tx;
    
    return pck;
}


/* this function used for error handling */
void diep(char *s) {
    perror(s);
    exit(1);
}

// this function creates a socket and return the file descriptor
int create_socket(int domain, int type, int protocol, int isBroadcast) {
    int sfd;
    int broadcast = 1;
    if((sfd = socket(domain, type, protocol)) == -1)
    diep("socket");
    
    if(isBroadcast) {
        if (setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1)
        diep("broadcast");
    }
    
    return sfd;
}

struct sockaddr_in manage_sockaddr_in_send(char *ip_address, int port) {
    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr)); //initialize
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port); // htons() ensures that the byte order is correct (Host TO Network order/Short integer)
    if(inet_aton(ip_address, &saddr.sin_addr.s_addr) == 0)
    diep("inet_aton");
    
    return saddr;
}

struct sockaddr_in manage_sockaddr_in_recv(int port) {
    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr)); //initialize
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    return saddr;
}

// get the MAC address
static char *interface_mactoa() {
    static char buff[256];
    
    struct ifreq s;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    
    strcpy(s.ifr_name, "wlan0");
    if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
        unsigned char *ptr = (unsigned char*) s.ifr_addr.sa_data;
        
        sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
        (ptr[0] & 0xff), (ptr[1] & 0xff), (ptr[2] & 0xff),
        (ptr[3] & 0xff), (ptr[4] & 0xff), (ptr[5] & 0xff));
    }
    
    return (buff);
}

int main() {
    int sfd;
    socklen_t len;
    char line[BUFLEN];
    struct TDMAControlPacket *pck;
    
    int pack_count;
    int numbytes;
    int slot_time;
    int join = 0;
    int quit = 0;
    char mac_addr[18];
    memset(&mac_addr, '', 18);
    
    long proc_time;
    struct timeval start, end;
    
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
    sfd = create_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0);
    
    caddr = manage_sockaddr_in_recv(BPORT);
    
    // the socket sfd should be bound to the address in caddr.
    if(bind(sfd, (struct sockaddr *)&caddr, sizeof(caddr)) == -1)
    diep("bind");
    
    len = sizeof(caddr);
    
    pck  = malloc(sizeof(struct TDMAControlPacket));
    
    printf("listener: waiting to recvfrom...nn");
    
    int pck_no = 1;
    int count = 0;
    
    // Creating data of size BLCK_LEN
    char sdbuf[BLCK_LEN];
    bzero(sdbuf, BLCK_LEN + 1);
    int newChar;
    long dataSize;
    for(newChar = 0; newChar < BLCK_LEN; newChar++) {
        sdbuf[newChar] = 'a';
    }
    
    dataSize = 0;
    
    struct UdpPacket data;
    data = set_tdma_packet(pck_no, sdbuf, mac_addr, 0, end);
    
    //printf("%sn", sdbuf);
    //printf("%dn", sizeof(sdbuf));
    
    while(1) {
        /*
        * Receive a packet from sfd, that the data should be put into line
        * line can store at most BUFLEN characters
        * The zero parameter says that no special flags should be used
        * Data about the sender should be stored in saddr, which has room for len byte
        */
        if ((numbytes = recvfrom(sfd, line, sizeof(struct TDMAControlPacket), 0, (struct sockaddr *)&saddr, &len)) == -1)
        diep("recvfrom()");
        
        gettimeofday(&start, NULL);
        
        pck = (struct TDMAControlPacket *)line;
        
        slot_time = pck->slot_duration;
        line[numbytes] = '';
        
        strcpy(mac_addr, interface_mactoa());
        
        gettimeofday(&start, NULL);
        
        if(pck->type == 0 && join == 0) {
            struct UdpPacket jpck;
            
            jpck = set_tdma_packet(pck_no, "JOIN", mac_addr, 0, start);
            
            caddr = manage_sockaddr_in_send(SERVER_IP, JPORT);
            len = sizeof(caddr);
            if(sendto(sfd, (const char *)&jpck, sizeof(jpck), 0, (struct sockaddr *)&caddr, len) == -1)
            diep("send");
            
            join = 1;
        }
        else if(pck->type == 1 && join == 1 && strcmp(pck->id, mac_addr) == 0) {
            caddr = manage_sockaddr_in_send(SERVER_IP, DTPORT);
            len = sizeof(caddr);
            
            proc_time = 0;
            pack_count = 0;
            
            /*Send Data to Server*/
            while(dataSize < TOTAL_DATA_SIZE * 1024 * 1024) {
                gettimeofday(&end, NULL);
                proc_time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
                if(proc_time >= (slot_time - GUARD))
                break;
                
                data.pck_id = pck_no;
                data.send_time_sec = end.tv_sec;
                data.send_time_usec = end.tv_usec;
                
                if(sendto(sfd, (const char *)&data, sizeof(data), 0, (struct sockaddr *)&caddr, len) == -1)
                diep("sendto FILE:");
                dataSize += sizeof(sdbuf);
                
                pck_no++;
                pack_count++;
            }
            
            struct UdpPacket dataTotal;
            dataTotal = set_tdma_packet(pack_count, "TOTAL", mac_addr, 9, end);
            
            if(sendto(sfd, (const char *)&dataTotal, sizeof(dataTotal), 0, (struct sockaddr *)&caddr, len) == -1)
            diep("sendto FILE:");
            
            printf("Finished | %d | %d | %ld | %ld,%ldn", pck_no, pack_count, dataSize, end.tv_sec, end.tv_usec);
            
            caddr = manage_sockaddr_in_send(SERVER_IP, JPORT);
            len = sizeof(caddr);
            
            struct UdpPacket end_data;
            end_data = set_tdma_packet(0, "END_TX", mac_addr, 1, end);
            
            if(sendto(sfd, (const char *)&end_data, sizeof(end_data), 0, (struct sockaddr *)&caddr, len) == -1)
            diep("sendto");
            
            if(dataSize >= TOTAL_DATA_SIZE * 1024 * 1024) quit = 1;
        }
        else if(pck->type == 1 && join == 1 && strcmp(pck->id, mac_addr) != 0) ;
        //printf("Waiting for Beacon|||n");
        else if(pck->type == 0 && join == 1) ;
        //printf("Contention Slot|||n");
        else if(pck->type == 1 && join == 0) ;
        //printf("Wrong Entry|||n");
        else ;
        //printf("Don't Know what is happening|||n");
        
        if(quit == 1) break;
    }
    close(sfd);
    
    return 0;
}
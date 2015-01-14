#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#define BPORT 9930 // use for broadcasting
#define JPORT 9090 // server is listening the join request

#define BUFLEN 4196
#define BLCK_LEN 512
#define MAXCLIENT 10
#define TIME_FRAME 0.3 // in seconds
#define CONTENTION 100000
#define GUARD_TIME 6000

int sfd, quit_count;

char client_mac_arr[MAXCLIENT][18];


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
struct TDMAControlPacket set_tdma_packet(int pckType, char *id, int duration) {
    struct TDMAControlPacket pck;
    bzero(&pck, sizeof(pck));
    pck.type = pckType;
    strcpy(pck.id, id);
    //strcpy(pck.dtbuf, databuffer);
    pck.slot_duration = duration;
    
    return pck;
}


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
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if((sfd = socket(domain, type, protocol)) == -1)
    diep("socket");
    
    if(!isBroadcast) {
        if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof (struct timeval)) == -1)
        diep("timeout");
    }
    else
    {
        if(setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1)
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

int check_client_id(char *id) {
    int found = 0;
    int counter;
    for(counter = 0; counter < MAXCLIENT; counter++) {
        if(strcmp(id, client_mac_arr[counter]) == 0) {
            found = 1;
            return found;
        }
    }
    return found;
}

// command line parameter should be broadcast address
int main(int argc, char **argv) {
    int count, client_count;
    int numbytes;
    char line[BUFLEN];
    socklen_t len;
    long proc_time, timeout;
    struct timeval start, end, val1, timeo1, timeo2;
    struct TDMAControlPacket pck;
    struct UdpPacket *udpPack;
    
    for(count = 0; count < MAXCLIENT; count++)
    memset(&client_mac_arr[count],'',18);
    /*
    * socketaddr_in is a structure containing an Internet socket address.
    * It contains: an address family, a port number, an IP address
    */
    struct sockaddr_in caddr, saddr;
    
    if(argc != 2) {
        printf("Usage: %s HostAddressn", argv[0]);
        return -1;
    }
    
    /*
    * Create a socket.
    * AF_INET says that it will be an Internet socket.
    * SOCK_DGRAM says that it will use datagram delivery instead of virtual circuits.
    * IPPROTO_UDP says that it will use the UDP protocol
    */
    sfd = create_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 1);
    
    caddr = manage_sockaddr_in_send(argv[1], BPORT);
    
    // broadcasting becaon packet
    client_count = 0;
    quit_count = 0;
    
    while(1) {
        /*
        * broadcasting contention packet
        */
        pck = set_tdma_packet(0, "NO", (CONTENTION - GUARD_TIME));
        len = sizeof(caddr);
        
        gettimeofday(&start, NULL);
        
        if ((numbytes = sendto(sfd, (const char *)&pck, sizeof(pck), 0, (struct sockaddr *)&caddr, len)) == -1)
        diep("sendto()");
        
        
        printf("nContention Send | %ld,%ldn", start.tv_sec, start.tv_usec);
        /*
        * listening Join request from client
        */
        close(sfd);
        sfd = create_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0);
        caddr = manage_sockaddr_in_recv(JPORT);
        
        // the socket sfd should be bound to the address in caddr.
        if(bind(sfd, (struct sockaddr *)&caddr, sizeof(caddr)) == -1)
        diep("bind");
        
        proc_time = 0;
        bzero(&udpPack, sizeof(udpPack)); // should be removed if OK without this line
        len = sizeof(caddr);
        
        
        while(1) {
            gettimeofday(&end, NULL);
            proc_time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
            if(proc_time >= 92000)
            break;
            
            numbytes = recvfrom(sfd, line, sizeof(struct UdpPacket), 0, (struct sockaddr *)&saddr, &len);
            
            udpPack = (struct UdpPacket *)line;
            line[numbytes] = '';
            
            printf("Join Request | %s | %ld,%ldn", udpPack->id, end.tv_sec, end.tv_usec);
            
            if(!check_client_id(udpPack->id)) {
                strcpy(client_mac_arr[client_count], udpPack->id);
                client_count++;
            }
            else
            printf("Client already exist: %sn", client_mac_arr[client_count - 1]);
        }
        
        printf("Contention Slot | %ldnn", proc_time);
        
        // calculating time slot duration based on clients
        int time_slot_duration;
        time_slot_duration = (TIME_FRAME * 1000000 - CONTENTION) / client_count;
        
        /*
        * broadcasting control packet
        */
        count = 0;
        while(client_mac_arr[count] != '' && count < client_count) {
            close(sfd);
            sfd = create_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 1);
            caddr = manage_sockaddr_in_send(argv[1], BPORT);
            len = sizeof(caddr);
            
            pck = set_tdma_packet(1, client_mac_arr[count], (time_slot_duration - GUARD_TIME));
            
            if ((numbytes = sendto(sfd, (const char *)&pck, sizeof(pck), 0, (struct sockaddr *)&caddr, len)) == -1)
            diep("sendto()");
            
            gettimeofday(&val1, NULL);
            printf("Control Send | %s | %ld,%ldn", pck.id, val1.tv_sec, val1.tv_usec);
            
            close(sfd);
            sfd = create_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0);
            caddr = manage_sockaddr_in_recv(JPORT);
            
            // the socket sfd should be bound to the address in caddr.
            if(bind(sfd, (struct sockaddr *)&caddr, sizeof(caddr)) == -1)
            diep("bind");
            
            proc_time = 0;
            bzero(&udpPack, sizeof(udpPack)); // should be removed if OK without this line
            len = sizeof(caddr);
            
            // receiving notification from clients
            
            gettimeofday(&timeo1, NULL);
            while(1) {
                numbytes = recvfrom(sfd, line, sizeof(struct UdpPacket), 0, (struct sockaddr *)&saddr, &len);
                
                udpPack = (struct UdpPacket *)line;
                line[numbytes] = '';
                
                if(strcmp(udpPack->id, client_mac_arr[count]) == 0 && udpPack->end_of_transmission == 1) {
                    printf("GET NOTIFICATION | %s | %dn", udpPack->id, udpPack->end_of_transmission);
                    break;
                }
                
                gettimeofday(&timeo2, NULL);
                timeout = ((timeo2.tv_sec * 1000000 + timeo2.tv_usec) - (timeo1.tv_sec * 1000000 + timeo1.tv_usec));
                if(timeout > 2000000) {
                    quit_count++;
                    break;
                }
                if(quit_count == 2) {
                    diep("NO CLIENT");
                }
            }
            
            count++;
        }
    }
    close(sfd);
    
    return 0;
}
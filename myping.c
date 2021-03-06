// icmp.cpp
// Robert Iakobashvili for Ariel uni, license BSD/MIT/Apache
// 
// Sending ICMP Echo Requests using Raw-sockets.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h><
#include <sys/time.h> // gettimeofday()
#include <fcntl.h>
#include <resolv.h>
#include <netdb.h>
#include <sys/wait.h>
#include <time.h>
#include <net/ethernet.h>


#define ICMP_ECHO       8
#define ICMP_ECHOREPLY  0
#define IP_MAXPACKET 65535


 // IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8 

// Checksum algo
unsigned short calculate_checksum(unsigned short * paddress, int len);

// 1. Change SOURCE_IP and DESTINATION_IP to the relevant
//     for your computer
// 2. Compile it using MSVC compiler or g++
// 3. Run it from the account with administrative permissions,
//    since opening of a raw-socket requires elevated preveledges.
//
//    On Windows, right click the exe and select "Run as administrator"
//    On Linux, run it as a root or with sudo.
//
// 4. For debugging and development, run MS Visual Studio (MSVS) as admin by
//    right-clicking at the icon of MSVS and selecting from the right-click 
//    menu "Run as administrator"
// myheader_to_send->ip_dst=DESTINATION_IP;
//  Note. You can place another IP-source address that does not belong to your
//  computer (IP-spoofing), i.e. just another IP from your subnet, and the ICMP
//  still be sent, but do not expect to see ICMP_ECHO_REPLY in most such cases
//  since anti-spoofing is wide-spread.

#define SOURCE_IP "127.0.0.1"
// i.e the gateway or ping to google.com for their ip-address
#define DESTINATION_IP "8.8.8.8"



int main (){

   
    struct ip iphdr; // IPv4 header
    struct icmp icmphdr; // ICMP-header
    char data[IP_MAXPACKET] = "This is the ping.!!!\n";

    int datalen = strlen(data) + 1;

/*
    //==================
    // IP header
    //==================

    // IP protocol version (4 bits)
    iphdr.ip_v = 4;

    // IP header length (4 bits): Number of 32-bit words in header = 5
    iphdr.ip_hl = IP4_HDRLEN / 4; // not the most correct

    // Type of service (8 bits) - not using, zero it.
    iphdr.ip_tos = 0;

    // Total length of datagram (16 bits): IP header + ICMP header + ICMP data
    iphdr.ip_len = htons (IP4_HDRLEN + ICMP_HDRLEN + datalen);

    // ID sequence number (16 bits): not in use since we do not allow fragmentation
    iphdr.ip_id = 0;

    // Fragmentation bits - we are sending short packets below MTU-size and without 
    // fragmentation
    int ip_flags[4];

    // Reserved bit
    ip_flags[0] = 0;

    // "Do not fragment" bit
    ip_flags[1] = 0;

    // "More fragments" bit
    ip_flags[2] = 0;

    // Fragmentation offset (13 bits)
    ip_flags[3] = 0;

    iphdr.ip_off = htons ((ip_flags[0] << 15) + (ip_flags[1] << 14)
                      + (ip_flags[2] << 13) +  ip_flags[3]);

    // TTL (8 bits): 128 - you can play with it: set to some reasonable number
    iphdr.ip_ttl = 128;

    // Upper protocol (8 bits): ICMP is protocol number 1
    iphdr.ip_p = IPPROTO_ICMP;

    // Source IP
    if (inet_pton (AF_INET, SOURCE_IP, &(iphdr.ip_src)) <= 0) 
    {
        fprintf (stderr, "inet_pton() failed for source-ip with error: %d"
#if defined _WIN32
			, WSAGetLastError()
#else
			, errno
#endif
			);
        return -1; 
    }
 
   // Destination IPv
    if (inet_pton (AF_INET, DESTINATION_IP, &(iphdr.ip_dst)) <= 0)
    {
        fprintf (stderr, "inet_pton() failed for destination-ip with error: %d" 
#if defined _WIN32
			, WSAGetLastError()
#else
			, errno
#endif
			);
        return -1;
    }

    // IPv4 header checksum (16 bits): set to 0 prior to calculating in order not to include itself.
    iphdr.ip_sum = 0;
    iphdr.ip_sum = calculate_checksum((unsigned short *) &iphdr, IP4_HDRLEN);
*/

    //===================
    // ICMP header
    //===================

    // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_type = ICMP_ECHO;

    // Message Code (8 bits): echo request
    icmphdr.icmp_code = 0;

    // Identifier (16 bits): some number to trace the response.
    // It will be copied to the response packet and used to map response to the request sent earlier.
    // Thus, it serves as a Transaction-ID when we need to make "ping"
    icmphdr.icmp_id = 18; // hai

    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_seq = 0;

    // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
    icmphdr.icmp_cksum = 0;

    // Combine the packet <
    char packet[IP_MAXPACKET];

    // First, IP header.
    // memcpy (packet, &iphdr, IP4_HDRLEN);

    // Next, ICMP header
    memcpy (packet, &icmphdr, ICMP_HDRLEN);

    // After ICMP header, add the ICMP data.
    memcpy (packet + ICMP_HDRLEN, data, datalen);

    // Calculate the ICMP header checksumacket[i]%d'
    ((struct icmp*)packet)->icmp_cksum=calculate_checksum((unsigned short *) packet, ICMP_HDRLEN + datalen);
    

    struct sockaddr_in dest_in;
    memset (&dest_in, 0, sizeof (struct sockaddr_in));
    dest_in.sin_family = AF_INET;

    // The port is irrelant for Networking and therefore was zeroed.
    dest_in.sin_addr.s_addr=inet_addr(DESTINATION_IP);
    


    // Create raw socket for IP-RAW (make IP-header by yourself)
    
    int sock = -1;
  
    if ((sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1){
        fprintf (stderr, "socket() failed with error: %d",errno);
        fprintf (stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }

    // Send the packet using sendto() for sending datagrams.
    // starting the clock
    clock_t start, end, diff;
    start = clock();
   
    int packet_size;
    packet_size=sendto (sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *) &dest_in, sizeof (dest_in));
    if (packet_size==-1){
        printf("sendto failed with error:%d",errno);
        return -1;
    }
    printf("we sent 1 packet:\n");
    printf("Size: %d bytes: ICMP header(%d) + data(%d)\n", packet_size, ICMP_HDRLEN, datalen);
    printf("Data sent: %s \n", packet + ICMP_HDRLEN);

    socklen_t len = sizeof(dest_in);
    int recieve_size=-1;
    // making sure we recieved a packet 
    recieve_size=recvfrom(sock, &packet, sizeof(packet), 0, (struct sockaddr *) &dest_in, &len);
    if (recieve_size>0){
        //printing all the packet data
        printf("Msg recieved\n");
        printf("packet size: %d bytes: IP header(%d) + ICMP header(%d) + data(%d)\n", recieve_size, IP4_HDRLEN, ICMP_HDRLEN,datalen);
        printf("Data: %s \n", packet + IP4_HDRLEN+ICMP_HDRLEN);
    
        struct ip  *myheader=(struct ip  *)packet;
        printf("ip header data\n");
        printf("version of ip: %d\n",(myheader->ip_v));
        printf("length of header: %d\n",(myheader->ip_hl));
        printf("type of service: %d\n",(myheader->ip_tos));
        printf("length of the whole packet: %d\n",(myheader->ip_len));
        printf("unique identifier of the packet: %d\n",(myheader->ip_id));
        printf("fragmantation flags: %d\n",(myheader->ip_off));
        printf("time to live: %d\n",(myheader->ip_ttl));
        printf("protocol: %d\n",(myheader->ip_p));
        printf("checksum: %d\n",(myheader->ip_sum));
        printf("ip source: %s\n",inet_ntoa(myheader->ip_src));
        printf("ip destionation: %s\n",inet_ntoa(myheader->ip_dst));
    
        struct icmp *myicmpheader=(struct icmp *)(packet+IP4_HDRLEN);
        printf("\nicmp header data\n");
        printf("type: %d\n",(myicmpheader->icmp_type));
        printf("code: %d\n",(myicmpheader->icmp_code));
        printf("checksum: %d\n",(myicmpheader->icmp_cksum));
        printf("id: %d\n",(myicmpheader->icmp_id));
        printf("sequence: %d\n",(myicmpheader->icmp_seq));
        printf("icmp data: %ld\n",(myicmpheader->icmp_data));
    
        printf ("\npacket data\n");
        int startp= IP4_HDRLEN+ ICMP_HDRLEN;
        int endp= IP4_HDRLEN+ICMP_HDRLEN + datalen;
        for( int i=startp;i<endp;i++){
            printf("%c",packet[i]);
        }        
    }
    //ending the clock
    end=clock();
    diff =(end-start);
    printf("\ntime diff is %f milli seconds\n", diff/10.0);
    printf("\ntime diff is %f micro seconds\n", diff*100.0);
    // cosig the socket
    close(sock);

    return 0;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short * paddress, int len){
	int nleft = len;
	int sum = 0;
	unsigned short * w = paddress;
	unsigned short answer = 0;

	while (nleft > 1){
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1){
		*((unsigned char *)&answer) = *((unsigned char *)w);
		sum += answer;
	}

	// add back carry outs from top 16 bits to low 16 bits
	sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
	sum += (sum >> 16);                 // add carry
	answer = ~sum;                      // truncate to 16 bits

	return answer;
}





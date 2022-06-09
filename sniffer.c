
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/ethernet.h>      // ethernet header
#include <netinet/ip_icmp.h> // icmp headder declaration.
#include <string.h>
#include <arpa/inet.h>






int main(){
    int PACKET_LEN = 65536;
    int raw_socket;
    int data_size;
    char buff[PACKET_LEN];
    struct sockaddr saddr;
    struct sockaddr_in src;
    struct sockaddr_in dst;
    int count=0;
    //creating raw socket:
    //the third paramter enables the socket to read full packet content
    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(raw_socket < 0){
        printf("Error creating socket\n");
    }

    // Receive all packets:
    while(1){
        bzero(buff, sizeof(buff)); // init buffer.
        // receve data from all ip sorce and dest
        data_size = recvfrom(raw_socket, buff,PACKET_LEN, 0, NULL, NULL);
        // check if we gwt a packet
        if(data_size < 0){
            printf("failed to get the packet\n");
        }
        else{ 
            //reading from icmp:
            struct ethhdr *eth = (struct ethhdr *)buff;
            struct iphdr *iph;
            if (ntohs(eth->h_proto) == 0x0800) { // 0x0800 is IP type
                iph = (struct iphdr *)(buff + sizeof(struct ethhdr));
            
                //getting the length of the ip header length field in bits in case of icmp protocol.
                int iph_len = iph->ihl *4;
                // check if the protocal is icmp id so print the wanted info
                if(iph->protocol == IPPROTO_ICMP){
                    
                    printf("*********** message #%d ***************\n",count);
                    // Starts after IP HEADER so we head there.
                    struct icmphdr *icmph = (struct icmphdr *)(buff + sizeof(struct ethhdr) + iph_len);

                    // Getting the ip data :
                    bzero(&src,sizeof(src));
                    bzero(&dst,sizeof(dst));
                    src.sin_addr.s_addr = iph->saddr;
                    dst.sin_addr.s_addr = iph->daddr;

               
                  
                    // printf("header data\n");
                    // printf("version of ip: %d\n",(iph->version));
                    // printf("length of header: %d\n",(iph->ihl));
                    // printf("type of service: %d\n",(iph->tos));
                    // printf("length of the whole packet: %d\n",(iph->tot_len));
                    // printf("unique identifier of the packet: %d\n",(iph->id));
                    // printf("fragmantation flags: %d\n",(iph->frag_off));
                    // printf("time to live: %d\n",(iph->ttl));
                    // printf("protocol: %d\n",(iph->protocol));
                    // printf("checksum: %d\n\n",(iph->check));
                    printf("SRC ip : %s\n",inet_ntoa(src.sin_addr)); //convert back
                    printf("DST ip : %s\n",inet_ntoa(dst.sin_addr)); // convert back
                    int type=icmph->type;
                    if (type==0){
                        printf("type : %d echo (ping) reply\n",type);
                    }
                    else if(type==8){
                        printf("type : %d echo (ping) request\n",type);
                    }
                    else{
                        printf("type : %d\n",type);    
                    }
                    printf("code : %d\n",(icmph->code));
                    // printf("checksum : %d\n",(icmph->checksum));
                    // printf("sequence : %d\n",(icmph->un.echo.sequence));
                    // printf("ICMP_seq: %d/%d\n",icmph->un.echo.sequence>>8, icmph->un.echo.sequence);



                    // printf ("\npacket data\n");
                    // int startp= sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct icmphdr);
                    // int endp= data_size;
                    // for( int i=startp;i<endp;i++){
                    //     printf("%c",buff[i]);
                    // }
                    // printf("\n");

                    count++;

            
                }
            }   
        }
    }
    close(raw_socket);
    return 0;    

}

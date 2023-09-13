#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // for sleep()

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <uuid/uuid.h>

int main(int argc, char *argv[])
{
    if (argc != 4) {
       printf("Command line args should be interface_address ip_address and port\n");
       printf("(e.g. for SSDP, `./unicast 172.27.40.208 3702`)\n");
       return 1;
    }

    char* if_address = argv[1]; // e.g. 239.255.255.250 for SSDP
    char* ip_address = argv[2]; // e.g. 239.255.255.250 for SSDP
    int port = atoi(argv[3]); // 0 if error, which is an invalid port

    // !!! If test requires, make these configurable via args
    //
    char out[1024];
    uuid_t b;
    uuid_generate(b);
    uuid_unparse_lower(b, out);

    const char* message = NULL;
    char disco[2048];
    sprintf(disco, "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\"><s:Header><a:Action s:mustUnderstand=\"1\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</a:Action><a:MessageID>uuid:%s</a:MessageID><a:ReplyTo><a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address></a:ReplyTo><a:To s:mustUnderstand=\"1\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To></s:Header><s:Body><Probe xmlns=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\"><d:Types xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:dp0=\"http://www.onvif.org/ver10/network/wsdl\">dp0:NetworkVideoTransmitter</d:Types></Probe></s:Body></s:Envelope>\n", out);
    char clock[1024];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(clock, "now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    message = disco;
    printf("%s", message);

    // Creating socket file descriptor
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
   
    // Source
    struct sockaddr_in     src_addr;
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(9000); 
    src_addr.sin_addr.s_addr = inet_addr(if_address);

    // Bind the socket to the source address
    if (bind(sockfd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Destination
    struct sockaddr_in     dst_addr;
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(port);
    dst_addr.sin_addr.s_addr = inet_addr(ip_address);
       
    sendto(sockfd, (const char *)message, strlen(message),
        MSG_CONFIRM, (const struct sockaddr *) &dst_addr, 
            sizeof(dst_addr));
    printf("Message sent\n\n");
      
    printf("Receiving...\n");
    socklen_t len;
    char buffer[2048]; 
    int n = recvfrom(sockfd, (char *)buffer, sizeof(buffer), 
                0, (struct sockaddr *) &src_addr,
                &len);
    buffer[n] = '\0';
    printf("%s\n", buffer);   

    char ip[16];
    inet_ntop(AF_INET, &src_addr.sin_addr, ip, sizeof(ip));
    printf("Received from %s\n", ip);

    close(sockfd);
    return 0;
}

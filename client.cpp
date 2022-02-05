

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sstream>

#define PORT "34403" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

using namespace std;
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
    
}

int main(int argc, char *argv[])
{
    //initialize TCP socket of client, souce code: Beej's Guide to Soket Programming
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int r;
    char s[32];
    int decision;//1 for A, 2 for B, 3 for C
    
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((r = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
        return 1;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("client: connect");
        continue;
        }
    break;
        
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    cout<<"The client is up and running"<<endl;  //Initializaton finished
    
    freeaddrinfo(servinfo); // all done with this structure
    //send client location to scheduler through TCP connection
    send(sockfd, argv[1], sizeof argv[1]+1, 0);

    printf("The client has sent query to Scheduler using TCP: client location %s\n",argv[1]);
    
    //receive the information about assigned hospital
    if((numbytes = recv(sockfd, buf, sizeof(buf), 0)) ==-1){
        perror("recv");
        exit(1);
    }
    for(int i=numbytes;i<MAXDATASIZE;i++){
        buf[i]='\0';
    }
    istringstream istr(buf);
    istr >> decision;

    switch (decision) {
        case 1:
            printf("The client has received results from the Scheduler: assigned to Hospital A\n");
            break;
        case 2:
            printf("The client has received results from the Scheduler: assigned to Hospital B\n");
            break;
        case 3:
            printf("The client has received results from the Scheduler: assigned to Hospital C\n");
            break;
        default:
            printf("The client has received results from the Scheduler: assigned to Hospital None\n");
            break;
    }
    close(sockfd);
    return 0;
    
}

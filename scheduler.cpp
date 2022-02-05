
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
#include <iostream>
#include <string>
#include<cstring>

#define PORT "34403" // the port client will be connecting to
#define BACKLOG 10 // how many pending  connections queue will hold
#define HOS_A "30403"//the port number of hospital A
#define HOS_B "31403"//the port number of hospital B
#define HOS_C "32403"//the port number of hostpial C
#define MYPORT "33403" // the port number of scheduler UDP

#define MAXBUFLEN 100
using namespace std;

void sigchld_handler(int s)
{
// waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6：
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET){
            return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

in_port_t *get_in_port(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_port);
}

int designate(double scoreA,double scoreB,double scoreC,double distanceA,double distanceB,double distanceC){
    if(scoreA>scoreB){
        if(scoreA>scoreC){
            return 1;
        }
        else if (scoreA == scoreC){
            if(distanceA<distanceC){
                return 1;
            }
            else return 3;
        }
        else return 3;  //scoreA < scoreC
    }
    else if (scoreA==scoreB){
        if (scoreA>scoreC){
            return 1;
        }
        else if (scoreA==scoreC){
            if(distanceA<distanceC){
                return 1;
            }
            else return 3;
        }
        else return 3;    //scoreA < scoreC
    }
    else{     //scoreA <scoreB
        if(scoreB>scoreC){
            return 2;
        }
        else if(scoreB==scoreC){
            if(distanceB<distanceC){
                return 2;
            }
            else return 3;
        }
        else return 3;
    }
}

void printScrDis(int portnum, double score, double distance){ // to display score and distance information according to different situations.
    switch (portnum) {
        case 30403:
            if(score!=0 && distance!=0)
                printf("The Scheduler has received map information from Hospital A, the score = ​%f and the distance = ​%f.\n",score,distance);
            else if (score==0 && distance!=0)
                printf("The Scheduler has received map information from Hospital A, the score = ​None and the distance = ​%f.\n",distance);
            else if (score!=0 && distance==0)
                printf("The Scheduler has received map information from Hospital A, the score = ​%f and the distance = ​None.\n",score);
            else printf("The Scheduler has received map information from Hospital A, the score = ​None and the distance = ​None.\n");
            
            break;
            
        case 31403:
            if(score!=0 && distance!=0)
                printf("The Scheduler has received map information from Hospital B, the score = ​%f and the distance = ​%f.\n",score,distance);
            else if (score==0 && distance!=0)
                printf("The Scheduler has received map information from Hospital B, the score = ​None and the distance = ​%f.\n",distance);
            else if (score!=0 && distance==0)
                printf("The Scheduler has received map information from Hospital B, the score = ​%f and the distance = ​None.\n",score);
            else printf("The Scheduler has received map information from Hospital B, the score = ​None and the distance = ​None.\n");
            
            break;
        case 32403:
            if(score!=0 && distance!=0)
                printf("The Scheduler has received map information from Hospital C, the score = ​%f and the distance = ​%f.\n",score,distance);
            else if (score==0 && distance!=0)
                printf("The Scheduler has received map information from Hospital C, the score = ​None and the distance = ​%f.\n",distance);
            else if (score!=0 && distance==0)
                printf("The Scheduler has received map information from Hospital C, the score = ​%f and the distance = ​None.\n",score);
            else printf("The Scheduler has received map information from Hospital C, the score = ​None and the distance = ​None.\n");
            
            break;
    }
}

int main(int argc,char * argv[])
{
    //initialization of TCP socket of scheduler,souce code: Beej's Guide to Soket Programming
    int sockfd_TCP, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints_TCP, *servinfo_TCP, *p_TCP;
    struct sockaddr_storage their_addr_TCP; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char dst[32];
    int rv_TCP;
    char buf_TCP[MAXBUFLEN];
    struct sockaddr_in connectedAddr;
    char clientLocation[MAXBUFLEN];
    
    memset(&hints_TCP, 0, sizeof hints_TCP);
    hints_TCP.ai_family = AF_INET;  //IPv4
    hints_TCP.ai_socktype = SOCK_STREAM;
    hints_TCP.ai_flags = AI_PASSIVE;
    
    for(int i=0;i<MAXBUFLEN;i++){
        buf_TCP[i]=0;
    }

    //error-checking
    if ((rv_TCP = getaddrinfo(NULL, PORT, &hints_TCP, &servinfo_TCP)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_TCP));
        return 1;
    }
        // loop through all the results and bind to the first we can
    for(p_TCP = servinfo_TCP; p_TCP != NULL; p_TCP = p_TCP->ai_next) {
        if ((sockfd_TCP = socket(p_TCP->ai_family, p_TCP->ai_socktype,p_TCP->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        int opt=1;
        if (setsockopt(sockfd_TCP, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == -1) {
            perror("setsockopt");
            exit(1);

        }
        if (bind(sockfd_TCP, p_TCP->ai_addr, p_TCP->ai_addrlen) == -1) {
            close(sockfd_TCP);
            perror("server: bind");
            continue;

        }
        break;

    }
    freeaddrinfo(servinfo_TCP);
    if(p_TCP==NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd_TCP, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
//    initialize UDP socket information of scheduler, souce code: Beej's Guide to Soket Programming
    
    int sockfd_UDP;
    struct addrinfo hints_UDP, *servinfo_UDP, *p_UDP;
    int rv_UDP;
    int numbytes;
    struct sockaddr_storage their_addr_UDP;
    char buf[MAXBUFLEN];   //UDP buffer
    socklen_t addr_len;
    char s[32];
    
    memset(&hints_UDP, 0, sizeof hints_UDP);
    hints_UDP.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints_UDP.ai_socktype = SOCK_DGRAM;
    hints_UDP.ai_flags = AI_PASSIVE;
    if ((rv_UDP = getaddrinfo(NULL, MYPORT, &hints_UDP, &servinfo_UDP)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_UDP));
        return 1;
    }
    
    for(p_UDP = servinfo_UDP; p_UDP != NULL; p_UDP = p_UDP->ai_next) {
        if ((sockfd_UDP = socket(p_UDP->ai_family, p_UDP->ai_socktype,p_UDP->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
    }
        if (bind(sockfd_UDP, p_UDP->ai_addr, p_UDP->ai_addrlen) == -1) {
            close(sockfd_UDP);
            perror("listener: bind");
            continue;
        }
        break;
    }
    if (p_UDP == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    
    freeaddrinfo(servinfo_UDP);
    
    
    //initialize UDP socket connected to hospital A, souce code: Beej's Guide to Soket Programming
    struct addrinfo hints_UDPA, *servinfo_UDPA, *p_UDPA;
    int rv_UDPA;

    memset(&hints_UDPA, 0, sizeof hints_UDPA);
    hints_UDPA.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints_UDPA.ai_socktype = SOCK_DGRAM;
    hints_UDPA.ai_flags = AI_PASSIVE; // use my IP
    if ((rv_UDPA = getaddrinfo("127.0.0.1", HOS_A, &hints_UDPA, &servinfo_UDPA)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_UDPA));
        return 1;
    }
    // loop through all the results and bind to the first we can
    p_UDPA = servinfo_UDPA;
        
    //initialize UDP socket connected to hospital B, souce code: Beej's Guide to Soket Programming
    struct addrinfo hints_UDPB, *servinfo_UDPB, *p_UDPB;
    int rv_UDPB;

    memset(&hints_UDPB, 0, sizeof hints_UDPB);
    hints_UDPB.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints_UDPB.ai_socktype = SOCK_DGRAM;
    hints_UDPB.ai_flags = AI_PASSIVE; // use my IP

    if ((rv_UDPB = getaddrinfo("127.0.0.1", HOS_B, &hints_UDPB, &servinfo_UDPB)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_UDPB));
        return 1;
    }
    p_UDPB = servinfo_UDPB;
    
    //initialize UDP socket connected to hospital C, souce code: Beej's Guide to Soket Programming
    struct addrinfo hints_UDPC, *servinfo_UDPC, *p_UDPC;
    int rv_UDPC;
    
    memset(&hints_UDPC, 0, sizeof hints_UDPC);
    hints_UDPC.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints_UDPC.ai_socktype = SOCK_DGRAM;
    hints_UDPC.ai_flags = AI_PASSIVE; // use my IP
    if ((rv_UDPC = getaddrinfo("127.0.0.1", HOS_C, &hints_UDPC, &servinfo_UDPC)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_UDPC));
        return 1;
    }
    
    p_UDPC = servinfo_UDPC;
    
    printf("The Scheduler is up and running.\n");
    //receive hospitals' capacities and initail occupancies
    int total,totalA,totalB,totalC; //total capability
    int occu,occuA,occuB,occuC,occuNew; //occupancy
    double avaA,avaB,avaC,avaNew; //availability
    int portnum; //received port number from client or hospital
    double score,scoreA,scoreB,scoreC=0;
    double distance,distanceA,distanceB,distanceC=0;
    int decision=0;//the final designated hospital; 1 for A, 2 for B, 3 for C
    
    //keep scheduler working once it is booted up
    while(1) {
        for(int i =1;i<4;i++){
            //receive the total capacity and the occupancy of hospital A/B/C
            if ((numbytes = recvfrom(sockfd_UDP, &buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr_UDP, &addr_len)) == -1) {
                perror("recvfrom");
                exit(1);
            }
            for(int i=numbytes;i<MAXBUFLEN;i++){
                buf[i]='\0';
            }

            istringstream istr(buf);//split the two elements
            istr>>total>>occu>>portnum;
            //print on screen message about the total capacity and the initial occupancy
            switch (portnum) {
                case 30403:
                    printf("The Scheduler has received information from Hospital A: total capacity is %d, and initial occupancy is %d\n",total,occu);
                    totalA=total;
                    occuA=occu;
                    break;
                case 31403:
                    printf("The Scheduler has received information from Hospital B: total capacity is %d, and initial occupancy is %d\n",total,occu);
                    totalB=total;
                    occuB=occu;
                    break;
                case 32403:
                    printf("The Scheduler has received information from Hospital C: total capacity is %d, and initial occupancy is %d\n",total,occu);
                    totalC=total;
                    occuC=occu;
                    break;
                    
            }
            
        }
        
        sin_size = sizeof their_addr_TCP;
        new_fd = accept(sockfd_TCP, (struct sockaddr *)&their_addr_TCP, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(their_addr_TCP.ss_family, get_in_addr((struct sockaddr *)&their_addr_TCP), dst, sizeof dst);
        //receive TCP message containing client location
        recv(new_fd, buf_TCP, sizeof buf_TCP, 0);

        inet_ntop(their_addr_TCP.ss_family, get_in_addr((struct sockaddr *)&their_addr_TCP), dst, sizeof dst);
        printf("The Scheduler has received client at location ​%s from the client using TCP over port 34403\n",buf_TCP);
        strcpy(clientLocation, buf_TCP);    //save the client location in scheduler
        // only send client location to hospital with availability
        if(totalA>occuA && occuA>=0){
            if ((numbytes = sendto(sockfd_UDP, clientLocation, strlen(clientLocation), 0,
                 p_UDPA->ai_addr, p_UDPA->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
            }
            printf("The Scheduler has sent client location to Hospital A using UDP over port %s\n",MYPORT);
        }
        if(totalB>occuB && occuB>=0){
            if ((numbytes = sendto(sockfd_UDP, clientLocation, strlen(clientLocation), 0,
                 p_UDPB->ai_addr, p_UDPB->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
            }
            printf("The Scheduler has sent client location to Hospital B using UDP over port %s\n",MYPORT);
        }
        if(totalC>occuC && occuC>=0){
            if ((numbytes = sendto(sockfd_UDP, clientLocation, strlen(clientLocation), 0,
                 p_UDPC->ai_addr, p_UDPC->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
            }
            printf("The Scheduler has sent client location to Hospital C using UDP over port %s\n",MYPORT);
        }
        
//        receive score and distance from hospitals
        int pause=0;  //count the number of UDP packet that not sent to hospital
        if(totalA<=occuA || occuA<0)
            ++pause;
        if(totalB<=occuB || occuB<0)
            ++pause;
        if(totalC<=occuC || occuC<0)
            ++pause;
        int repeat = 4;
        repeat = repeat-pause;  // scheduler doesn't send to full hospital
        
        //receive score and distance from hospitals
        for(int i=1;i<repeat;i++){
            if ((numbytes = recvfrom(sockfd_UDP, &buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr_UDP, &addr_len)) == -1){
                perror("recvfrom");
                exit(1);
            }

            for(int i=numbytes;i<MAXBUFLEN;i++){
                buf[i]='\0';
            }

            istringstream istr(buf);
            istr >> score>>distance>>portnum; //split the elements


            switch (portnum) {
                case 30403:
                    printScrDis(30403, score, distance);
                    scoreA=score;
                    distanceA=distance;
                    break;
                case 31403:
                    printScrDis(31403, score, distance);
                    scoreB=score;
                    distanceB=distance;
                    break;
                case 32403:
                    printScrDis(32403, score, distance);
                    scoreC=score;
                    distanceC=distance;
                    break;
            }
        }
        //deciding which hospital should be assigned to the client
        if(scoreA!=0 && scoreB!=0 && scoreC!=0 && distanceA!=0 && distanceB!=0 && distanceC!=0){
            decision=designate(scoreA, scoreB, scoreC,distanceA,distanceB,distanceC);
            switch (decision) {
                case 1:
                    ++occuA;
                    avaA = double (totalA-occuA)/totalA;
                    scoreA = double (1/(distanceA*(1.1-avaA)));
                    avaNew = avaA;
                    occuNew = occuA;
                    printf("The Scheduler has assigned Hospital A to the client.\n");
                    break;
                case 2:
                    ++occuB;
                    avaB = double (totalB-occuB)/totalB;
                    scoreB = double( 1/(distanceB*(1.1-avaB)));
                    avaNew = avaB;
                    occuNew = occuB;
                    printf("The Scheduler has assigned Hospital B to the client.\n");
                    break;
                case 3:
                    ++occuC;
                    avaC = double (totalC-occuC)/totalC;
                    scoreC =double ( 1/(distanceC*(1.1-avaC)));
                    avaNew = avaC;
                    occuNew = occuC;
                    printf("The Scheduler has assigned Hospital C to the client.\n");
                    break;
                default:
                    break;
                    
            }
        }
        else{
            printf("The Scheduler has assigned Hospital None to the client\n");   //if any hospital scores = 0, or distance = 0, display none assignment
        }
        
        // sends final score to client by TCP
        string sDecision = to_string(decision);
        char *chDecision = (char*)sDecision.c_str();
        if(send(new_fd, chDecision, strlen(chDecision), 0)==-1){
            perror("send");
        }
        printf("The Scheduler has sent the result to client using TCP over port %s.\n",PORT);
        
        
        //send updated occupation and availability to corresponding hospital
         string message=to_string(occuNew)+" "+to_string(avaNew);
         char *msg = (char*)message.c_str();
         
         switch (decision) {
             case 1:
                 if ((numbytes = sendto(sockfd_UDP,msg, strlen(msg), 0, p_UDPA->ai_addr, p_UDPA->ai_addrlen)) == -1) {
                     perror("talker: sendto");
                     exit(1);
                 }
                 printf("The Scheduler has sent the result to Hospital A using UDP over port 33403.\n");
                 break;
             case 2:
                 if ((numbytes = sendto(sockfd_UDP,msg, strlen(msg), 0, p_UDPB->ai_addr, p_UDPB->ai_addrlen)) == -1) {
                     perror("talker: sendto");
                     exit(1);
                 }
                 printf("The Scheduler has sent the result to Hospital B using UDP over port 33403.\n");
                 break;
             case 3:
                 if ((numbytes = sendto(sockfd_UDP,msg, strlen(msg), 0, p_UDPC->ai_addr, p_UDPC->ai_addrlen)) == -1) {
                     perror("talker: sendto");
                     exit(1);
                 }
                 printf("The Scheduler has sent the result to Hospital C using UDP over port 33403.\n");
                 break;
         }
        
    //   completeed communication with scheduler using UDP
    
    }
    close(sockfd_UDP);
    close(new_fd);
    close(sockfd_TCP);
    exit(0);
    return 0;
    
}


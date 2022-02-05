#include<sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <string.h>
#include <set>
#include <map>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
//#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//UDP arguments
#define SERVERPORT "33403"
#define MYPORT "32403"
#define DEST_IP_ADDRESS "127.0.0.1"
#define DEST_PORT 33403
#define SERV_PORT 32403

#define MAXBUFLEN 100
#define Max 503
#define INF 0xcffffff
using namespace std;

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)sa)->sin_addr);
}
    return &(((struct sockaddr_in6*)sa)->sin6_addr); }


typedef struct AMGraph {                            //define Graph
    int vex, arc;
    double arcs[Max][Max];                               //adjacent matrix
};

double dis[Max]; //shortest distance between two points
int path[Max];                            //the path that get shortest distance
bool book[Max];

struct ListNode{
    int val;
    struct ListNode* next;
    ListNode(int x) :
        val(x), next(NULL){
    }
 
};

struct ListNodeDB{
    double val;
    struct ListNodeDB* next;
    ListNodeDB(double x) :
        val(x), next(NULL){
    }
 
};

//reindex the vertix of  map.txt

void reindex(ListNode* vex1, ListNode* vex2,int lines, int numNodes, int *allNodes, map<int,int> &indexVex1, map<int,int> &indexVex2){
    int *index1;  //the address of vex1 after reindex
    int *index2; //the address of vex2 after reindex

    for(int a = 0 ;a<lines; a++){
        vex1 = vex1->next;
        vex2 = vex2->next;

        index1 = find(allNodes,allNodes + numNodes,vex1->val);
        index2 = find(allNodes,allNodes + numNodes,vex2->val);
        
        indexVex1[index1-allNodes+1] = vex1->val;  //insert key-value of original vertex index and reindex
        indexVex2[index2-allNodes+1] = vex2->val;
    }

}

void initialMatrix(AMGraph &G, ListNode* head1, ListNode* head2, ListNodeDB* head3,map<int,int> indexVex1, map<int,int> indexVex2){
    for (int i = 1; i <= G.vex; i++)
    for (int j = 1; j <= G.vex; j++)
            G.arcs[i][j] = INF;

    double routeDis;
    ListNode* m = head1;
    ListNode* n = head2;
    ListNodeDB* r = head3;

    m = m->next;
    n = n->next;
    r = r->next;
    int v1,v2;

    for (int i = 1; i <= G.arc; i++){
        if(r!=nullptr){
            routeDis = r->val;
            for(std::map<int,int>::iterator it1 = indexVex1.begin();it1!=indexVex1.end();it1++)
                {
                    if(it1->second == m->val){
                        v1 = it1->first;
                    }
                }
            for(std::map<int,int>::iterator it2 = indexVex2.begin();it2!=indexVex2.end();it2++)
                {
                    if(it2->second == n->val){
                        v2 = it2->first;
                    }
                }
            G.arcs[v1][v2] = routeDis;
            r = r->next;
            m = m->next;
            n = n->next;
        };

    };
}
void symmetry(AMGraph &G, int *vertixNo, int &locHos,int &reLocHos,int &destination,int &reDestination){
    int *index3;
    int *index4;
    index3 = find(vertixNo,vertixNo+G.vex ,locHos);
    index4 = find(vertixNo,vertixNo+G.vex,destination);
    reLocHos = index3-vertixNo+1;
    reDestination = index4-vertixNo+1;
    
    for(int i=1;i<=G.vex;i++){
        for(int j=1;j<=G.vex;j++){
            if(G.arcs[i][j]!=G.arcs[j][i]){
                if(G.arcs[i][j]<INF){
                    G.arcs[j][i] = G.arcs [i][j];
                }
                else{
                    G.arcs[i][j] = G.arcs [j][i];
                }
                
            }
        }
    }
    
};

double Dijkstra1(AMGraph &G, int posHos,int des, int rePosHos, int reDes)                            //Dijstra algorithm for searching the shortest path, source code: https://blog.csdn.net/Carizy/article/details/107280388
{

    for (int i = 1; i <= G.vex; i++)
    {
        dis[i] = G.arcs[rePosHos][i];
        path[i] = dis[i] < INF ? 1 : -1;
    }
    book[rePosHos] = true;
    dis[rePosHos] = 0;
    for (int i = 2; i <= G.vex; i++)
    {

        int mins = INF, u = 1;
        for (int j = 1; j <= G.vex; j++)
        {
            if (!book[j] && mins > dis[j]) {
                mins = dis[j];
                u = j;
            }
        }
        book[u] = true;
        for (int j = 1; j <= G.vex; j++)
        {
            if (!book[j] && dis[j] > dis[u] + G.arcs[u][j]) {
                dis[j] = dis[u] + G.arcs[u][j];
                path[j] = u;
            }
        }
    }
    return dis[reDes];
};
int main(int argc, const char * argv[]) {
    AMGraph G;

    ifstream infile;
    string file;
    file="map.txt";
    infile.open(file.data());   //load map.txt
    assert(infile.is_open());    //if failed, stop the program

    int lineno;
    lineno = 0;
    double arcs;
    int vex1,vex2;
    double  w;
    set<int> vexs;
    map<int, int> indexVex1;   //to store key-value of vex1
    map<int, int> indexVex2;   //to store key-value of vex2
    
    ListNode* head1 = new ListNode(vex1);  //linklist ofvex
    ListNode* p1 = head1;
    
    ListNode* head2 = new ListNode(vex2);  //linklist of arc
    ListNode* p2 = head2;
    
    ListNodeDB* head3 = new ListNodeDB(w);  //linklist of arcs
    ListNodeDB* p3 = head3;
    //    notification of entering three arguments of hospital C
    if(argc!=4){
        cout <<"Please enter Location, Total Capacity, Occupancy"<<endl;
        exit(1);
    }


    while (!infile.eof())
    {
        infile >> vex1 >> vex2 >>w;
//        cout<<setiosflags(ios::fixed)<<setprecision(15)<<w<<endl; //显示浮点型数据的精度调整
        arcs=w;
        
        ListNode* q1 = new ListNode(vex1);  //vertixs of first column
        p1->next = q1;
        p1 = p1->next;
        
        ListNode* q2 = new ListNode(vex2);   //vertixs of send column
        p2->next = q2;
        p2 = p2->next;
        
        ListNodeDB* q3 = new ListNodeDB(w);  //arcs
        p3->next = q3;
        p3 = p3->next;
        

        vexs.insert(vex1);  //add elements into set vexs
        vexs.insert(vex2);   // add elements into set vexs, and ordered from small to big numbers

        lineno++;  //the number of routes in map.txt
    }
    G.arc=lineno;
    infile.close();
    G.vex = vexs.size();//the number of vertix of map.txt
    int vertixNo[vexs.size()];
    int counter = 0;
    
    set<int>::iterator iter = vexs.begin();
        while (iter!=vexs.end())
        {
            vertixNo[counter] = *iter;
            counter++;
            iter++;
        }
    p1 = head1;       //initialize p1 pointer
    p2 = head2;        //initialize p2 pointer

    reindex(p1,p2,lineno, G.vex, vertixNo,indexVex1,indexVex2);
    
    p1 = head1;   //initialize p1 pointer
    p2 = head2;    //initialize p2 pointer
    p3 = head3;     //initialize p3 pointer
    
    initialMatrix(G, p1, p2, p3,indexVex1, indexVex2); // Hospital finishes read map.txt
   
    int totalCapacity = atoi(argv[2]);//acqiure total capacity from terminal input
    int initialOccupancy =atoi(argv[3]);//acqiure initial occupancy from terminal
    double availability = 0;

    int locHos = atoi(argv[1]); // location of hospital C
    int reDestination = 0;   //reindex of client location
    int reLocHos = 0; //location of hospital C（reindexed)
    double distance = 0; //the distance between hospital C and client
    double score=0;
    
    if (argc != 4) {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }
    //initialize UDP socket,source code: Beej's Guide of Soket Programming
    int sock_fd;
    char send_buf[4096];
    char recv_buf[4096];
    int numbytes;
    int recv_num;
    int len_host;
    struct sockaddr_in addr_host;
      
    memset(&addr_host, 0, sizeof(addr_host));
    addr_host.sin_family = AF_INET;
    addr_host.sin_addr.s_addr = inet_addr(DEST_IP_ADDRESS);
    addr_host.sin_port = htons(DEST_PORT);
    len_host = sizeof(addr_host);
    int len_serv; // sizeof(addr_serv)
    struct sockaddr_in addr_serv;
    struct sockaddr_in addr_client;
    int send_num;
    
    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_serv.sin_port = htons(SERV_PORT);
    len_serv = sizeof(addr_serv);

    //create socket
    if( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("socket");
      exit(1);
    }
    
    //bind socket
    if(bind(sock_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0)
    {
      perror("bind error:");
      exit(1);
    }
    printf("Hospital C is up and running using UDP on port %s.\n",MYPORT);
    printf("Hospital C has total capacity ​%d and initial occupancy ​%d.\n",totalCapacity,initialOccupancy);
    //finish initialization of hospital C

    string message=to_string(totalCapacity)+" "+to_string(initialOccupancy)+" "+MYPORT;
    char *msg = (char*)message.c_str();
    //send total capacity and intial occupancy to scheduler
   
    if((numbytes=sendto(sock_fd, msg, strlen(msg), 0, (struct sockaddr *)&addr_host, len_host))==-1){
        perror("sendto error:");
            exit(1);
    }
    while (1) {
        //calculate availability of hospital C
            availability =double (totalCapacity - initialOccupancy)/totalCapacity;
        if((numbytes = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len_serv))==-1){
            perror("recvfrom error:");
            exit(1);
        }
        int clientLocation=atoi(recv_buf);  //convert client location into int type
        if(vexs.count(clientLocation)==0){
            printf("Hospital C does not have the location ​%d​ in map\n",clientLocation);
            printf("Hospital C has sent \"location not found\" to the Scheduler\n");
        }
        else{
            printf("Hospital C has received input from client at location ​%d\n",clientLocation);
        }
    
        if(availability>0){
            printf("Hospital C has capacity = ​%d, occupation= ​%d​, availability = %f\n",totalCapacity,initialOccupancy,availability);
        }
        else{
            printf("Hospital C has capacity = ​%d, occupation= ​%d​, availability = None\n",totalCapacity,initialOccupancy);
        }
        //        calculate shortest distance between hospital and client if they are not at the same location
        if(clientLocation!=locHos){
            symmetry(G,vertixNo,locHos,reLocHos,clientLocation,reDestination);
            distance = Dijkstra1(G,locHos,clientLocation,reLocHos , reDestination);  //the shortest distance between hospital C and client
            if(distance!=0){
                printf("Hospital C has found the shortest path to client, distance = ​%f\n",distance);
                
            }
            else
                printf("Hospital C has found the shortest path to client, distance = ​None\n");
        }
        else{
            printf("Hospital C has found the shortest path to client, distance = None.\n");   //if client at same location as hospital,distance=none
        }
        
    //    calculate final score of hospital C
        if(availability==0 || distance == 0){
            score = 0;
            printf("Hospital C has the score = ​None\n");
        }
        else{
            score = double (1/(distance*(1.1-availability)));
            printf("Hospital C has the score = ​%f\n",score);
        }
        //send score and distance
        string scrDis_str=to_string(score)+" "+to_string(distance)+" "+"32403";
        char *scrDis = (char*)scrDis_str.c_str();
        numbytes=0;
        if((numbytes=sendto(sock_fd, scrDis, strlen(scrDis), 0, (struct sockaddr *)&addr_host, len_host))==-1 ){
            perror("sendto error:");
            exit(1);
        }
        //display on screen message according to different situations
        if(score!=0 && distance!=0){
            printf("Hospital C has sent score = %f and distance = %f to the Scheduler.\n",score,distance);
        }
        else if(score==0 && distance!=0){
            printf("Hospital C has sent score = None and distance = %f to the Scheduler.\n",distance);
        }
        else if (score!=0 && distance==0){
            printf("Hospital C has sent score = %f and distance = None to the Scheduler.\n",score);
        }
        else printf("Hospital C has sent score = None and distance = None to the Scheduler.\n");
        
        //receive updated occupation and availability from scheduler
        
        if((numbytes = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len_serv))==-1){
            perror("recvfrom error:");
            exit(1);
        }
       
        for(int i=numbytes;i<MAXBUFLEN;i++){
            recv_buf[i]='\0';
        }
        
    //
        int occuNew;
        double avaNew;
        istringstream istr(recv_buf);//split the two elements
        istr>>occuNew>>avaNew;
        printf("Hospital C has been assigned to a client, occupation is updated to ​%d, availability is updated to ​%f\n",occuNew,avaNew);
        
        
    }
    close(sock_fd);
    return 0;
}

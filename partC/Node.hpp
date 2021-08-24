#include <iostream>
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/tcp.h>
#include <stack>
#include <climits>
#include <bits/stdc++.h>
#include "select.c"
#include "Protocol.hpp"
using namespace std;

bool NACK = false;
bool ACK = true;
namespace Net{
    struct iport_sock{
        string nib_ip = "";
        int nib_id = -1;
        int nib_port = -1;
        int nib_sock = -1;
    };
class Node
{
    private:
        int _id; //id
        std::map<int , iport_sock > _nibSockDetails;
        std::map<int , string > _foundedPath; //save the shortest paths that already found
        int _port; // my port
        string _ip; //my ip
        char buffer[1025];//
        struct sockaddr_in listener;
        int _sockfd; //server sock
        



    public:
        Node(int port):_id(-1), _port(port) ,_ip("10.0.0.")
        {
            //server side
            int sckt = socket(AF_INET, SOCK_STREAM, 0);
            if (sckt < 0 )
            {
                printf("Could not create Listening socket, exiting %d...\n",_id);
                exit(0);
            }

            memset(&listener, 0, sizeof(listener));

            listener.sin_family = AF_INET;
            listener.sin_addr.s_addr = INADDR_ANY;
            listener.sin_port = htons(_port);
            int e = bind(sckt, (struct sockaddr *)&listener, sizeof(listener));
            if ( e < 0)
            {
                perror("[-]Error in bind");
                exit(1);
            }
            if(listen(sckt, 512) == 0){
                // printf("[+]Listening....\n");

            }
            else{
                perror("[-]Error in listening");
                exit(1);
            }
            add_fd_to_monitoring(sckt);
            _sockfd = sckt;
            get_from_user();
        
        }
        ~Node()
        {

        }
        void get_from_user();
        bool my_connect(string ip, int port );
        bool send(int id ,uint len, string message);
        int discover(int id ,int parent ,string path);
        bool route(string path);
        bool peers();//print the nib
        int nodeServer();
        int server_rec_send(int fd);
        int relay(string path ,int dest,  string msg);
        int nack(string path);



    };
}

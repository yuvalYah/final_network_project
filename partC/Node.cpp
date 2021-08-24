#pragma once
#include "Node.hpp"

const int SIZE = 492;
namespace Net{

//global var
int sizeOfMinPath = INT_MAX; //After we fount some path we save his size 
string mypath = ""; //After we found some path we save the path
string myMsgRelay = ""; //The msg i whant to send

bool is_route_command = false; //If we done sava the path and we whant ro print "route,x" 
int nackNumber = 0; //Number of nack i get from my nib

    /** This function is on in the constructor
        how get input from the user.
    */
    void Node::get_from_user()
    {
        int ret, i;
        char buffer[1025];

        while(true)
        {        
            ret = 0;
            printf("waiting for input...\n");
            ret = wait_for_input();
            printf("fd: %d is ready. reading...\n", ret);
            if(ret == 3 ) nodeServer(); //some node wants to connect to me
            
            else if(ret > 3 ) server_rec_send(ret); // Some node send me msg

            else //The user entered input
            {
                bzero(&buffer ,sizeof(buffer));
                read(ret, buffer, 1025); //read the input

                string input(buffer);
                string command = input.substr(0, input.find(","));//the command
                string other = input.substr(input.find(",")+1); //Arguments
                
                if(command == "setid")//setid,x
                {
                    if(this->_id >= 0) cout<<"You have some id !! "<<endl;
                    else{
                        this->_id = stoi(other);
                        this->_ip += to_string(_id);
                        cout<<"ack"<<endl;
                    }
                }
                else if(command == "connect")
                {
                    if(this->_id == -1) cout<<"You need to setid first!!"<<endl;
                    else {
                        string ipStr = other.substr(0, other.find(":"));
                        int portStr = stoi(other.substr(other.find(":")+1));
                        my_connect(ipStr , portStr);
                    }
                }
                else if(command == "send")
                {
                    int id_tosend = stoi(other.substr(0,other.find(","))); // dest id
                    other = other.substr(other.find(",")+1); 
                    uint len = stoi(other.substr(0,other.find(","))); // msg len
                    string msg = other.substr(other.find(",")+1); //msg
                    nackNumber=0; //init the nack number to 0

                    send(id_tosend ,len ,msg ); //call our send function
                }
                else if(command == "route") 
                {
                    //init global var
                    sizeOfMinPath = INT_MAX;
                    mypath = "";
                    is_route_command = true;
                    nackNumber=0;

                    map<int,string >::iterator iterat  = _foundedPath.find(stoi(other));
                    map<int,struct iport_sock>::iterator iterat2  = _nibSockDetails.find(stoi(other));

                    if(iterat2 != _nibSockDetails.end()) // to the nib
                    {
                        cout<<"ack"<<endl;
                        cout<<_id<<"->"<<stoi(other);
                    }
                    else if(iterat != _foundedPath.end()) // if has path save ->just print
                    {
                        cout<<"ack"<<endl;
                        string path (_foundedPath[stoi(other)]);
                        for(int i = 0 ; i<path.length() ; i++)
                        {
                            if(path[i] == ',') cout<< "->";
                            else cout<<path[i];
                        }    
                    }
                    else  discover(stoi(other) , -1 , to_string(_id)); // When we dont have the path ->discover the path and print
                    cout<<endl;
                }
                else if(input == "peers\n") peers();
                else cout <<"Invalid input!"<<endl;  
            }  
        }
    }
    bool Node::my_connect(string ip, int port)
    {
        bool flag = true;
        char buf[512];
        socklen_t len;
        int sockfd;
        struct sockaddr_in servaddr; 

        sockfd = socket(AF_INET, SOCK_STREAM, 0); //open client sock
        if (sockfd == -1) flag = false; 
        
        bzero(&servaddr, sizeof(servaddr));

        servaddr.sin_family = AF_INET; 
        servaddr.sin_addr.s_addr = INADDR_ANY; 
        servaddr.sin_port = htons(port); 

        // connect the client socket to server socket 
        if (flag && ::connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) flag = false; 
        
        // function for sending to the server
        Net::Protocol msg{_id,0,0,4,""};
        const char* msg_protocol = (const char*)msg;

        if (flag && ::send(sockfd,msg_protocol, sizeof(Protocol), 0) == -1)  flag = false; 
    
        if(flag){ 
            bzero(&buf, sizeof(buf));
            int n = recv(sockfd, buf, sizeof(buf), 0);//recv ack msg

            Net::Protocol rec(buf);

            struct iport_sock details; //Update the nib in his details
            details.nib_ip = ip;
            details.nib_id = rec.get_srcID();
            details.nib_port = port;
            details.nib_sock = sockfd;

            _nibSockDetails[rec.get_srcID()]=details; //Seve him

            add_fd_to_monitoring(sockfd);
            cout<<"ack"<<endl;
            cout<<rec.get_srcID()<<endl;
        }
        else  cout<<"nack"<<endl;

        return ACK;
    }
        

    bool Node::route(string path) //1,2,3,4,5
    {
        //We have path and we serch the parent to return him route msg
        stack<int> pathStack;
        string copyPath(path);
        size_t pos = 0;
        int parent = -1;
        bool flag = false;

        while ((pos = copyPath.find(',')) != string::npos) {
            int element = stoi(copyPath.substr(0, pos));
            if(element == _id){
                flag = (pathStack.size() == 0)? true : false; //if size == 0 -> im the src node
                if(!flag) parent = pathStack.top(); //else do pop and get the parent of this node
                break;
            }
            else{
                pathStack.push(element);
                copyPath.erase(0, pos + 1);
            }
        }
        if(flag) //if this node is the src and now need ro do relay&send
        {
            size_t len = count(path.begin() , path.end() ,',') + 1 ;
            if(len < sizeOfMinPath) 
            {
                sizeOfMinPath = len;
                mypath = path;
            }
            // sleep(4);
            if(mypath == "") //didnt fount any path
            {
                cout<<"nack"<<endl;
            }
            else 
            {
                //relay msg
                int index = mypath.find_last_of(",");
                int dest = stoi(mypath.substr(index +1));
                _foundedPath[dest] = mypath; //update the path to dest

                if(is_route_command) // if we did all the discover and route for print the route 
                {
                    copyPath = mypath + ",";
                    pos = 0;
                    string str = "";
                    while ((pos = copyPath.find(',')) != string::npos) {
                        int element = stoi(copyPath.substr(0, pos));
                        str+= "->"+copyPath.substr(0, pos);
                        copyPath.erase(0, pos + 1);
                            
                    }
                    str = str.substr(str.find(">") +1);
                    cout<<"ack"<<endl;
                    cout<<str<<endl;    
                    
                    is_route_command = false;
                    return true;
                }

                char buf[512];
                string tempMypath(mypath); 
                tempMypath = tempMypath.substr(mypath.find(",")+1);
                int next = stoi(tempMypath.substr(0 , tempMypath.find(","))); //find the next node to relay
                
                string stringPath = to_string(dest)+"&"+ tempMypath + "_"; //string path look : "dest&path_" for us to send in payload
                int size = SIZE - stringPath.length();//update the size i can send in the payload 

                int trallMsg = (myMsgRelay.length()+size -1)/size;// Calculate the number of msg to send
                string other = myMsgRelay; //save the msg i want to send

                for(int i = 0 ; i < trallMsg ; i++){
                    string piece; //he save the piece in the msg to send
                    if( i == trallMsg-1) piece = other.substr(i*size);
                    else piece = other.substr(i*size , size); 

                    Net::Protocol raley_msg{_id, next , (trallMsg -1-i) , 64 ,(stringPath + piece).c_str()}; //relay msg
                    string str = to_string(raley_msg.get_msgID()) + "," + to_string(_id) + ","+to_string(next) +","+to_string(trallMsg -1-i)+",64,"+stringPath + piece;

                    if (::send(_nibSockDetails[next].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;

                } //recv
                recv(_nibSockDetails[next].nib_sock, buf, sizeof(buf), 0);  
                Net::Protocol rec(buf);

                cout<<"ack"<<endl; //ack print
                return true;
            }
            return true;
        }
        //else send route msg
        Net::Protocol msg{_id,parent,0,16, path.c_str() };
        string str = to_string(msg.get_msgID()) + "," + to_string(_id) + ","+to_string(parent) +",0,16," + path ;

        if (::send(_nibSockDetails[parent].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;
        return true;

    }
    bool Node::peers()//print the nib
    {
        string str ="";
        map<int,struct iport_sock >::iterator it ; 
        for (it = _nibSockDetails.begin(); it != _nibSockDetails.end(); it++)
        {
            if(it->first != _id) str+="," + to_string(it->first);
        }
        cout<<"ack"<<endl;
        cout << str.substr(str.find(",") + 1 )<<endl;
        return true;

    }
    int Node::nodeServer() //Some node want to connect to me
    {
        char buf[512];
        socklen_t addr_size = sizeof(struct sockaddr_in);
        struct sockaddr_in new_addr;

        int new_sock = accept(_sockfd, (struct sockaddr*)&new_addr, &addr_size);
        if(new_sock < 0){
            perror("[-]Error in listening");
            exit(1);
        }
        bzero(&buf, sizeof(buf));

        int n = recv(new_sock, buf, sizeof(buf), 0); //recv the connect msg

        Net::Protocol from(buf);
        Net::Protocol msg{this->_id , from.get_srcID() , 0 , 1, "1"};

        const char* msg_protocol = (const char*)msg;
        ::send(new_sock,msg_protocol, sizeof(Protocol), 0); //send ack msg
        
        struct iport_sock details;
        details.nib_ip = " ";
        details.nib_id = from.get_srcID();
        details.nib_port = 0;
        details.nib_sock = new_sock;

        _nibSockDetails[from.get_srcID()]=details; //save him in my nib

        add_fd_to_monitoring(new_sock); //add to monitoring
        
    }
    bool Node::send(int id ,uint len, string message) //send msg to some node
    {
        nackNumber = 0;//init nacknum
        is_route_command = false; //init thus flag
        int trallMsg = (message.length() + SIZE - 1) / SIZE; // round up division
        string other = message.substr(0,len); // the msg len should be the len that user write

        map<int,struct iport_sock >::iterator it  = _nibSockDetails.find(id);
        if(it != _nibSockDetails.end()) // -> id is my neighbor , just send 
        { 
            char buf[512];
            for(int i = 0 ; i < trallMsg ; i++){ 

                string piece ; //if the msg to big so send in pieces
                if(i == trallMsg -1 ) piece = other.substr(i*SIZE); 
                else piece = other.substr(i*SIZE ,SIZE);
                
                Net::Protocol msg{_id,id, (trallMsg-1-i) ,32,piece.c_str()};
                string str = to_string(msg.get_msgID()) + "," + to_string(_id) + ","+to_string(id) +","+to_string(trallMsg-1-i) +",32," + piece;

                if (::send(_nibSockDetails[id].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;
                
                bzero(&buf, sizeof(buf));
                recv(_nibSockDetails[id].nib_sock, buf, sizeof(buf), 0);   //recv ack msg from node(id)

                Net::Protocol rec(buf);
            }
            cout<<"ack"<<endl; //print ack to the user
            return true;

        }
        map<int,string >::iterator iterat  = _foundedPath.find(id);
        if(iterat != _foundedPath.end())  //if not my nib and i sended him msg 
        {
            char buf[512];
            string tempMypath(_foundedPath[id]);
            tempMypath = tempMypath.substr(tempMypath.find(",")+1);//short the path (if the path is 1,2,3 now the path is 2,3 when 1 is the src,3 is dest)
            int next = stoi(tempMypath.substr(0 , tempMypath.find(","))); //find the next in the path to send

            string stringPath = to_string(id)+"&"+ tempMypath + "_";//string path to send in the payload
            int size = SIZE - stringPath.length(); //the new size

            int trallMsg = (message.length() + size -1)/size; //Calculate the number of msg
            string other = message;

            for(int i =0 ; i < trallMsg ; i++){
                string piece;

                if( i == trallMsg -1 ) piece = other.substr(i*size);
                else piece = other.substr(i*size , size);

                Net::Protocol raley_msg{_id, next , trallMsg -i-1 , 64 ,(stringPath + piece).c_str()}; //relay msg
                string str = to_string(raley_msg.get_msgID()) + "," + to_string(_id) + ","+to_string(next) +",1,64,"+stringPath + piece;
        
                if (::send(_nibSockDetails[next].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;
            }
                //recv
            recv(_nibSockDetails[next].nib_sock, buf, sizeof(buf), 0);  
            Net::Protocol rec(buf);
            return true;
        }
        //else if i didnt have the path or the dest isnt my nib
        myMsgRelay = message; //save the msg in global var
        sizeOfMinPath = INT_MAX; // init the global var
        mypath = ""; //init mypath
        
        // if id not neighboor -> send discover msg to all my neihboors 
        for (it = _nibSockDetails.begin(); it != _nibSockDetails.end(); it++)
        {
            //1 to all nib send discover msg
            Net::Protocol discover_msg{_id, it->first , 0 , 8 ,(to_string(id)+","+to_string(_id)).c_str()}; //discover msg

            string str = to_string(discover_msg.get_msgID()) + "," + to_string(_id) + ","+to_string(it->first) +",0,8," + to_string(id) + "," + to_string(_id);
            if (::send(_nibSockDetails[it->first].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;
        }
        return true;
    }

    //prevnt send discover msg to the src id 
    //path its string with save the path
    int Node::discover(int id ,int parent ,string path)
    {
        nackNumber=0;//init the nack number
        map<int,struct iport_sock >::iterator it  = _nibSockDetails.find(id);
        //if we found the dest then its send route msg
        if(it != _nibSockDetails.end()){
            route(path +","+to_string(id));
            return true;
        }
        char buf[512];
        if(_nibSockDetails.size() <= 1)  //the currnt node not hava another neighbors accept his parent -> send nack (leaf)
        {
            Net::Protocol nack_msg(_id, parent , 0 , 2 ,path.c_str()); //nack msg
            string str = to_string(nack_msg.get_msgID()) + "," + to_string(_id) + ","+to_string(parent) +",0,2," + path;

            if (::send(_nibSockDetails[parent].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;
            return true;
        }
        for (it = _nibSockDetails.begin(); it != _nibSockDetails.end(); it++)
        {
            //1 to all nib send discover msg
            if(it->first != parent){
                Net::Protocol discover_msg(_id, it->first , 0 , 8 , ( to_string(id)+","+path).c_str() ); //discover msg
                string str = to_string(discover_msg.get_msgID()) + "," + to_string(_id) + ","+to_string(it->first) +",0,8," +  to_string(id) + ","+path;

                if (::send(_nibSockDetails[it->first].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;
            }
        }
    }
    int Node::server_rec_send(int fd) //recv msg
    {
        char buf[512];
        bzero(&buf, sizeof(buf));

        int n = recv(fd, buf, sizeof(buf), 0);
        if(n < 0) cout<<"Error in recv!\n";
        if(n == 0 ){ // if some node log out
            remove_fd_from_monitoring(fd);

            map<int , struct iport_sock > ::iterator it;
            int idToDelete = -1;
            //delete The detached node from the nib list
            for(it = _nibSockDetails.begin() ; it != _nibSockDetails.end(); it++)
            {
                if(fd == _nibSockDetails[it->first].nib_sock)
                {
                    idToDelete = it->first ;
                    _nibSockDetails.erase(it->first);
                    break;
                }
            }
            map<int , string > ::iterator it2;;
            //remove the paths with this node
            for (auto it2 = _foundedPath.cbegin(); it2 != _foundedPath.cend() /* not hoisted */; /* no increment */)
            {
                size_t find =it2->second.find(to_string(idToDelete)) ;

                if (find != string::npos) _foundedPath.erase(it2++); 
                else ++it2; 
            }
            return 1;
        }
        Net::Protocol recvMsg(buf);
        if(recvMsg.get_funcID() == 2 )//nack
        {
            
            nackNumber++;//count the nack number 
            nack(recvMsg.get_payload());
        }
        if(recvMsg.get_funcID() == 8) //discover
        {
            string msg = recvMsg.get_payload();
            int psik = msg.find(",");
            discover(stoi(msg.substr(0 , psik ) ) , recvMsg.get_srcID() , msg.substr(psik+1) +','+ to_string(_id));
        }
        if(recvMsg.get_funcID() == 32) //send
        {
            string msgid = to_string(recvMsg.get_msgID());
            Net::Protocol ackMsg{this->_id , recvMsg.get_srcID() , 0 , 1, msgid.c_str()};

            const char* msg_protocol = (const char*)ackMsg;
            ::send(fd,msg_protocol, sizeof(Protocol), 0);

            cout<<recvMsg.get_payload()<<endl;//print the msg
        }
        if(recvMsg.get_funcID() == 16) // roure
        {
            route(recvMsg.get_payload());
        }
        if(recvMsg.get_funcID() == 64) //relay 4&1,2,3,4_this is my msg
        {
            //send back ack msg 
            string msgid = to_string(recvMsg.get_msgID());
            Net::Protocol ackMsg{this->_id , recvMsg.get_srcID() , 0 , 1, msgid.c_str()};

            const char* msg_protocol = (const char*)ackMsg;
            ::send(fd,msg_protocol, sizeof(Protocol), 0);

            //to func
            int dest = stoi(recvMsg.get_payload().substr(0 , recvMsg.get_payload().find("&")));//Extract the dest from the payload

            string rec = recvMsg.get_payload().substr(recvMsg.get_payload().find("&")+1 );
            int ind = rec.find("_");
            string message = rec.substr(ind+1);//Extract the msg from the payload
            string path = rec.substr(0,ind);//Extract the path from the payload

            relay(message , dest , path);  //do relay
        }     
    }
    int Node::relay(string msg ,int dest, string path)
    {
        string temp = path.substr(path.find(to_string(_id)));
        temp = temp.substr(temp.find(",")+1);
        cout<<path<<endl;
        int next = stoi(temp.substr(0,temp.find(",")));//the next to relay
        int src = stoi(path.substr(0,path.find(",")));//the sender of this msg

        char buf[512];
        bzero(&buf, sizeof(buf));

        if(_nibSockDetails.find(next) == _nibSockDetails.end())
        {
            //send,nack msg
            cout<<"nack"<<endl;
            return true;
        }
        if(next == dest)
        {
            //send msg
            Net::Protocol send_msg{src,dest,0,32,msg.c_str()};
            string str = to_string(send_msg.get_msgID()) + "," + to_string(src) + ","+to_string(dest) +",0,32," + msg;

            if (::send(_nibSockDetails[dest].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;
                
        }
        else{
            //raley msg
            string tempMypath(path);
            tempMypath = tempMypath.substr(path.find(",")+1);
            // int next = stoi(tempMypath.substr(0 , tempMypath.find(",")));

            Net::Protocol raley_msg{_id, next , 1 , 64 ,(to_string(dest) + "&"+ tempMypath + "_" + msg).c_str()}; //raley msg
            string str = to_string(raley_msg.get_msgID()) + "," + to_string(_id) + ","+to_string(next) +",1,64," +to_string(dest) + "&"+ tempMypath + "_" + msg;
            
            if (::send(_nibSockDetails[next].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;  
        }

        //ack recv
        recv(_nibSockDetails[next].nib_sock, buf, sizeof(buf), 0);   //1,2,3,4,5
        Net::Protocol rec(buf);
        // cout<<"Ack"<<endl;
      
    }
    int Node::nack(string path) //nack function to "relay" nack msg to src
    {
        stack<int> pathStack;
        string copyPath(path);
        size_t pos = 0;
        int parent = -1;
        bool flag = false;

        while ((pos = copyPath.find(',')) != string::npos) {
            int element = stoi(copyPath.substr(0, pos));
            if(element == _id){
                flag = (pathStack.size() == 0)? true : false;
                if(!flag) parent = pathStack.top();
                break;
            }
            else{
                pathStack.push(element);
                copyPath.erase(0, pos + 1);
            }
        }
        if(flag ){
            if(nackNumber == _nibSockDetails.size() && mypath=="") cout<< "nack"<<endl;
        }
        else if (!flag && nackNumber == _nibSockDetails.size()-1) {
            Net::Protocol msg{_id,parent,0,2, path.c_str() };
            string str = to_string(msg.get_msgID()) + "," + to_string(_id) + ","+to_string(parent) +",0,2," + path ;

            if (::send(_nibSockDetails[parent].nib_sock , str.c_str() , sizeof(Protocol), 0) < 0)  cout<<"nack"<<endl;
        }
        return true;

    }
  

}

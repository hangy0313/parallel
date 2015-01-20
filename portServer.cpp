#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

#define portServerPort 7013

struct portTable
{
    int portNumber;
    string processName;
};

class connection
{
private:
    int sockfd;
    int length;
    int recfd;
    struct sockaddr_in myAddr;
    struct sockaddr_in clientAddr;
public:
    void socketConnect();
    bool socketAccept();
    bool sendPortNum(int num);
    bool sendMsg(string msgString);
    string recieveMsg();
    void socketCloseRecfd();
};

class service
{
private:
    portTable *pt;
    int totalCount, downCount;
    connection *conn;
    string msgString;
public:
    service();
    void dispatch();
    void allocate();
    void reclaim();
};


int main()
{
    service serve;
    
    serve.dispatch();
    
    return 0;
}

//connection
void connection::socketConnect()
{
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket Error!!!");
        
        return;
    }
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(portServerPort);
    if(bind(sockfd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0){
        perror("Bind Error!!!");
        
        return;
    }
    if(listen(sockfd, 1) < 0){
        perror("Listen Error!!!");
        
        return;
    }
    length = sizeof(clientAddr);
    cout << "Server Start" << endl;
}

bool connection::socketAccept()
{
    if((recfd = accept(sockfd, (struct sockaddr*)&clientAddr, (socklen_t *)&length)) < 0){
        cout << "Accept Error" << endl;
        
        return false;
    }else{
        cout << "Accept Success" << endl;
        
        return true;
    }
}

bool connection::sendPortNum(int num)
{
    if(write(recfd, (void*)&num, sizeof(num)) < 0){
        cout << "Server write error";
        
        return false;
    }else{
        return true;
    }
}

bool connection::sendMsg(string msgString)
{
    char msg[50];
    
    strcpy(msg, msgString.c_str());
    
    if(write(recfd, msg, sizeof(msg)) < 0){
        cout << "Server write error";
        
        return false;
    }else{
        return true;
    }
}

string connection::recieveMsg()
{
    char msg[50];
    
    if(read(recfd, msg, sizeof(msg)) < 0){
        cout << "server get msg error" << endl;
    }
    string msgString(msg);
    
    return msgString;
}

void connection::socketCloseRecfd()
{
    close(recfd);
}

//service
service::service()
{
    conn = new connection();
    conn->socketConnect();
    
    totalCount = 10;
    pt = new struct portTable[10];
    for(int i = 0;i < totalCount;i++){
        pt[i].portNumber = i + 7070;
        pt[i].processName = "";
    }
}

void service::dispatch()
{
    while(1){
        if(!conn->socketAccept()){
            return;
        }
        
        msgString = conn->recieveMsg();
        cout << "server get msg: " << msgString << endl;
        
        if(msgString == "allocate"){
            allocate();
        }
        if(msgString == "reclaim"){
            reclaim();
        }
        
        for(int i = 0;i < totalCount;i++){
            cout << pt[i].processName << " " <<  pt[i].portNumber << endl;
        }
        
        conn->socketCloseRecfd();
    }

}

void service::allocate()
{
    cout << "=============allocate============" << endl;
    
    msgString = conn->recieveMsg();
    for(int i = 0;i < totalCount;i++){
        if(pt[i].processName == ""){
            pt[i].processName = msgString;
            conn->sendPortNum(pt[i].portNumber);
            break;
        }
    }
}

void service::reclaim()
{
    cout << "===========reclaim==============" << endl;
    
    msgString = conn->recieveMsg();
    for(int i = 0;i < totalCount;i++){
        if(pt[i].processName == msgString){
            pt[i].processName = "";
            break;
        }
    }
}

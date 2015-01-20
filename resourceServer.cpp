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

#define resourceServerPort 7014
#define numberOfsResource 10

struct resourceTable
{
    string name, ip, path;
    int port;
    bool flag;
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
    int recievePortNum();
    void socketCloseRecfd();
};

class service
{
private:
    resourceTable *rt;
    connection *conn;
    string msgString;
public:
    service();
    void dispatch();
    bool allocate();
    bool registerResource();
    bool reclaimResource();
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
    myAddr.sin_port = htons(resourceServerPort);
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

int connection::recievePortNum()
{
    int num;
    
    if(read(recfd, (void*)&num, sizeof(num)) < 0){
        cout << "Server write error";
        
        return 0;
    }else{
        return num;
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
    
    rt = new struct resourceTable[numberOfsResource];
    for(int i = 0;i < numberOfsResource;i++){
        rt[i].name = "";
        rt[i].ip = "";
        rt[i].port = 0;
        rt[i].path = "";
        rt[i].flag = false;
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
        
        if(msgString == "registerResource"){
            registerResource();
        }
        if(msgString == "allocate"){
            allocate();
        }
        if(msgString == "reclaimResource"){
            reclaimResource();
        }
        for(int i = 0;i < numberOfsResource;i++){
            if(rt[i].name != ""){
                cout << rt[i].name << " ";
            }else{
                cout << "NULL ";
            }
            if(rt[i].flag){
                cout << "Used" << endl;
            }else{
                cout << "Unused" << endl;
            }
        }
        conn->socketCloseRecfd();
    }

}

bool service::allocate()
{
    string name;
    char msg[50];
    
    cout << "=============allocate============" << endl;
    
    name = conn->recieveMsg();
    
    for(int i = 0;i < numberOfsResource;i++){
        if(rt[i].name == name && !rt[i].flag){
            rt[i].flag = true;
            
            strcpy(msg, rt[i].ip.c_str());
            conn->sendMsg(msg);
            conn->sendPortNum(rt[i].port);
            strcpy(msg, rt[i].path.c_str());
            conn->sendMsg(msg);
            
            return true;
        }
    }
    
    return false;

}

bool service::registerResource()
{
    string name, ip, path;
    int port, insertIndex = -1;
    
    cout << "===========registerResource==============" << endl;
   
    name = conn->recieveMsg();
    ip = conn->recieveMsg();
    port = conn->recievePortNum();
    path = conn->recieveMsg();
    
    for(int i = 0;i < numberOfsResource;i++){
        if(insertIndex == -1 && rt[i].name == ""){
            insertIndex = i;
        }
        if(rt[i].ip == name){
            
            return false;
        }
    }
    
    if(insertIndex != -1){
        rt[insertIndex].name = name;
        rt[insertIndex].ip = ip;
        rt[insertIndex].port = port;
        rt[insertIndex].path = path;
    }
    
    return true;
}

bool service::reclaimResource()
{
    string name;
    
    cout << "=============reclaimResource============" << endl;
    
    name = conn->recieveMsg();
    
    for(int i = 0;i < numberOfsResource;i++){
        if(rt[i].name == name){
            rt[i].flag = false;

            return true;
        }
    }
    
    return false;
}

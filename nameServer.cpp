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

#define numOfDir 10

struct directory
{
    char name[30], ip[30];
    int port;
}dir[numOfDir];

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
    bool sendDir(directory &dir);
    void recieveDir(int index);
    bool sendMsg(string msgString);
    string recieveMsg();
    void socketCloseRecfd();
};

class service
{
private:
    int dirIndex;
    connection *conn;
    string msgString;
public:
    service();
    void dispatch();
    void registerIp();
    void query();
    void deleteIp();
};

void showDirInfo();

int main()
{
    service serve;
    
    for(int i = 0;i < numOfDir;i++){
        string tmp = "";
        strcpy(dir[i].name, tmp.c_str());
        strcpy(dir[i].ip, tmp.c_str());
        dir[i].port = 0;
    }
    
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
    myAddr.sin_port = htons(7012);
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

bool connection::sendDir(directory &dir)
{
    cout << "send info: " << endl
         << "name: " << dir.name << endl
         << "ip: " << dir.ip << endl
         << "port: " << dir.port << endl;
    
    if(write(recfd, (void*)&dir, sizeof(dir)) < 0){
        cout << "Server write error";
        
        return false;
    }else{
        return true;
    }
}

void connection::recieveDir(int index)
{
    if(read(recfd, (void*)&dir[index], sizeof(directory)) < 0){
        cout << "server get msg error" << endl;
    }
    
    return;
}

void connection::socketCloseRecfd()
{
    close(recfd);
}

//service
service::service()
{
    dirIndex = 0;
    conn = new connection();
    conn->socketConnect();
}

void service::dispatch()
{
    while(1){
        if(!conn->socketAccept()){
            return;
        }
        
        msgString = conn->recieveMsg();
        cout << "server get msg: " << msgString << endl;
        
        if(msgString == "registerIp"){
            registerIp();
        }
        if(msgString == "query"){
            query();
        }
        if(msgString == "deleteIp"){
            deleteIp();
        }
        conn->socketCloseRecfd();
    }

}

void service::registerIp()
{
    cout << "=============registerIp============" << endl;
    
    conn->recieveDir(dirIndex);
    dirIndex++;
    
    showDirInfo();
}

void service::query()
{
    cout << "===========query==============" << endl;
    
    msgString = conn->recieveMsg();
    
    for(int i = 0;i < numOfDir;i++){
        if(msgString == dir[i].name){
            conn->sendDir(dir[i]);
            break;
        }
    }
}

void service::deleteIp()
{
    cout << "===========deleteIp==============" << endl;
    
    msgString = conn->recieveMsg();
    
    for(int i = 0;i < numOfDir;i++){
        if(msgString == dir[i].name){
            string tmp = "";
            strcpy(dir[i].name, tmp.c_str());
            strcpy(dir[i].ip, tmp.c_str());
            dir[i].port = 0;
            break;
        }
    }
    
    showDirInfo();
}

void showDirInfo()
{
    
    for(int i = 0;i < numOfDir;i++){
        string tmp(dir[i].name);
        if(tmp == ""){
            cout << "NULL" << endl;
        }else{
            cout << dir[i].name << " " << dir[i].ip << " " << dir[i].port << endl;
        }
    }
}

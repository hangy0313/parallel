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
#include <sstream>

using namespace std;

#define portServerPort 7013
#define resourceServerPort 7014
#define resourceServerIp "140.117.156.152"

struct directory
{
    string name, ip, path;
    int port;
} dir = {"", "127.0.0.1", "", 0};

class connection
{
private:
    int sockfd;
    int length;
    int recfd;
    char serverIp[15];
    int port;
    struct sockaddr_in myAddr;
    struct sockaddr_in clientAddr;
    struct sockaddr_in serverAddr;
public:
    bool sendMsg(string msgString);
    string recieveMsg();
    void socketCloseRecfd();
    void requestConnect();
    void setIp(string ip);
    void setPort(int num);
    void requestForResource();
    void requestForExecute();
};

void getResource();
void execute();

int main(int argc, char *argv[])
{
    string name(argv[1]);
    string cmd, exeName;
    char cmdChar[100];
    
    dir.name = name;
    
    getResource();
    
    cmd = "scp ./";
    cmd.append(dir.name);
    cmd.append(".cpp ");
    cmd.append("hangy@");
    cmd.append(dir.ip);
    cmd.append(":");
    cmd.append(dir.path);
    cout << cmd << endl;
    strcpy(cmdChar, cmd.c_str());
    system(cmdChar);
    
    execute();
    
    return 0;
}

//connection

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

void connection::requestConnect()
{
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket Error!!!");
        
        return;
    }
    
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(0);
    if(bind(sockfd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0){
        perror("Bind Error!!!");
        
        return;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);
    
    if(connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("connect error!!!\n");
        
        return;
    }else{
        printf("connect success\n");
    }
}

void connection::socketCloseRecfd()
{
    close(recfd);
}

void connection::setIp(string ip)
{
    strcpy(serverIp, ip.c_str());
    
    return;
}
void connection::setPort(int num)
{
    port = num;
}

void connection::requestForResource()
{
    char msg[50];
    string msgString = "allocate";
    
    strcpy(msg, msgString.c_str());
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    
    strcpy(msg, dir.name.c_str());
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    
    if(read(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    string ip(msg);
    dir.ip = ip;
    
    if(read(sockfd, (void*)&dir.port, sizeof(dir.port)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    
    if(read(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    string path(msg);
    dir.path = path;

    close(sockfd);
}

void connection::requestForExecute()
{
    char msg[50];
    string msgString = "execute";
    
    strcpy(msg, msgString.c_str());
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    
    close(sockfd);
}


void getResource()
{
    connection conn;
    conn.setIp(resourceServerIp);
    conn.setPort(resourceServerPort);
    conn.requestConnect();
    conn.requestForResource();
}

void execute()
{
    connection conn;
    conn.setIp(dir.ip);
    conn.setPort(dir.port);
    conn.requestConnect();
    conn.requestForExecute();
}

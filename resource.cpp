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
} dir = {"project3", "", "", 0};

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
    void socketConnect();
    bool socketAccept();
    bool sendMsg(string msgString);
    string recieveMsg();
    void socketCloseRecfd();
    void requestConnect();
    void setIp(string ip);
    void setPort(int num);
    void requestPort();
    void requestForRegister();
    void requestForRemove();
};

void requestPortNumber();
void registerResource();
void wait();
void execute();
void freeResource();
string intTostr(int &i);

int main(int argc, char *argv[])
{
    string name(argv[1]);
    string ip(argv[2]);
    string path(argv[3]);
    
    dir.name = name;
    dir.ip = ip;
    dir.path = path;
    
    requestPortNumber();
    registerResource();
    
    while(1){
        wait();
    }
    
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
    myAddr.sin_port = htons(dir.port);
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

void connection::requestPort()
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
    
    if(read(sockfd, (void*)&dir.port, sizeof(dir.port)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    close(sockfd);
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

void connection::requestForRegister()
{
    char msg[50];
    string msgString = "registerResource";
    
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
    
    strcpy(msg, dir.ip.c_str());
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    
    if(write(sockfd, (void*)&dir.port, sizeof(dir.port)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    
    strcpy(msg, dir.path.c_str());
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    
    close(sockfd);
}

void connection::requestForRemove()
{
    char msg[50];
    string msgString = "reclaimResource";
    
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
    
    close(sockfd);
}


void requestPortNumber()
{
    connection conn;
    conn.setIp("127.0.0.1");
    conn.setPort(portServerPort);
    conn.requestConnect();
    conn.requestPort();
}

void registerResource()
{
    connection conn;
    conn.setIp(resourceServerIp);
    conn.setPort(resourceServerPort);
    conn.requestConnect();
    conn.requestForRegister();
}

void wait()
{
    connection conn;
    conn.socketConnect();
    while(1){
    if(!conn.socketAccept()){
        return;
    }
    
    string msgString = conn.recieveMsg();
    cout << "server get msg: " << msgString << endl;
    if(msgString == "execute"){
        //conn.socketCloseRecfd();
        execute();
    }
    }
}

void execute()
{
    string cmd;
    char cmdChar[100];
    
    cmd = "g++ -o ";
    cmd.append(dir.name);
    cmd.append(" ");
    cmd.append(dir.name);
    cmd.append(".cpp");
    strcpy(cmdChar, cmd.c_str());
    cout << cmdChar << endl;
    system(cmdChar);
    
    cmd = dir.path;
    cmd.append(dir.name);
    //cmd.append(" ");
    //string portString = intTostr(dir.port);
    //cmd.append(portString);
    strcpy(cmdChar, cmd.c_str());
    cout << cmdChar << endl;
    system(cmdChar);
    
    freeResource();
}

void freeResource()
{
    connection conn;
    conn.setIp(resourceServerIp);
    conn.setPort(resourceServerPort);
    conn.requestConnect();
    conn.requestForRemove();
}

string intTostr(int &i)
{
    string s;
    stringstream ss(s);
    ss << i;
    
    return ss.str();
}
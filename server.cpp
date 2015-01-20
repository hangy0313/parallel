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
#include <sstream>
#include <cstring>

using namespace std;

#define nameServerIp "127.0.0.1"
#define nameServerPort 7012

struct customer
{
    int totalMoney;
    string userName, password;
};

struct directory
{
    char name[30], ip[30];
    int port;
} dir = {"project3_server", "127.0.0.1", 7099};

class connection
{
private:
    int sockfd;
    int length;
    int recfd;
    struct sockaddr_in myAddr;
    struct sockaddr_in clientAddr;
    struct sockaddr_in serverAddr;
public:
    void socketConnect();
    bool socketAccept();
    bool sendMsg(string msgString);
    string recieveMsg();
    void socketCloseRecfd();
    void registerIp();
    void registerConnect();
    bool registerDir();
    void deleteIp();
};

class service
{
private:
    customer cus;
public:
    service();
    string identify(string getUserName, string getPassword);
    string updateInfo(string getUserName, string getPassword);
    string addMoney(string getUserName, string getPassword, int money);
    string subMoney(string getUserName, string getPassword, int money);
};

class dispatch
{
private:
    connection *conn;
    connection *connForRegister;
    service *serve;
    int recfd;
    char msg[50];
    string msgString;
    bool check;
public:
    dispatch();
    void assignServe();
    void idetify();
    void sendServerState(string str);
    void signIn();
    void updateInfo();
    void addMoney();
    void subMoney();
};

string intTostr(int &i);

int main(int argc, char *argv[])
{
    dispatch d;
    d.assignServe();
    
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
    
    //close tcp time wait
    int opt=1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct linger li;
    li.l_onoff = 1;
    li.l_linger = 0;
    setsockopt (sockfd,SOL_SOCKET, SO_LINGER,(const char *)&li,sizeof (li));
    
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

void connection::socketCloseRecfd()
{
    close(recfd);
}

void connection::registerIp()
{
    registerConnect();
    registerDir();
    close(sockfd);
}

void connection::registerConnect()
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
    serverAddr.sin_port = htons(nameServerPort);
    serverAddr.sin_addr.s_addr = inet_addr(nameServerIp);
    
    if(connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("connect error!!!\n");
        
        return;
    }else{
        printf("connect success\n");
    }
}

bool connection::registerDir()
{
    string msgString = "registerIp";
    char msg[50];
    
    strcpy(msg, msgString.c_str());
    
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return false;
    }
    
    if(write(sockfd, (void*)&dir, sizeof(dir)) < 0){
        cout << "write error" << endl;
        
        return false;
    }
    
    return true;
}

void connection::deleteIp()
{
    registerConnect();

    string msgString = "deleteIp";
    char msg[50];
    
    strcpy(msg, msgString.c_str());
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }
    
    string tmp(dir.name);
    strcpy(msg, tmp.c_str());
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return;
    }

    
    close(sockfd);
}

//service
service::service()
{
    cus.userName = "test";
    cus.password = "1234";
    cus.totalMoney = 1000;
}

string service::identify(string getUserName, string getPassword)
{
    bool check = false;
    string msgString;
    
    //判斷帳號密碼正確性
    if(getUserName == cus.userName && getPassword == cus.password){
        check = true;
        cout << "userName and password is right" << endl;
    }else{
        cout << "userName and password is wrong" << endl;
    }
    
    if(check){
        msgString = "right";
    }else{
        msgString = "wrong";
    }
    
    return msgString;
}

string service::updateInfo(string getUserName, string getPassword)
{
    string msgString = identify(getUserName, getPassword);
    
    if(msgString == "right"){
        msgString = intTostr(cus.totalMoney);
    }else{
        msgString = "0";
    }
    
    return msgString;
}

string service::addMoney(string getUserName, string getPassword, int money)
{
    string msgString = identify(getUserName, getPassword);
    
    if(msgString == "right"){
        cus.totalMoney += money;
        msgString = intTostr(cus.totalMoney);
    }else{
        msgString = "0";
    }
    
    return msgString;
}

string service::subMoney(string getUserName, string getPassword, int money)
{
    string msgString = identify(getUserName, getPassword);
    
    if(msgString == "right"){
        cus.totalMoney -= money;
        msgString = intTostr(cus.totalMoney);
    }else{
        msgString = "0";
    }
    
    return msgString;
}

//dispatch
dispatch::dispatch()
{
    conn = new connection();
    connForRegister = new connection();
    serve = new service();
    
    connForRegister->registerIp();
    conn->socketConnect();
}

void dispatch::assignServe()
{
    while(1){
        if(!conn->socketAccept()){
            return;
        }
        
        msgString = conn->recieveMsg();
        cout << "server get msg: " << msgString << endl;
        
        if(msgString == "signIn"){
            signIn();
        }
        if(msgString == "updateInfo"){
            cout << "test";
            updateInfo();
        }
        if(msgString == "addMoney"){
            addMoney();
        }
        if(msgString == "subMoney"){
            subMoney();
        }
        conn->socketCloseRecfd();
        if(msgString == "leave"){
            break;
        }
    }
    connForRegister->deleteIp();
}

void dispatch::signIn()
{
    string getUserName, getPassword;
    
    conn->sendMsg("service: signIn");

    getUserName = conn->recieveMsg();
    getPassword = conn->recieveMsg();
    
    msgString = serve->identify(getUserName, getPassword);
    conn->sendMsg(msgString);
}
void dispatch::updateInfo()
{
    string getUserName, getPassword;
    
    conn->sendMsg("service: updateInfo");
    
    getUserName = conn->recieveMsg();
    getPassword = conn->recieveMsg();
    
    msgString = serve->updateInfo(getUserName, getPassword);
    conn->sendMsg(msgString);
}
void dispatch::addMoney()
{
    string getUserName, getPassword;
    int money;
    
    conn->sendMsg("service: addMoney");
    
    getUserName = conn->recieveMsg();
    getPassword = conn->recieveMsg();
    money = atoi(conn->recieveMsg().c_str());
    
    msgString = serve->addMoney(getUserName, getPassword, money);
    conn->sendMsg(msgString);
}
void dispatch::subMoney()
{
    string getUserName, getPassword;
    int money;
    
    conn->sendMsg("service: addMoney");
    
    getUserName = conn->recieveMsg();
    getPassword = conn->recieveMsg();
    money = atoi(conn->recieveMsg().c_str());
    
    msgString = serve->subMoney(getUserName, getPassword, money);
    conn->sendMsg(msgString);
}

string intTostr(int &i)
{
    string s;
    stringstream ss(s);
    ss << i;
    
    return ss.str();
}

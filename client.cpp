#include <stdio.h>
#include <stdlib.h>
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

#define nameServerIp "127.0.0.1"
#define nameServerPort 7012

bool stringToBool(string str);

struct customer
{
    int totlaMoney;
    string userName, password;
};

struct directory
{
    char name[30], ip[30];
    int port;
} dir = {"project3_server", "140.117.156.150", 7099};

class connection
{
private:
    int sockfd;
    int port;
    char serverIP[15];
    struct sockaddr_in myAddr;
    struct sockaddr_in serverAddr;
public:
    void socketConnect();
    bool sendMsg(string msgString);
    string recieveMsg();
    void recieveDir();
    void socketCloseSockfd();
    void setPort(int num);
    void setIp(string ip);
};

class proxy
{
private:
    connection *conn, *connNameServer;
    customer cus;
    bool checkSignIn;
    char msg[50];
    string msgString;
public:
    void initial();
    void closeConn();
    bool getCheckSignIn();
    void service(string serve);
    bool sendRequest(string serve);
    bool identify();
    void signIn();
    void updateInfo();
    void addMoney();
    void subMoney();
    void showCustomerInfo();
    void getServerIp();
};

class User
{
private:
    proxy *p;
public:
    User();
    void serviceList();
};

int main(int argc, char *argv[])
{
    User u;
    
    u.serviceList();
    
    return 0;
}

void connection::socketConnect()
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
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    
    if(connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("connect error!!!\n");
        
        return;
    }else{
        printf("connect success\n");
    }
}

bool connection::sendMsg(string msgString)
{
    char msg[50];
    
    strcpy(msg, msgString.c_str());
    
    if(write(sockfd, msg, sizeof(msg)) < 0){
        cout << "write error" << endl;
        
        return false;
    }
        
    return true;
}

string connection::recieveMsg()
{
    char msg[50] = {0};
    
    if(read(sockfd, msg, sizeof(msg)) < 0){
        cout << "read error" << endl;
    }
    
    string passString(msg);
    return passString;
}

void connection::recieveDir()
{
    directory tmpDir;
    
    if(read(sockfd, (void*)&dir, sizeof(dir)) < 0){
        cout << "read error" << endl;
    }
    
    return;
}

void connection::socketCloseSockfd()
{
    close(sockfd);
}

void connection::setPort(int num)
{
    port = num;
}

void connection::setIp(string ip)
{
    strcpy(serverIP, ip.c_str());
}

void proxy::initial()
{
    conn = new connection;
    conn->setPort(dir.port);
    conn->setIp(dir.ip);
    conn->socketConnect();
    checkSignIn = false;
}

bool proxy::getCheckSignIn()
{
    return checkSignIn;
}

void proxy::service(string serve)
{
    initial();
    if(serve.compare("signIn") == 0){
        signIn();
    }
    
    if(serve.compare("updateInfo") == 0){
        updateInfo();
    }
    
    if(serve.compare("addMoney") == 0){
        addMoney();
    }
    
    if(serve.compare("subMoney") == 0){
        subMoney();
    }
    if(serve.compare("leave") == 0){
        conn->sendMsg("leave");
    }
    conn->socketCloseSockfd();
    
    return;
}

bool proxy::sendRequest(string serve)
{
    //送出要的服務項目
    if(conn->sendMsg(serve)){
        cout << "client send request: " << serve << endl;
    }else{
        return false;
    }

    msgString = conn->recieveMsg();
    cout << "status: " << msgString << endl;
    
    return true;
}

bool proxy::identify()
{
    //傳遞帳號密碼
    if(conn->sendMsg(cus.userName)){
        cout << "client send userName: " << endl;
    }else{
        return false;
    }
    
    if(conn->sendMsg(cus.password)){
        cout << "client send password: " << endl;
    }else{
        return false;
    }
    
    return true;
}

void proxy::signIn()
{
    if(!sendRequest("signIn")){
        return;
    }
    
    cout << "userName: ";
    cin >> cus.userName;
    cout << "password: ";
    cin >> cus.password;
    
    if(!identify()){
        cout << "userName or password error" << endl;
        return;
    }

    //取得server的檢查結果
    msgString = conn->recieveMsg();
    cout << "userName & password are " << msgString << endl;
    
    if(msgString == "right"){
        checkSignIn = true;
    }else{
        checkSignIn = false;
    }
    
    return;
}

void proxy::updateInfo()
{
    if(!sendRequest("updateInfo")){
        return;
    }
    
    if(!identify()){
        return;
    }
    
    msgString = conn->recieveMsg();
    cout << "update successful" << endl;
    cus.totlaMoney = atoi(msgString.c_str());
    showCustomerInfo();
}

void proxy::addMoney()
{
    if(!sendRequest("addMoney")){
        return;
    }
    
    if(!identify()){
        return;
    }
    
    cout << "Add money:";
    cin >> msg;

    if(conn->sendMsg(msg)){
        cout << "client send request for add money " << msg << endl;
    }else{
        return;
    }
    msgString = conn->recieveMsg();
    cout << "add money successful" << endl;
    cus.totlaMoney = atoi(msgString.c_str());
    showCustomerInfo();
}

void proxy::subMoney()
{
    if(!sendRequest("subMoney")){
        return;
    }
    
    if(!identify()){
        return;
    }
    
    cout << "Sub money:";
    cin >> msg;

    if(conn->sendMsg(msg)){
        cout << "client send request for sub money " << msg << endl;
    }else{
        return;
    }
    msgString = conn->recieveMsg();
    cout << "sub money successful" << endl;
    cus.totlaMoney = atoi(msgString.c_str());
    showCustomerInfo();
}

void proxy::showCustomerInfo()
{
    cout << "totalMoney: " << cus.totlaMoney << endl;
}

void proxy::getServerIp()
{
    cout << "Get server's IP" << endl;
    connNameServer = new connection;
    connNameServer->setPort(nameServerPort);
    connNameServer->setIp(nameServerIp);
    
    connNameServer->socketConnect();
    
    connNameServer->sendMsg("query");
    connNameServer->sendMsg(dir.name);
    
    connNameServer->recieveDir();
    cout << "get server info: " << endl
         << "name: " << dir.name << endl
         << "ip: " << dir.ip << endl
         << "port: " << dir.port << endl;
    connNameServer->socketCloseSockfd();
}

User::User()
{
    p = new proxy;
}

void User::serviceList()
{
    bool first = true;
    int choose;
    char leave;
    
    p->getServerIp();
    
    while(1){
        if(first){
            p->service("signIn");
            if(p->getCheckSignIn()){
                first = false;
            }
            continue;
        }
        
        cout << "Choose service:" << endl
        << "1. update info" << endl
        << "2. add money" << endl
        << "3. sub money" << endl;
        cin >> choose;
        switch (choose) {
            case 0:
                p->service("siginIn");
                break;
            case 1:
                p->service("updateInfo");
                break;
            case 2:
                p->service("addMoney");
                break;
            case 3:
                p->service("subMoney");
                break;
            default:
                break;
        }
        cout << "Leave?";
        cin >> leave;
        if(leave == 'y' || leave == 'Y'){
            break;
        }
    }
    p->service("leave");
}


bool stringToBool(string str)
{
    return str != "0";
}

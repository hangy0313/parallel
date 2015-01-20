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

using namespace std;

struct customer
{
    int totalMoney;
    string userName, password;
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
    bool sendMsg(string msgString);
    string recieveMsg();
    void socketCloseRecfd();
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

int main()
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
    myAddr.sin_port = htons(7002);
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
        msgString = to_string(cus.totalMoney);
    }else{
        msgString = "0";
    }
    
    return msgString;
}

string service::addMoney(string getUserName, string getPassword, int money)
{
    string msgString = identify(getUserName, getPassword);
    
    if(msgString == "right"){
        msgString = to_string(cus.totalMoney + money);
    }else{
        msgString = "0";
    }
    
    return msgString;
}

string service::subMoney(string getUserName, string getPassword, int money)
{
    string msgString = identify(getUserName, getPassword);
    
    if(msgString == "right"){
        msgString = to_string(cus.totalMoney - money);
    }else{
        msgString = "0";
    }
    
    return msgString;
}

//dispatch
dispatch::dispatch()
{
    conn = new connection();
    serve = new service();
    
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
    }
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


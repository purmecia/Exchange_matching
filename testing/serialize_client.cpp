#include<iostream>
#include "tinyxml2.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#define BUF_SIZE 10000000
using namespace std;
using namespace tinyxml2;

int main(int argc,char* argv[]) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = inet_addr("152.3.65.138");  
    serv_addr.sin_port = htons(12345);

    char bufSend[BUF_SIZE] = {0};
    char bufRecv[BUF_SIZE] = {0};

    while(1){
        //创建套接字
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))!=0){
            printf("connect failed\n");
            return -1;
        } 
        std::string filepath;
        std::cout<<"Please input the file path (q to quit): "<<std::endl;
        std::cin>>filepath;
        if(filepath=="q"){
            break;
        }
        
        XMLDocument doc;

        if(doc.LoadFile(filepath.c_str())!=0)
        {
            std::cout<<"File not found!"<<std::endl;
            continue;
        }

        XMLPrinter printer;
        doc.Print( &printer );
        cout<<"xml content length:"<<printer.CStrSize()<<endl;
        cout<<"xml content:"<<printer.CStr()<<endl;
        
        ostringstream os;
        os<<printer.CStrSize()<<endl<<printer.CStr()<<endl;

        strcpy(bufSend, os.str().c_str());
        int n_sent = send(sock, bufSend, strlen(bufSend)+1, 0);
        printf("Message sent: %d bytes\n", n_sent);

        int n_recv = recv(sock, bufRecv, BUF_SIZE, 0);
        printf("Message received: %d bytes\n", n_recv);
        printf("Message received: %s\n", bufRecv);
        memset(bufSend, 0, BUF_SIZE);  
        memset(bufRecv, 0, BUF_SIZE);  

    }
    return 0;
}
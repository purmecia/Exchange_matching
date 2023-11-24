#include"db_func.hpp"
#include"request.hpp"
#include"ThreadPool.h"
#include<sys/socket.h>
#include <unistd.h> 
#include<arpa/inet.h> 
#include<thread>
#include<cstdlib>
std::vector<std::unique_ptr<std::thread> > threads;

class ClientHandler {
public:
    ClientHandler() {}
    void operator()(int Connect, sockaddr_in ClientAddr) {
        // std::cout<<"Client connected!"<<std::endl;
        pqxx::connection *C;
        try{
            //C = new pqxx::connection("host=db dbname=trades user=postgres password=passw0rd");
           // C = new pqxx::connection("host=db dbname=trades user=postgres password=passw0rd host=db port=5432");
            // C = new pqxx::connection("dbname=" + std::string(getenv("POSTGRES_DB")) + \
            // " user=" + std::string(getenv("POSTGRES_USER")) + \
            // " password=" + std::string(getenv("POSTGRES_PASSWORD")) + " " + \
            // "host=" + std::string(getenv("POSTGRES_HOST")) + " " + \
            // "port=" + std::string(getenv("POSTGRES_PORT")));
            C = new pqxx::connection("dbname=trades user=postgres password=passw0rd host=db port=5432");
            //C = new pqxx::connection("dbname=trades user=postgres password=passw0rd");
            
            if(C->is_open()){
                std::cout<<"Connected to database successfully: "<< C->dbname()<<std::endl;
            }else{
                std::cerr<<"Can't open database"<<std::endl;
                exit(EXIT_FAILURE);
            }
        }catch(const std::exception &e){
            std::cerr<<e.what()<<std::endl;
            exit(EXIT_FAILURE);
        }
        
        
        // //read request 
        std::vector<char> buf(10000);
        int bytes = recv(Connect, buf.data(), buf.size(), 0);
        
        //read request length
        std::string raw_str(buf.data(), bytes);
        // std::cout<<"Request raw request: "<<raw_str<<std::endl;
        size_t first_line_end = raw_str.find("\n");
        if (first_line_end == std::string::npos) {
            std::cerr << "Invalid request!" << std::endl;
            return;
        }
        int request_length = std::stoi(raw_str.substr(0, first_line_end));
        std::cout<<"Request length: "<<request_length<<std::endl;

        size_t request_start = first_line_end + 1;
        std::string request=raw_str.substr(request_start);
        int total_bytes=request.length();
        while(request_length>=total_bytes){
            std::vector<char> buf(10000);
            int bytes = recv(Connect, buf.data(), buf.size(), 0);
            request.append(buf.data(),bytes);
            if(bytes>0){
                total_bytes+=bytes;
            }
            else
                break;
        }
        // //print request
        //std::cout<<"Request received: "<<request<<std::endl;

        //read request ending
        //parse into xml

        // std::cout<<"Connect successfully"<<std::endl;
        tinyxml2::XMLDocument doc;
        // doc.LoadFile("./system_test/test_buy.xml");
        doc.Parse(request.c_str());
        //get root value as request type from xml
        std::string request_type = doc.RootElement()->Value();
        if(request_type=="create"){
            std::cout<<"Create request received!"<<std::endl;
            std::string resp=Parse_Create_Request(C,doc);
            std::cout<<"Response: "<<resp<<std::endl;
            //send back response
            send(Connect, resp.c_str(), resp.length(), 0);

        }
        else if(request_type=="transactions"){
            std::cout<<"Transactions request received!"<<std::endl;
            std::string resp=Parse_Transactions_Request(C,doc);
            std::cout<<"Response: "<<resp<<std::endl;
            send(Connect, resp.c_str(), resp.length(), 0);
        }
        else{
            std::cerr<<"Invalid request type!"<<std::endl;
            return;
        }
        close(Connect);
        if(C->is_open()){
            C->disconnect();
            std::cout << "Closed database successfully" << std::endl;
        }
      
    }    
};


int main(int argc, char *argv[]) {
    ThreadPool pool(50);
    int Connect;
    int Server = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(12345);
    ServerAddr.sin_addr.s_addr = INADDR_ANY;
    bind(Server, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
    listen(Server, SOMAXCONN);

    std::cout<<"Server is running..."<<std::endl;
    pqxx::connection *C;
    C = new pqxx::connection("dbname=trades user=postgres password=passw0rd host=db port=5432");
    //C = new pqxx::connection("dbname=trades user=postgres password=passw0rd");
    DropTable(C);
    CreateTable("./CreateTable.txt",C);
    std::string execution = "set transaction isolation level repeatable read;";
    ExecuteCommand(execution,C);
    // freopen("server_result.txt", "w", stdout); 
    while (true) {
        sockaddr_in ClientAddr;
        socklen_t client_len = sizeof(ClientAddr);
        Connect = accept(Server, (sockaddr*)&ClientAddr, &client_len);

        std::cout<<"New client connected!"<<std::endl;
        pool.enqueue((ClientHandler()), Connect,ClientAddr);

    }
    return 0;
}




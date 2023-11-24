#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <random>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <mutex>
#include "tinyxml2.h"
#define BUF_SIZE 1000000
using namespace tinyxml2;
using namespace std;

vector<std::string> symbol_names;
std::atomic<int64_t> latency(0);
std::mutex latency_mutex;

void init(int symbol_amount){
    for(int c=0;c<symbol_amount;c++){
        //random string
        std::string symbol_name = "";
        int length = 3;
        for(int i = 0; i < length; i++){
            symbol_name += (char) (rand()%26 + 'a');
        }

        symbol_names.push_back(symbol_name);
    }
}


void generate_create_xml(vector<long long> userid, tinyxml2::XMLDocument& doc){
    const char* declaration ="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    doc.Parse(declaration);
    tinyxml2::XMLElement *root = doc.NewElement("create");
    doc.InsertEndChild(root);
    for(int i = 0; i < userid.size(); i++){
        tinyxml2::XMLElement *account = doc.NewElement("account");
        account->SetAttribute("id", (uint64_t) userid[i]);
        //random balance
        double random_balance = (double) rand() / RAND_MAX * 1000000+1;
        account->SetAttribute("balance", random_balance);
        root->InsertEndChild(account);
    }
    //insert symbol
    for(auto symbol: symbol_names){
        tinyxml2::XMLElement *symbol_element = doc.NewElement("symbol");
        symbol_element->SetAttribute("sym", symbol.c_str());
        //set balance for each user of such symbol
        for(int i = 0; i < userid.size(); i++){
            tinyxml2::XMLElement *user = doc.NewElement("account");
            user->SetAttribute("id", (uint64_t) userid[i]);
            double random_balance = (double) rand() / RAND_MAX * 1000000+1;
            user->SetText(random_balance);
            symbol_element->InsertEndChild(user);
        }
        root->InsertEndChild(symbol_element);
    }

}

void generate_transactions_xml(long long user_id, int num_of_transactions, tinyxml2::XMLDocument& doc, int query_num, int cancel_num){
    const char* declaration ="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    doc.Parse(declaration);
    tinyxml2::XMLElement *root = doc.NewElement("transactions");
    root->SetAttribute("id", (uint64_t) user_id);

    doc.InsertEndChild(root);
    //add order part
    for(int i = 0; i < num_of_transactions; i++){
        tinyxml2::XMLElement *transaction = doc.NewElement("order");
        transaction->SetAttribute("sym", symbol_names[rand()%(symbol_names.size())].c_str());
        //random to be positive or negative
        int sign = rand()%2;
        if(sign == 0){
            transaction->SetAttribute("amount", (double) rand() / RAND_MAX * 1000+1);
        }else{
            transaction->SetAttribute("amount", (double) -rand() / RAND_MAX * 1000+1);
        }
        transaction->SetAttribute("limit", (double) rand() / RAND_MAX * 1000+1);
        root->InsertEndChild(transaction);
    }
    //add query part
    for(int i = 0; i < query_num; i++){
        tinyxml2::XMLElement *transaction = doc.NewElement("query");
        transaction->SetAttribute("id", (uint64_t) rand()%100);
        root->InsertEndChild(transaction);
    }
    //add cancel part
    for(int i = 0; i < cancel_num; i++){
        tinyxml2::XMLElement *transaction = doc.NewElement("cancel");
        transaction->SetAttribute("id", (uint64_t) rand()%100);
        root->InsertEndChild(transaction);
    }
}


void threadedSend(tinyxml2::XMLDocument& doc){
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = inet_addr("152.3.65.138");  
    serv_addr.sin_port = htons(12345);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))!=0){
        printf("connect failed\n");
        return ;
    } 

    char bufSend[BUF_SIZE] = {0};
    char bufRecv[BUF_SIZE] = {0};


    XMLPrinter printer;
    doc.Print( &printer );
    // cout<<"xml content length:"<<printer.CStrSize()<<endl;
    cout<<"xml content:"<<printer.CStr()<<endl;
    
    ostringstream os;
    os<<printer.CStrSize()<<endl<<printer.CStr()<<endl;

    strcpy(bufSend, os.str().c_str());
    int n_sent = send(sock, bufSend, strlen(bufSend)+1, 0);
    // printf("Message sent: %d bytes\n", n_sent);

    int n_recv = recv(sock, bufRecv, BUF_SIZE, 0);
    printf("Message received: %d bytes\n", n_recv);
    printf("Message received: %s\n", bufRecv);
    memset(bufSend, 0, BUF_SIZE);  
    memset(bufRecv, 0, BUF_SIZE);  
    close(sock); 

}

void randomize_test(int user_number, int transaction_number, int query_number, int cancel_number) {
    //srand(time(NULL));
    auto start = chrono::high_resolution_clock::now();
    vector<long long> user_id_set;
    for(int i = 0; i < user_number; i++){
        long long user_id = rand() % 1000000000 + 1;
        user_id_set.push_back(user_id);
    }
    XMLDocument create_doc;
    generate_create_xml(user_id_set, create_doc);
    // print create xml
    // XMLPrinter printer;
    // create_doc.Print( &printer );
    // cout<<"xml content:"<<printer.CStr()<<endl;
    cout<<"xsend create xml"<<endl;
    threadedSend(create_doc);
    for(auto id: user_id_set){
        XMLDocument transaction_doc;
        generate_transactions_xml(id, transaction_number, transaction_doc, query_number, cancel_number);
        // print transaction xml
        // XMLPrinter printer;
        // transaction_doc.Print( &printer );
        // cout<<"xml content:"<<printer.CStr()<<endl;
        cout<<"xsend transaction xml of "<<id<<endl;
        threadedSend(transaction_doc);
    }
    auto end = chrono::high_resolution_clock::now();
    long long request_latency = chrono::duration_cast<chrono::microseconds>(end - start).count();
    {
        std::unique_lock<std::mutex> lock(latency_mutex);
        latency += request_latency;
    }
}



int main(int argc, char* argv[]) {
    srand(time(NULL));
    
    if (argc < 6) {
        std::cerr << "Usage: " << argv[0] << " thread_num  user_num transaction_num symbol_num query_num cancel_num" << std::endl;
        return 1;
    }
    freopen("client_result.txt", "w", stdout); 
    init(std::stoi(argv[4]));
    std::vector<std::thread> threads;
    auto start =chrono::high_resolution_clock::now();
    int thread_num = std::stoi(argv[1]);
    int user_num = std::stoi(argv[2]);
    int transaction_num = std::stoi(argv[3]);
    int symbol_num = std::stoi(argv[4]);
    int query_num = std::stoi(argv[5]);
    int cancel_num = std::stoi(argv[6]);

    int total_request= thread_num * (1 + user_num);

    for (int i = 0; i < std::stoi(argv[1]); i++) {
        threads.emplace_back(randomize_test, std::stoi(argv[2]),std::stoi(argv[3]),std::stoi(argv[4]),std::stoi(argv[5]));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    std::cerr<<"overall running time = "<<(double)chrono::duration_cast<chrono::microseconds>(end-start).count()/1000000<<"s"<<endl;
    std::cerr<<"overall request = "<<total_request<<endl;
    std::cerr<<"average latency = "<<(double)latency/total_request/1000000<<"s"<<endl;
    std::cerr<<"overall throughput = "<<(double)total_request*1000000/chrono::duration_cast<chrono::microseconds>(end-start).count()<<"req/s"<<endl;

    return 0;
}

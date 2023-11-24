#ifndef __RESP_H__
#define __RESP_H__
#include<iostream>
#include<string>
#include <variant>
#include<sstream>
#include<vector>
#include "tinyxml2.h"
#include"db_func.hpp"
typedef struct account_error{
    long long account_id;
    std::string error_message;
    
}account_error;


typedef struct account_success{
    long long account_id;
}account_success;
typedef struct symbol_error{
    std::string symbol;
    long long account_id;
    std::string error_message;
}symbol_error;
typedef struct symbol_success{
    std::string symbol;
    long long account_id;
}symbol_success;

class create_response{
public:
    // std::vector<std::variant<account_error, account_success, symbol_error, symbol_success> > responses;
    tinyxml2::XMLDocument doc;
    create_response(){
        const char* declaration ="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        doc.Parse(declaration);
        tinyxml2::XMLElement *root = doc.NewElement("results");
        doc.InsertEndChild(root);
    };
    void add_response(account_error& e){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *error = doc.NewElement("error");
        error->SetAttribute("id", (uint64_t) e.account_id);
        tinyxml2::XMLText *errorText = doc.NewText(e.error_message.c_str());
        error->InsertEndChild(errorText);
        root->InsertEndChild(error);
    }
    void add_response(account_success& s){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *success = doc.NewElement("created");
        success->SetAttribute("id", (uint64_t) s.account_id);
        root->InsertEndChild(success);
    }
    void add_response(symbol_error& e){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *error = doc.NewElement("error");
        error->SetAttribute("sym", e.symbol.c_str());
        error->SetAttribute("id", (uint64_t) e.account_id);
        tinyxml2::XMLText *errorText = doc.NewText(e.error_message.c_str());
        error->InsertEndChild(errorText);
        root->InsertEndChild(error);
    }
    void add_response(symbol_success& s){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *success = doc.NewElement("created");
        success->SetAttribute("sym", s.symbol.c_str());
        success->SetAttribute("id", (uint64_t) s.account_id);
        root->InsertEndChild(success);
    }
    std::string to_string(){
        tinyxml2::XMLPrinter printer;
        doc.Print(&printer);
        return printer.CStr();
    }
};
//--------------------Transcations--------------------

typedef struct transaction_opened{
    std::string symbol;
    long long transaction_id;
    double amount;
    double limit;
}transaction_opened;

typedef struct transaction_error{
    std::string symbol;
    double amount;
    double limit;
    std::string error_message;
}transaction_error;

typedef struct query_cancel_error{
    long long transaction_id;
    std::string error_message;
}query_cancel_error;

typedef struct query_response{
    long long transaction_id;
    std::string status;
    double amount;
    time_t  canceltime;
    std::vector<double> executed_share; 
    std::vector<double> executed_price; 
    std::vector<time_t> executed_time;
}query_response;

typedef struct cancel_response{
    long long transaction_id;
    double canceled_share;
    time_t canceldtime;
    std::vector<double> executed_share; 
    std::vector<double> executed_price; 
    std::vector<time_t> executed_time;
}cancel_response;

class Transactions_response{
public:
    // std::vector<std::variant<transaction_opened, transaction_error, query_cancel_error, query_response, cancel_response>> responses;
    tinyxml2::XMLDocument doc;
    Transactions_response(){
        const char* declaration ="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        doc.Parse(declaration);
        tinyxml2::XMLElement *root = doc.NewElement("results");
        doc.InsertEndChild(root);
    };
    void add_response(transaction_opened& o){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *success = doc.NewElement("opened");
        success->SetAttribute("sym", o.symbol.c_str());
        success->SetAttribute("amount", o.amount);
        success->SetAttribute("limit", o.limit);
        success->SetAttribute("id", (uint64_t) o.transaction_id);
        root->InsertEndChild(success);
    }
    void add_response(transaction_error& e){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *error = doc.NewElement("error");
        error->SetAttribute("sym", e.symbol.c_str());
        error->SetAttribute("amount", e.amount);
        error->SetAttribute("limit", e.limit);
        tinyxml2::XMLText *errorText = doc.NewText(e.error_message.c_str());
        error->InsertEndChild(errorText);
        root->InsertEndChild(error);
    }
    void add_response(query_cancel_error& e){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *error = doc.NewElement("error");
        error->SetAttribute("id", (uint64_t) e.transaction_id);
        tinyxml2::XMLText *errorText = doc.NewText(e.error_message.c_str());
        error->InsertEndChild(errorText);
        root->InsertEndChild(error);
    }
    void add_response(query_response& q){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *query = doc.NewElement("status");
        query->SetAttribute("id", (uint64_t) q.transaction_id);
        // if status is "open"
        if(q.status == "OPEN"){
            tinyxml2::XMLElement *open = doc.NewElement("open");
            open->SetAttribute("shares", q.amount);
            query->InsertEndChild(open);

        }else if (q.status == "CANCELED"){
            tinyxml2::XMLElement *canceled = doc.NewElement("canceled");
            canceled->SetAttribute("shares", q.amount);
            canceled->SetAttribute("time", q.canceltime);
            query->InsertEndChild(canceled);
        }

        for(int i = 0; i < q.executed_share.size(); i++){
            tinyxml2::XMLElement *executed = doc.NewElement("executed");
            executed->SetAttribute("shares", q.executed_share[i]);
            executed->SetAttribute("price", q.executed_price[i]);
            executed->SetAttribute("time", q.executed_time[i]);
            query->InsertEndChild(executed);
        }
        root->InsertEndChild(query);
    }
    void add_response(cancel_response& c){
        tinyxml2::XMLElement *root = doc.RootElement();
        tinyxml2::XMLElement *cancel = doc.NewElement("canceled");
        cancel->SetAttribute("id", (uint64_t) c.transaction_id);

        tinyxml2::XMLElement *canceled_sub = doc.NewElement("canceled");
        canceled_sub->SetAttribute("shares", c.canceled_share);
        canceled_sub->SetAttribute("time", c.canceldtime);
        cancel->InsertEndChild(canceled_sub);

        for(int i = 0; i < c.executed_share.size(); i++){
            tinyxml2::XMLElement *executed = doc.NewElement("executed");
            executed->SetAttribute("shares", c.executed_share[i]);
            executed->SetAttribute("price", c.executed_price[i]);
            executed->SetAttribute("time", c.executed_time[i]);
            cancel->InsertEndChild(executed);
        }
        root->InsertEndChild(cancel);
    }
    std::string to_string(){
        tinyxml2::XMLPrinter printer;
        doc.Print(&printer);
        return printer.CStr();
    }
    //response.add_response(transaction_opened{"AAPL", 100, 100, 123456789});
};

// //--------------------MuTian--------------------

// //Account exist error create twice
// std::string AccountExist(long id);
// //Account created response
// std::string AccountCreated(long id);

// //Position created response
// std::string PositionCreated(std::string &symbol_name,long id,double amount);

// //ACCOUNT not exists error when creating a position
// std::string AccountNotExistError(std::string &symbol_name,long id);

// //Balance insufficient error response
// std::string InsufficientBalance(std::string &symbol_name, double amount, double price);
// //Symbol share insufficient error resposne
// std::string InsufficientShare(std::string &symbol_name, double amount, double price);
// //create a transaction response
// std::string TransactionSetResponse(std::string &symbol_name,
//                                     double amount,
//                                     double price,
//                                     long transaction_id);
// //cancel response
// std::string CancelResponse(long transaction_id, 
//                             double canceled_share, 
//                             long canceldtime,
//                             std::vector<double>& executed_share, 
//                             std::vector<double>& executed_price, 
//                             std::vector<long>& executed_time);
// //error response for cancel transaction not exist
// std::string TransactionNotExist(long transaction_id);
// //error response for query transaction not exist
// std::string TransactionNotExist(long transaction_id, bool status);

// //query response
// std::string QueryResponse(long transaction_id,
//                             std::string &status,
//                             double amount,
//                             long canceltime,
//                             std::vector<double>& executed_share, 
//                             std::vector<double>& executed_price, 
//                             std::vector<long>& executed_time);




#endif
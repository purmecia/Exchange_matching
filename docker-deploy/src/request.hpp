# ifndef __REQUEST_H__
# define __REQUEST_H__

# include <iostream>
# include <string>
# include "tinyxml2.h"
# include "db_func.hpp"
# include "response.hpp"
typedef struct request_account{
    long long account_id;
    double balance;
}request_account;

typedef struct request_symbol{
    std::string symbol;
    struct share{
        long long account_id;
        double amount;
    };
    std::vector<share> shares;
}request_symbol;
// class create_request {
// private:
//     //possibly be creating account or symbol
//     std::vector<std::variant<request_account, request_symbol>>> requests;

// public:
//     create_request(std::vector<std::variant<request_account, request_symbol>>>){};

// };

//constructor
std::string Parse_Create_Request(pqxx::connection *C,tinyxml2::XMLDocument& doc){

    //create response xml
    create_response response;

    tinyxml2::XMLElement *child = doc.RootElement()->FirstChildElement();
    while(child!=NULL){
        //if child is account
        // std::cout<<"child value: "<<child->Value()<<std::endl;
        std::string childvalue=child->Value();
        if(childvalue=="account"){
            //get account id

            long long account_id = std::stoll(child->Attribute("id"));
            std::cout<<"recevied create account:"<<account_id<<std::endl;
            //get balance
            double balance = std::stod(child->Attribute("balance"));
            //create a request_account
            // request_account account = {account_id, balance};
            //push into requests
            // requests.push_back(account);
            // std::cout<<"account id: "<<account_id<<std::endl;
            // std::cout<<"balance: "<<balance<<std::endl;
            // TODO: handle account create request
            AddAccount(C,account_id, balance,response);
        }
        //if child is symbol
        else if(childvalue=="symbol"){
            //get symbol name
            std::string symbol_name = child->Attribute("sym");
            //get all shares
            std::cout<<"recevied create symbol: "<<symbol_name<<std::endl;
            tinyxml2::XMLElement *share = child->FirstChildElement();
            std::vector<request_symbol::share> shares;
            while(share!=NULL){
                //get account id
                long long account_id = std::stoll(share->Attribute("id"));
                //get amount
                double amount = std::stod(share->GetText());
                //create a share
                request_symbol::share share_pair = {account_id, amount};
                //push into shares
                shares.push_back(share_pair);
                //next share
                // std::cout<<"symbol name: "<<symbol_name<<std::endl;
                // std::cout<<"account id: "<<account_id<<std::endl;
                // std::cout<<"amount: "<<amount<<std::endl;

                share = share->NextSiblingElement();
            }
            //create a request_symbol
            request_symbol symbol = {symbol_name, shares};
            //push into requests
            // requests.push_back(symbol);
            //TODO: handle symbol create request
            AddSymbol(C, symbol_name);
            for(int i=0; i<shares.size(); i++){
                AddPostion(C, symbol_name, shares[i].account_id, shares[i].amount, response);
            }

        }
        //next child
        child = child->NextSiblingElement();
    }

    //return response xml
    return response.to_string();

}


typedef struct order{
    std::string symbol;
    double amount;
    double limit;
}order;
typedef struct query{
    long long transaction_id;
}query;
typedef struct cancel{
    long long transaction_id;
}cancel;
// class transactions_request {
// private:

//     long long account_id;
//     std::vector< std::variant<order, query, cancel>> requests;

// public:
//     transactions_request(std::vector< std::variant<order, query, cancel>>);


// };


std::string  Parse_Transactions_Request(pqxx::connection *C,tinyxml2::XMLDocument& doc){
    std::cout<<"recevied transactions request"<<std::endl;
    Transactions_response response;
    
    //get account id
    long long account_id = std::stoll(doc.RootElement()->Attribute("id"));
    //get all orders
    tinyxml2::XMLElement *child = doc.RootElement()->FirstChildElement();
    while(child!=NULL){
        //if child is order
        std::string childvalue=child->Value();
        if(childvalue=="order"){
            //get symbol
            std::string symbol = child->Attribute("sym");
            //get amount
            double amount = std::stod(child->Attribute("amount"));
            //get limit
            double limit = std::stod(child->Attribute("limit"));
            //create an order
            // order order = {symbol, amount, limit};
            //push into requests
            // requests.push_back(order);
            // std::cout<<"recevied order: "<<std::endl;
            // std::cout<<"account id: "<<account_id<<std::endl;
            // std::cout<<"symbol: "<<symbol<<std::endl;
            // std::cout<<"amount: "<<amount<<std::endl;
            // std::cout<<"limit: "<<limit<<std::endl;
            // std::cout<<"-----Do transaction"<<std::endl;

            DoTransaction(C, account_id, symbol, amount, limit,response);
        }
        //if child is query
        else if(childvalue=="query"){
            //get transaction id
            long long transaction_id = std::stoll(child->Attribute("id"));
            //create a query
            // query query = {transaction_id};
            //push into requests
            // requests.push_back(query);
            std::cout<<"recevied query: "<<std::endl;
            std::cout<<"account id: "<<account_id<<std::endl;
            std::cout<<"transaction id: "<<transaction_id<<std::endl;

            QueryTransaction(C, account_id,transaction_id,response);
        }
        //if child is cancel
        else if(childvalue=="cancel"){
            //get transaction id
            long long transaction_id = std::stoll(child->Attribute("id"));
            //create a cancel
            // cancel cancel = {transaction_id};
            //push into requests
            // requests.push_back(cancel);
            std::cout<<"recevied cancel: "<<std::endl;
            std::cout<<"account id: "<<account_id<<std::endl;
            std::cout<<"transaction id: "<<transaction_id<<std::endl;

            CancelTransaction(C, account_id,transaction_id,response);
        }
        //next child
        child = child->NextSiblingElement();
    }

    return response.to_string();
}

# endif
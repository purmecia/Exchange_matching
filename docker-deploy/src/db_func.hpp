#ifndef __DB_FUNC_H__
#define __DB_FUNC_H__

#include<iostream>
#include<string>
#include<pqxx/pqxx>
#include<fstream>
#include<chrono>
#include<ctime>
#include<assert.h>
#include"response.hpp"
#include<mutex>

typedef long long ll;
long curtime();

void ExecuteCommand(std::string query, pqxx::connection *C);

void DropTable(pqxx::connection *C);
void CreateTable(std::string filename,pqxx::connection *C);

void AddAccount(pqxx::connection *C,ll id, double balance,create_response &response);
bool AccountExist(pqxx::connection *C, ll id);

bool SymbolExist(pqxx::connection *C, std::string &symbol_name);
void AddSymbol(pqxx::connection *C,std::string &symbol_name);

void AddPostion(pqxx::connection *C, 
                std::string &symbol_name, 
                ll account_id, 
                double amount,
                create_response &response);
//check buyer's balance before buying symbol
double GetBalance(pqxx::work &W, ll account_id);
//check seller's symbol amount befor selling
double GetSymbolAmount(pqxx::work &W,
                        ll account_id,
                        std::string &symbol_name);
//update user's balance (after an order)
void UpdateBalance(pqxx::work &W, ll account_id, double money);
//update user's symbol amount (after an order)
void UpdatePositionSymbol(pqxx::work &W,
                    ll account_id,
                    std::string &symbol_name,
                    double amount);
//Set transaction status
void SetTransaction(pqxx::work &W,
                        ll transaction_id,
                        std::string status,
                        double amount);
//insert a transaction
ll InsertTransaction(pqxx::work &W,
                        ll account_id,
                        std::string &symbol_name,
                        double amount,
                        double price);

//create a execution after the deal matched
void AddExecution(pqxx::work &W, 
                ll trans_buy_id, 
                ll trans_sell_id,
                double price,
                double amount,
                long time);

// match the transaction for the buyer
void MatchTransactionBuyer(pqxx::work &W,ll transaction_id);
// match the transaction for the seller
void MatchTransactionSeller(pqxx::work &W,ll transaction_id);
//the whole process when creating a transaction
void DoTransaction(pqxx::connection *C, 
            ll account_id,
            std::string &symbol_name,
            double amount,
            double price,
            Transactions_response &response);
//check if the transaction id is valid
bool TransactionExist(pqxx::connection *C, ll transaction_id);
//Add cancelation
void AddCancelation(pqxx::work &W,ll transaction_id,long time);
//cancel a transaction
void CancelTransaction(pqxx::connection *C,ll account_id, ll transaction_id,Transactions_response &response);

//query a transaction
void QueryTransaction(pqxx::connection *C,ll account_id,ll transaction_id,Transactions_response &response);


#endif
#include"db_func.hpp"
std::mutex mtx;

long curtime(){
    time_t now = time(0);
    long curtime = static_cast<long>(now);
    return curtime;
}

void ExecuteCommand(std::string command, pqxx::connection *C){
    pqxx::work W(*C);
    W.exec(command);
    W.commit();
}

void DropTable(pqxx::connection *C){
    std::vector<std::string> tablename{"SYMBOL","ACCOUNT","POSITION","TRANSACTION","EXECUTION","CANCELATION"};
    for(auto name:tablename){
        std::string query = "DROP TABLE IF EXISTS "+name+" CASCADE;";
        ExecuteCommand(query,C);
    }
}
void CreateTable(std::string filename, pqxx::connection *C){
    std::ifstream f(filename);
    std::string command,line;
    if(f.is_open()){
        while(std::getline(f,line)){
            command+=line;
        }
        //std::cout<<command<<std::endl;
        ExecuteCommand(command,C);
        std::cout<<"Successfully Created tables"<<std::endl;
        f.close();
    }else{
        std::cerr<<"Cannot open file for creating tables"<<std::endl;
        exit(EXIT_FAILURE);
    }
}
// void AddAccount(pqxx::connection *C,ll id, double balance, create_response &response){
//     std::cout<<"Enter AddAccount"<<std::endl;
//     pqxx::work W(*C);
//     std::stringstream command;
//     std::cout<<"account id:"<<id<<std::endl;
//     if(AccountExist(W,id)){
//         account_error e = {id,"Account already exists"};
//         response.add_response(e);
//         return;
//     }
//     if(balance <0){
//         account_error e = {id,"Account balance less than 0"};
//         response.add_response(e);
//         return;
//     }
//     command<<"INSERT INTO ACCOUNT (ACCOUNT_ID, BALANCE) "<<"VALUES ("<<W.quote(id)
//     <<", "<<W.quote(balance)<<");";
//     try{
//         W.exec(command.str());
//         W.commit();
//         account_success s={id};
//         response.add_response(s);
//     }catch(const std::exception &e){
//         if(AccountExist(W,id)){
//             account_error e = {id,"Account already exists"};
//             response.add_response(e);
//             return;
//         }
//         std::cerr<<e.what()<<std::endl;
//         W.abort();
//         return;
//     }
// }
// bool AccountExist(pqxx::work &W,ll id){
    
//     std::stringstream query;
//     query<<"SELECT * FROM ACCOUNT WHERE ACCOUNT_ID = "<<W.quote(id)<<";";
//     pqxx::result r(W.exec(query.str()));
//     return r.size()!=0;
// }

bool AccountExist(pqxx::connection *C, ll id) {
    pqxx::nontransaction N(*C);
    std::stringstream query;
    query << "SELECT * FROM ACCOUNT WHERE ACCOUNT_ID = " << N.quote(id) << ";";
    pqxx::result r(N.exec(query.str()));
    return r.size() != 0;
}
void AddAccount(pqxx::connection *C, ll id, double balance, create_response &response) {
    // std::cout << "Enter AddAccount" << std::endl;
    std::stringstream command;
    // std::cout << "account id:" << id << std::endl;

    // Check if the account exists before creating a new transaction
    {
        //pqxx::nontransaction N(*C);
        if (AccountExist(C, id)) {
            account_error e = {id, "Account already exists"};
            response.add_response(e);
            return;
        }
    }

    if (balance < 0) {
        account_error e = {id, "Account balance less than 0"};
        response.add_response(e);
        return;
    }

    pqxx::work W(*C);
    command << "INSERT INTO ACCOUNT (ACCOUNT_ID, BALANCE) "
            << "VALUES (" << W.quote(id)
            << ", " << W.quote(balance) << ");";
    try {
        W.exec(command.str());
        W.commit();
        account_success s = {id};
        response.add_response(s);
    } catch (const std::exception &e) {
        // Check again if the account exists in case it was created after the nontransaction check
        {
            //pqxx::nontransaction N(*C);
            if (AccountExist(C, id)) {
                account_error e = {id, "Account already exists"};
                response.add_response(e);
                return;
            }
        }
        std::cerr << e.what() << std::endl;
        W.abort();
        return;
    }
}

// bool SymbolExist(pqxx::nontransaction &N, std::string &symbol_name){
//     std::stringstream query;
//     query<<"SELECT * FROM SYMBOL WHERE SYMBOL_NAME = "<<N.quote(symbol_name)<<";";
//     pqxx::result r(N.exec(query.str()));
//     return r.size()!=0;
// }
// void AddSymbol(pqxx::connection *C,std::string &symbol_name){
  
//     std::stringstream command;
//     pqxx::nontransaction N(*C);
//     if(SymbolExist(N,symbol_name)){
//         return;
//     }
    
//     pqxx::work W(*C);
//     command<<"INSERT INTO SYMBOL VALUES ("<<W.quote(symbol_name)<<");";
//     try{
//         W.exec(command.str());
//         W.commit();
//     }catch(const std::exception &e){
//         std::cerr<<e.what()<<std::endl;
//         W.abort();
//         return;
//     }
    
// }
bool SymbolExist(pqxx::connection *C, std::string &symbol_name) {
    pqxx::nontransaction N(*C);
    std::stringstream query;
    query << "SELECT * FROM SYMBOL WHERE SYMBOL_NAME = " << N.quote(symbol_name) << ";";
    pqxx::result r(N.exec(query.str()));
    return r.size() != 0;
}
void AddSymbol(pqxx::connection *C, std::string &symbol_name) {
    
    std::cout<<"Enter AddSymbol"<<std::endl;
    // Check if the symbol exists
    if (SymbolExist(C, symbol_name)) {
        return;
    }
    pqxx::work W(*C);   
    // Insert the symbol
    std::stringstream command;
    command << "INSERT INTO SYMBOL VALUES (" << W.quote(symbol_name) << ");";
    try {
        W.exec(command.str());
        W.commit();
    } catch (const std::exception &e) {
        if(SymbolExist(C, symbol_name)){
            return;
        }
        std::cerr << e.what() << std::endl;
        W.abort();
        return;
    }
}

void AddPostion(pqxx::connection *C,
                std::string &symbol_name,
                ll account_id,
                double amount,
                create_response &response){
    std::cout<<"Enter AddPosition"<<std::endl;
 
     if(!AccountExist(C,account_id)){
        symbol_error e = {symbol_name,account_id,"Account not exists"};
        response.add_response(e);
        return;
    }
    pqxx::work W(*C);
    std::stringstream command;
    command<<"INSERT INTO POSITION (ACCOUNT_ID, SYMBOL_NAME, AMOUNT) VALUES ("
    <<W.quote(account_id)<<", "<<W.quote(symbol_name)<<", "<<W.quote(amount)<<")"
    <<"ON CONFLICT (ACCOUNT_ID, SYMBOL_NAME) DO UPDATE SET AMOUNT = POSITION.AMOUNT + "
    << W.quote(amount)<<";";
    for(auto i:symbol_name){
        if(!isalnum(i)){
        symbol_error e = {symbol_name,account_id,"Symbol name invalid"};
        response.add_response(e);
        return;
        }
    }
    if(amount<0){
        symbol_error e = {symbol_name,account_id,"Symbol amount less than 0 "};
        response.add_response(e);
        return;
    }
    try{
        W.exec(command.str());
        W.commit();
        symbol_success s ={symbol_name,account_id};
        response.add_response(s);
    }catch(const std::exception &e){
        std::cerr<<e.what()<<std::endl;
        W.abort();
        return;
    }
}
//check buyer's balance before buying symbol
double GetBalance(pqxx::work &W, ll account_id){
    std::cout<<"get balance"<<std::endl;
    std::stringstream query;
    query<<"SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID = "<<W.quote(account_id)<<";";
    pqxx::result R(W.exec(query.str()));
    return R.begin()[0].as<double>();
}
//check seller's symbol amount befor selling
double GetSymbolAmount(pqxx::work &W,
                        ll account_id,
                        std::string &symbol_name){
    std::cout<<"get symbol amount"<<std::endl;
    std::stringstream query;
    query<<"SELECT AMOUNT FROM POSITION WHERE ACCOUNT_ID = "<<W.quote(account_id)
    <<" AND SYMBOL_NAME = "<<W.quote(symbol_name)<<";";
    pqxx::result r(W.exec(query.str()));
    if(r.size()==0){
        return 0;
    }else{
        return r.begin()[0].as<double>();
    }
}
//update user's balance (after an order)
void UpdateBalance(pqxx::work &W, ll account_id, double money){
    std::cout<<"update balance"<<std::endl;
    std::stringstream command;
    command<<"UPDATE ACCOUNT SET BALANCE = ACCOUNT.BALANCE + "<< W.quote(money)
    <<" WHERE ACCOUNT_ID = "<<W.quote(account_id)<<";";
    W.exec(command.str());
}
//update user's symbol amount (after an order)
void UpdatePositionSymbol(pqxx::work &W,
                    ll account_id,
                    std::string &symbol_name,
                    double amount){
    
    std::cout<<"update position"<<std::endl;                    
    std::stringstream command;
    command<<"UPDATE POSITION SET AMOUNT = POSITION.AMOUNT + "<< W.quote(amount)
    <<"WHERE ACCOUNT_ID = "<<W.quote(account_id)<<" AND SYMBOL_NAME = "
    <<W.quote(symbol_name)<<";";
    W.exec(command.str());

}
//set transaction status
void SetTransaction(pqxx::work &W,
                        ll transaction_id,
                        std::string status,
                        double amount){

    std::stringstream command;
    command<<"UPDATE TRANSACTION "<<"SET STATUS = "
    <<W.quote(status)<<", AMOUNT = "<<amount
    <<" Where TRANSACTION.TRANSACTION_ID = "<<W.quote(transaction_id)<<";";
    W.exec(command.str());

}
//insert a transaction -> return transaction id if successful
ll InsertTransaction(pqxx::work &W,
                        ll account_id,
                        std::string &symbol_name,
                        double amount,
                        double price){
    std::cout<<"insert transaction"<<std::endl;
    
    std::stringstream command;
    command<<"INSERT INTO TRANSACTION ("<<"ACCOUNT_ID, SYMBOL_NAME, PRICE, AMOUNT, STATUS, TIME) "
    <<" VALUES ("<<W.quote(account_id)<<", "<<W.quote(symbol_name)<<", "<<W.quote(price)
    <<", "<<W.quote(amount)<<", "<<W.quote("OPEN")<<", "<<W.quote(curtime())<<") RETURNING TRANSACTION_ID;";
    pqxx::result r(W.exec(command.str()));
    return r.begin()[0].as<ll>();
}
//create a execution after the deal matched
void AddExecution(pqxx::work &W,
                ll trans_buy_id, 
                ll trans_sell_id,
                double price,
                double amount,
                long time){
    std::cout<<"add execution"<<std::endl;
    std::stringstream command;
    command << "INSERT INTO EXECUTION "
    <<"(TRANSACTION_BUY_ID, TRANSACTION_SELL_ID, TIME, AMOUNT, PRICE)"
    <<" VALUES ("<<W.quote(trans_buy_id)<<", "<<W.quote(trans_sell_id)<<", "
    <<W.quote(time)<<", "<<W.quote(amount)<<", "<<W.quote(price)<<");";
    W.exec(command.str());

}

// match the transaction for the buyer
void MatchTransactionBuyer(pqxx::work &W,ll transaction_id){
  
    std::cout<<"match transaction buyer"<<std::endl;
    std::stringstream command;
    command<<"SELECT TRANSACTION_ID, ACCOUNT_ID, SYMBOL_NAME, PRICE, AMOUNT FROM TRANSACTION"
    <<" WHERE STATUS = "<< W.quote("OPEN") << " AND TRANSACTION_ID = "<<W.quote(transaction_id)<<";";
    pqxx::result r(W.exec(command.str()));
    //got the buying info
    auto buyer_id = r.begin()[1].as<ll>();
    auto symbol_name = r.begin()[2].as<std::string>();
    auto price = r.begin()[3].as<double>();
    auto amount = r.begin()[4].as<double>(); 
    command.clear();
    command<<"SELECT TRANSACTION_ID, ACCOUNT_ID, PRICE, AMOUNT, TIME FROM TRANSACTION"
    <<" WHERE STATUS = "<< W.quote("OPEN") << "AND SYMBOL_NAME = "<<W.quote(symbol_name)
    <<" AND PRICE <= "<<W.quote(price)<<" AND AMOUNT < 0"<< " ORDER BY PRICE ASC, TIME ASC;";
    //we got all the selling records approriate for the buyer
    pqxx::result records(W.exec(command.str()));
    if(records.size()==0){
        //No match found
        return;
    }
    ll sell_trans_id;
    ll seller_id;
    double sell_price;
    double sell_amount;
    
    for(auto r = records.begin(); r!=records.end();++r){
        sell_trans_id = r[0].as<ll>();
        seller_id = r[1].as<ll>();
        sell_price = r[2].as<double>();
        sell_amount = r[3].as<double>();
        //sell_amount is negative
        //enough to sell to the buyer
        std::cout<<"buying amount: "<<amount<< "sell id:"<<sell_trans_id<<" sell price:"<<price<<" sell amount: "<<sell_amount<<std::endl;
        if(-sell_amount>=amount){
            //then deal on amount sell_price -> create execution
            std::cout<<"buying from "<<sell_trans_id<<" with price "<< sell_price<<" with amount "<<amount<<std::endl;
            AddExecution(W,transaction_id,sell_trans_id,sell_price,amount,curtime());
            //update seller's balance and buyer's position
            UpdateBalance(W, seller_id , amount*sell_price);
            UpdatePositionSymbol(W,buyer_id,symbol_name,amount);
            // update the sell symbol (sell_amount left)
            if(-sell_amount==amount){
                SetTransaction(W, sell_trans_id,"EXECUTED",0);
                SetTransaction(W, transaction_id,"EXECUTED",0);
            }
            else {
                SetTransaction(W, sell_trans_id,"OPEN",sell_amount+amount);
                SetTransaction(W, transaction_id,"EXECUTED",0);
                }
            return;
        }else{
            //then deal on sell_amount sell_price
            std::cout<<"buying from "<<sell_trans_id<<" with price "<< sell_price<<" with amount "<<-sell_amount<<std::endl;
            AddExecution(W,transaction_id,sell_trans_id,sell_price,-sell_amount,curtime());
            //update seller's balance and buyer's position
            UpdateBalance(W, seller_id , -sell_amount*sell_price);
            UpdatePositionSymbol(W,buyer_id,symbol_name,-sell_amount);
            //this sell_trans_id done -> close it
            SetTransaction(W, sell_trans_id,"EXECUTED",0);
            SetTransaction(W, transaction_id,"OPEN",sell_amount+amount);
            amount+=sell_amount;
        }
    }
}

//match the transaction for the seller
void MatchTransactionSeller(pqxx::work &W,ll transaction_id){
   
    std::cout<<"match transaction seller"<<std::endl;
    std::stringstream command;
    command<<"SELECT TRANSACTION_ID, ACCOUNT_ID, SYMBOL_NAME, PRICE, AMOUNT FROM TRANSACTION"
    <<" WHERE STATUS = "<< W.quote("OPEN") << " AND TRANSACTION_ID = "<<W.quote(transaction_id)<<";";
    pqxx::result r(W.exec(command.str()));
    //got the selling info
    auto seller_id = r.begin()[1].as<ll>();
    auto symbol_name = r.begin()[2].as<std::string>();
    auto price = r.begin()[3].as<double>();
    auto amount = r.begin()[4].as<double>(); 
    command.clear();
    command<<"SELECT TRANSACTION_ID, ACCOUNT_ID, PRICE, AMOUNT, TIME FROM TRANSACTION"
    <<" WHERE STATUS = "<< W.quote("OPEN") << " AND SYMBOL_NAME = "<<W.quote(symbol_name)
    <<" AND PRICE >= "<<W.quote(price)<<" AND AMOUNT > 0"<< " ORDER BY PRICE DESC, TIME ASC;";
    //we got all the selling records approriate for the buyer
    pqxx::result records(W.exec(command.str()));
    if(records.size()==0){
        //No match found
        return;
    }
    ll buy_trans_id;
    ll buyer_id;
    double buy_price;
    double buy_amount;
    for(auto r = records.begin(); r!=records.end();++r){
        buy_trans_id = r[0].as<ll>();
        buyer_id = r[1].as<ll>();
        buy_price = r[2].as<double>();
        buy_amount = r[3].as<double>();
        //amount is negative
        //buy amount cover all sell amount
        if(buy_amount>=-amount){
            //then deal on amount buy_price -> create execution
            AddExecution(W,buy_trans_id,transaction_id,buy_price,-amount,curtime());
            //update seller's balance and buyer's position
            UpdateBalance(W, seller_id , -amount*buy_price);
            UpdatePositionSymbol(W,buyer_id,symbol_name,-amount);
            // update the buy symbol (sell_amount left)
            if(buy_amount==-amount) {
                SetTransaction(W, buy_trans_id,"EXECUTED",0);
                SetTransaction(W, transaction_id,"EXECUTED",0);
            }
            else {
                SetTransaction(W, buy_trans_id,"OPEN",buy_amount+amount);
                SetTransaction(W, transaction_id,"EXECUTED",0);
            }
            return;
        }else{
            //then deal on buy_amount buy_price
            AddExecution(W,transaction_id,buy_trans_id, buy_price,buy_amount,curtime());
            //update seller's balance and buyer's position
            UpdateBalance(W, seller_id , buy_amount*buy_price);
            UpdatePositionSymbol(W,buyer_id,symbol_name,buy_amount);
            //this sell_trans_id done -> close it
            SetTransaction(W, transaction_id,"OPEN",amount+buy_amount);
            SetTransaction(W, buy_trans_id,"EXECUTED",0);
            amount+=buy_amount;
        }
    }
}
//the whole process when creating a transaction
void DoTransaction(pqxx::connection *C, 
            ll account_id,
            std::string &symbol_name,
            double amount,
            double price,
            Transactions_response &response){
    assert(amount != 0);
    std::lock_guard<std::mutex> guard(mtx);
    std::cout<<"do transaction"<<std::endl;
    //0. CHECK symbol exist? and account id
    //pqxx::nontransaction N(*C);
    if(!AccountExist(C,account_id)){
        transaction_error e ={symbol_name,amount,price,"Account not exists"};
        response.add_response(e);
        return;
    }
    if(!SymbolExist(C,symbol_name)){
        transaction_error e ={symbol_name,amount,price,"Symbol not exists"};
        response.add_response(e);
        return;
    }
    pqxx::work W(*C);
    if(price<0){
        transaction_error e ={symbol_name,amount,price,"price less than 0 invalid"};
        response.add_response(e);
        return;
    }
    //1. check account balance if amount > 0 (buy)
    //2. check position share if amount < 0 (sell)
    if(amount>0){
        double balance = GetBalance(W,account_id);
        if(balance < amount * price){
            
            transaction_error e ={symbol_name,amount,price,"Insufficient Balance for the order"};
            response.add_response(e);
            return;
        }else{
            // update balance
       
            UpdateBalance(W,account_id,-amount*price);
        }
    }else{
        double sym_amount = GetSymbolAmount(W,account_id,symbol_name);
        if(sym_amount<-amount){
            transaction_error e ={symbol_name,amount,price,"Insufficient share for the order"};
            response.add_response(e);
            return;
        }else{
            //update postion

            UpdatePositionSymbol(W,account_id,symbol_name,amount);
        }
    }

    //3. add order to transaction
    
    ll transaction_id = InsertTransaction(W,account_id,symbol_name,amount,price);
    transaction_opened s = {symbol_name,transaction_id,amount,price};
    response.add_response(s);
    //std::string success_resp = TransactionSetResponse(symbol_name,amount,price,transaction_id);
    //4. match order 
    
    if(amount>0) MatchTransactionBuyer(W,transaction_id);
    else MatchTransactionSeller(W,transaction_id);
    W.commit();
    return;
}
//check if the transaction id is valid
bool TransactionExist(pqxx::connection *C, ll transaction_id){
    std::cout<<"check transaction exist"<<std::endl;
    pqxx::nontransaction N(*C);
    std::stringstream query;
    query<<"SELECT 1 FROM TRANSACTION WHERE TRANSACTION_ID = "
    <<N.quote(transaction_id)<<";";
    pqxx::result r(N.exec(query.str()));
    return r.size()!=0;
}


//Add cancelation
void AddCancelation(pqxx::work &W,ll transaction_id,long time){
    std::cout<<"add cancelation"<<std::endl;
    std::stringstream command;
    command << "INSERT INTO CANCELATION "
    <<" VALUES ("<<W.quote(transaction_id)<<", "<<W.quote(time)<<");";
    W.exec(command.str());
    W.commit();
}

void CancelTransaction(pqxx::connection *C,ll account_id,ll transaction_id,Transactions_response&response){
    //transaction id invalid
    
    std::cout<<"cancel transaction"<<std::endl;
    if(!TransactionExist(C,transaction_id)) {
        query_cancel_error e = {transaction_id,"Transaction id not exists"};
        response.add_response(e);
        return;
    }

    //status = OPEN / EXECUTED / CANCELED
    pqxx::work W(*C);
    std::stringstream query;
    query<<"SELECT AMOUNT, STATUS, PRICE, SYMBOL_NAME FROM TRANSACTION WHERE TRANSACTION_ID = "
    <<W.quote(transaction_id)<<";";
    pqxx::result r(W.exec(query.str()));
    double amount = r.begin()[0].as<double>();
    std::string status = r.begin()[1].as<std::string>();
    double price = r.begin()[2].as<double>();
    std::string symbol_name = r.begin()[3].as<std::string>();
    std::vector<double> executed_share,executed_price;
    std::vector<long> executed_time;
    if(status == "CANCELED"){
        query_cancel_error e = {transaction_id,"Transaction already canceled"};
        response.add_response(e);
        return;
    }
    query.clear();

    query<<"SELECT TRANSACTION_ID FROM TRANSACTION WHERE ACCOUNT_ID = "
    <<W.quote(account_id)<<";";
    pqxx::result mytrans(W.exec(query.str()));
    int flag =0;
    for(auto x=mytrans.begin();x!=mytrans.end();++x){
        if(x[0].as<ll>()==transaction_id){
            flag=1;
            break;
        }
    }
    if(!flag){
        query_cancel_error e = {transaction_id,"Cancel a transaction not belonging to this account"};
        response.add_response(e);
        return;
    }

    query.clear();
    query<<"SELECT AMOUNT, PRICE, TIME FROM EXECUTION WHERE TRANSACTION_BUY_ID = "
    <<W.quote(transaction_id)
    <<" OR TRANSACTION_SELL_ID = "<<W.quote(transaction_id)<<";";
    pqxx::result exr(W.exec(query.str()));
    for(auto info=exr.begin();info!=exr.end();++info){
        executed_share.push_back(info[0].as<double>());
        executed_price.push_back(info[1].as<double>());
        executed_time.push_back(info[2].as<long>());
    }
    std::string resp;
    long timenow=curtime();
    if(status == "OPEN"){
        SetTransaction(W,transaction_id,"CANCELD",amount);
        AddCancelation(W,transaction_id,timenow);
        if(amount<0){
           UpdatePositionSymbol(W,account_id,symbol_name,amount);
        }else{
            UpdateBalance(W,account_id,amount*price);
        }
    }
    cancel_response s = {transaction_id,amount,timenow,executed_share,executed_price,executed_time};
    response.add_response(s);
    W.commit();
    return;
}


//query a transaction
void QueryTransaction(pqxx::connection *C,ll account_id,ll transaction_id,Transactions_response&response){
    std::cout<<"query transaction"<<std::endl;
    if(!TransactionExist(C,transaction_id)){
        query_cancel_error e = {transaction_id,"Transaction id not exists"};
        response.add_response(e);
        return;
    }
    pqxx::work W(*C);
    std::stringstream query;
    query<<"SELECT TRANSACTION_ID FROM TRANSACTION WHERE ACCOUNT_ID = "
    <<W.quote(account_id)<<";";
    pqxx::result mytrans(W.exec(query.str()));
    int flag =0;
    for(auto x=mytrans.begin();x!=mytrans.end();++x){
        if(x[0].as<ll>()==transaction_id){
            flag=1;
            break;
        }
    }
    if(!flag){
        query_cancel_error e = {transaction_id,"Query a transaction not belonging to this account"};
        response.add_response(e);
        return;
    }

    //status = OPEN / EXECUTED / CANCELED
    query.clear();

    query<<"SELECT AMOUNT, STATUS FROM TRANSACTION WHERE TRANSACTION_ID = "
    <<W.quote(transaction_id)<<";";
    pqxx::result r(W.exec(query.str()));
    double amount = r.begin()[0].as<double>();
    std::string status = r.begin()[1].as<std::string>();

    //get exectued info
    std::vector<double> executed_share,executed_price;
    std::vector<long> executed_time;
    query.clear();


    query<<"SELECT AMOUNT, PRICE, TIME FROM EXECUTION WHERE TRANSACTION_BUY_ID = "
    <<W.quote(transaction_id)
    <<" OR TRANSACTION_SELL_ID = "<<W.quote(transaction_id)<<";";
    pqxx::result exr(W.exec(query.str()));
    for(auto info=exr.begin();info!=exr.end();++info){
        executed_share.push_back(info[0].as<double>());
        executed_price.push_back(info[1].as<double>());
        executed_time.push_back(info[2].as<long>());
    }

    //get canceled info
    long canceledtime=0;
    if(status=="CANCELED"){
        query.clear();
        query<<"SELECT TIME FROM CANCELATION WHERE TRANSACTION_ID = "
        <<W.quote(transaction_id)<<";";
        pqxx::result celr(W.exec(query.str()));
        canceledtime = celr.begin()[0].as<long>();
    }
    query_response s = {transaction_id,status,amount,canceledtime,executed_share,executed_price,executed_time};
    response.add_response(s);
    W.commit();
  /* std::string resp = QueryResponse(transaction_id,
                                    status,amount,canceledtime,
                                    executed_share,executed_price,executed_time);*/ 
    return;

}
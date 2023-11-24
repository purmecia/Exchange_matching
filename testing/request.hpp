# ifndef __REQUEST_H__
# define __REQUEST_H__

# include <iostream>
# include <string>
# include <vector>
# include "tinyxml2.h"
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

# endif
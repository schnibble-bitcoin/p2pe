#ifndef ORDER_H
#define ORDER_H

#include "common.h"

class Client;

class Order
{
public:
    int64_t price;
    int64_t amount;
    Client* client;

public:
    Order(int64_t _price, int64_t _amount, Client* _client) : price(_price), amount(_amount), client(_client) {}
    bool operator <(const Order& right) const;
};

#endif // ORDER_H

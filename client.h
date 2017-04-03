#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"
#include "order.h"

class BalanceUSD;

extern set<Order> setOrderBuy;
extern set<Order> setOrderSell;
extern set<BalanceUSD> setDebtUSD;
extern set<BalanceUSD> setTPUSD;


struct Balance
{
    int64_t btc;
    int64_t usd;

    Balance() : btc(0), usd(0) {}
    Balance(int64_t _btc, int64_t _usd) : btc(_btc), usd(_usd) {}
//    Balance& operator=(const Balance& _right)
//    {
//        btc = _right.btc;
//        usd = _right.usd;
//        return *this;
//    }
};


class Client : public Balance
{
public:
    string pubKey;

public:
    Client(const string& _pubkey) : pubKey(_pubkey) {}
    bool operator <(const Client& right) const
    {
        return pubKey < right.pubKey;
    }

    void DepositBTC(int64_t _amount)
    {
        btc += _amount;
    }

    void WithdrawBTC(int64_t _amount)
    {
        btc -= _amount;
    }

    void DepositUSD(int64_t _amount)
    {
        usd += _amount;
    }

    void WithdrawUSD(int64_t _amount)
    {
        usd -= _amount;
    }

    void BuyBTCUSD(int64_t _price, int64_t _amount);

    void SellBTCUSD(int64_t _price, int64_t _amount)
    {
        usd += _amount;
        btc -= _price*_amount;
    }

};

struct BalanceUSD : public Balance
{
    Client* client;

    //BalanceUSD(const Balance& _balance) : Balance(_balance), client(nullptr) {}
    BalanceUSD(Client& _client) : Balance(_client), client(&_client) {}
    BalanceUSD(const Balance& _balance, Client* _client) : Balance(_balance), client(_client) {}

    bool operator <(const BalanceUSD& right) const
    {
        if(usd < 0)
            assert(btc > 0);
        if(btc < 0)
            assert(usd > 0);
        if(right.btc < 0)
            assert(right.usd > 0);
        if(right.usd < 0)
            assert(right.btc > 0);

        if (usd < 0 && right.usd < 0)
        {
            __int128_t x = usd*right.btc;
            __int128_t y = right.usd*btc;
            return (x == y)? *client < *right.client : x < y;
        }

        if (usd > 0 && right.usd > 0)
        {
            if (btc < 0 && right.btc < 0)
            {
                __int128_t x = usd*right.btc;
                __int128_t y = right.usd*btc;
                return (x == y)? *client < *right.client : x > y;
            }

            if (btc > 0 && right.btc > 0)
            {
                __int128_t x = usd*right.btc;
                __int128_t y = right.usd*btc;
                return (x == y)? *client < *right.client : x > y;
            }

            if (btc == right.btc)
                return (usd != right.usd)? usd < right.usd : *client < *right.client;

            return btc < right.btc;
        }

        // равно нулю
        if (usd == right.usd)
            return (btc != right.btc)? btc < right.btc : *client < *right.client;

        // разный знак
        return usd < right.usd;


//        // если разный знак usd
//        if ((usd^right.usd)&0x8000000000000000)
//            return usd < right.usd;

//        __int128_t x = usd*right.btc;
//        __int128_t y = right.usd*btc;

//        if (x == y)
//            return *client < *right.client;

//        if (usd < 0)
//        {
//            assert(btc > 0 && right.btc > 0);
//            return x < y;
//        }
//        return false;


// v1

//        if (usd < 0 && right.usd < 0)
//        {
//            assert(btc > 0 && right.btc > 0);
//            __int128_t x = usd*right.btc;
//            __int128_t y = right.usd*btc;
//            return (x == y)? *client < *right.client : x < y;
//        }
        //return (usd == right.usd)? ((btc == right.btc) ? *client < *right.client : btc < right.btc) : usd < right.usd;
    }
};

#endif // CLIENT_H

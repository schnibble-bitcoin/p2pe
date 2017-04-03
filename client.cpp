#include "client.h"

void Client::BuyBTCUSD(int64_t _price, int64_t _amount)
{

    if (usd < 0)
    {
        setDebtUSD.erase(*this);
        //setTPBTC.erase(*this);
    }
    else if (usd > 0)
    {
        setTPUSD.erase(*this);
//            if (btc < 0)
//                setDebtBTC.erase(*this);
    }

    btc += _amount;
    usd -= _price * _amount;
    printf("%s balance: %.4f %.4f\n", pubKey.c_str(), btc/(float)BTC_UNIT, usd/(float)USD_UNIT);



    if (usd < 0)
    {
        setDebtUSD.emplace(*this);
        //setTPBTC.emplace(*this);
    }
    else if (usd > 0)
    {
        setTPUSD.emplace(*this);
//            if (btc < 0)
//                setDebtBTC.emplace(*this);
    }
}


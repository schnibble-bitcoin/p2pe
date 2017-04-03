#include "common.h"
#include "client.h"
#include "order.h"
#include "liquidation.h"


using namespace std;

int64_t global_ask;
int64_t global_bid;





//bool BuyBTCUSD(Client& _user, int64_t _price, int64_t _amount)
//{
//    setOrderBuy.emplace(Order(_price, _amount, &_user));
//    //user.BuyBTCUSD(amount, price);
//}

int main()
{

    vector<Balance> vecBalance;

    vecBalance.emplace_back(-10, 2);
    vecBalance.emplace_back(-9, 2);
    vecBalance.emplace_back(-0, 2);
    vecBalance.emplace_back(-5, 2);
    vecBalance.emplace_back(-11, 2);
    vecBalance.emplace_back(-10, 6);
    vecBalance.emplace_back(-9, 6);
    vecBalance.emplace_back(0, 6);
    vecBalance.emplace_back(5, 6);
    vecBalance.emplace_back(11, 6);

    vecBalance.emplace_back(0, 0);

    vecBalance.emplace_back(-9, 2);
    vecBalance.emplace_back(-5, 2);
    vecBalance.emplace_back(-1, 2);
    vecBalance.emplace_back(-3, 2);
    vecBalance.emplace_back(-11, 6);
    vecBalance.emplace_back(-9, 6);

    vecBalance.emplace_back(1, 6);
    vecBalance.emplace_back(2, 6);
    vecBalance.emplace_back(3, 6);
    vecBalance.emplace_back(4, 6);
    vecBalance.emplace_back(9, 6);
    vecBalance.emplace_back(7, 6);
    vecBalance.emplace_back(11, 6);
    vecBalance.emplace_back(1, 4);
    vecBalance.emplace_back(2, 4);
    vecBalance.emplace_back(3, 4);
    vecBalance.emplace_back(5, 4);
    vecBalance.emplace_back(4, 4);
    vecBalance.emplace_back(6, 4);
    vecBalance.emplace_back(11, 4);
    vecBalance.emplace_back(111, 112);

    vecBalance.emplace_back(2, -10);
    vecBalance.emplace_back(2, -9);
    vecBalance.emplace_back(2, 0);
    vecBalance.emplace_back(2, -3);
    vecBalance.emplace_back(2, -11);
    vecBalance.emplace_back(6, -10);
    vecBalance.emplace_back(6, -9);
    vecBalance.emplace_back(6, 0);
    vecBalance.emplace_back(6, -5);
    vecBalance.emplace_back(6, -11);

    multiset<BalanceUSD> setBalance;
    Client client("schnibble");
    for(auto i: vecBalance)
    {
        setBalance.emplace(i, &client);
    }

    for(auto i: setBalance)
        printf("%s: usd=%lld btc=%lld   (%0.4f)\n", i.client->pubKey.c_str(), i.usd, i.btc, i.usd/(double)i.btc);
    return 0;


    global_ask = 231;
    global_bid = 219;
    Client schnibble("schnibble");
    schnibble.DepositBTC(4.3*BTC_UNIT);
//    BuyBTCUSD(schnibble, 245.33*USD_UNIT/BTC_UNIT, 12.3*BTC_UNIT);
//    BuyBTCUSD(schnibble, 225.33*USD_UNIT/BTC_UNIT, 2*BTC_UNIT);

    for (const auto &v : setOrderBuy)
        printf("%s order: %lld %lld\n", v.client->pubKey.c_str(), v.price, v.amount);

    Liquidation l(200, 195);
    l.Process();

    return 0;
}

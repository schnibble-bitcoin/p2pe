#ifndef LIQUDATION_H
#define LIQUDATION_H

#include "client.h"


class Liquidation
{

private:

    struct LiqudationState
    {
        Client* client;                                     // текущий клиент
        set<Order>::const_reverse_iterator itOrder;         // продавать будем начиная с этой заявки
        int64_t orderUsedAmount;                            // неизрасходованная часть заявки
        int64_t totalBalance;                               // текущий баланс

        LiqudationState(Client* _client, set<Order>::const_reverse_iterator _it_order,
                        int64_t _order_used_amount, int64_t _total_balance) :
            client(_client), itOrder(_it_order), orderUsedAmount(_order_used_amount), totalBalance(_total_balance) {}
    };

    struct LiqudationStep
    {
        BalanceUSD balance;                                 // ликвидируемый клиент и его текущий баланс
        set<Order>::const_reverse_iterator itOrder;         // текущая заявка
        int64_t amount;                                     // проданный объем

        LiqudationStep(BalanceUSD _balance, set<Order>::const_reverse_iterator _it_order, int64_t _amount) :
            balance(_balance), itOrder(_it_order), amount(_amount) {}
    };


    int64_t tp_buy_price;
    int64_t tp_buy_price_delta;
    vector<LiqudationState> vecState;
    vector<LiqudationStep> vecLiquidated;                   // последовательность и баланс пользователей на момент ликвидирования

    void CheckCriticalOrders();
    void ExecuteOrders(set<Order>::reverse_iterator it_partial_order, int64_t partial_amount);

public:

    Liquidation(int64_t _tp_buy_price, int64_t _tp_buy_price_new)
    {
        tp_buy_price = _tp_buy_price;
        tp_buy_price_delta = _tp_buy_price - _tp_buy_price_new;
    }

    void Process();
};


#endif // LIQUDATION_H

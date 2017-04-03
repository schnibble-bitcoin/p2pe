#include "liquidation.h"

set<Order> setOrderBuy;
set<Order> setOrderSell;
set<BalanceUSD> setDebtUSD;
set<BalanceUSD> setTPUSD;


void Liquidation::CheckCriticalOrders()
{
    // проверяем необхдимость ликвидации
    for (auto it_state = vecState.rbegin(); it_state != vecState.rend(); ++it_state)
    {
        // ищем критически важную заявку
        int64_t order_rest;
        int64_t cancelled_amount;

        auto it_exclude_order = it_state->itOrder;
        if (it_exclude_order == setOrderBuy.rend())
            --it_exclude_order;

        while(true)
        {

            if (it_exclude_order == it_state->itOrder)
            {
                cancelled_amount = it_state->orderUsedAmount;
                order_rest = 0;
            }
            else
            {
                cancelled_amount = it_exclude_order->amount;
                order_rest = (it_state->itOrder == setOrderBuy.rend())? cancelled_amount :   // больше не нужно
                              it_state->itOrder->amount - it_state->orderUsedAmount;
            }

            // оставшийся баланс после исключения заявки
            int64_t balance = it_state->totalBalance - cancelled_amount * it_exclude_order->price;
            // оставшиеся средства после исключения заявки
            int64_t assets = cancelled_amount;

            if (balance + assets * tp_buy_price < 0)
            {
                // пытаемся погасить долг за счет других заявок
                auto it_order = it_state->itOrder;
                while (it_order != setOrderBuy.rend())
                {
                    int64_t amount = min(assets, order_rest);
                    balance += amount * it_order->price;
                    assets -= amount;
                    order_rest -= amount;

                    if (balance >= 0 || assets == 0)
                        break;

                    // переходим к следующей заявке
                    if (++it_order != setOrderBuy.rend())
                        order_rest = it_order->amount;
                }

                if (it_order == setOrderBuy.rend())
                    // дальше ликвидируемся по принудительным тейк-профитам держателей usd
                    balance += assets*tp_buy_price;

                if (balance < 0)
                {
                    // нашли критическую заявку

                    // рассчитываем минимальную часть этой заявки, необходимую для ликвидации

                    int64_t order_price;
                    int64_t used_amount;

                    if (it_order == setOrderBuy.rend())
                    {
                        used_amount = assets;
                        order_price = tp_buy_price;
                    }
                    else
                    {
                        assert(assets == 0);
                        used_amount = it_order->amount - order_rest;
                        order_price = it_order->price;
                    }

                    int64_t amount = 0;
                    int64_t partial_amount = 0;

                    while (true)
                    {
                        // отменяем невыгодные заявки и взамен используем часть от критической
                        assert(it_exclude_order->price > order_price);
                        partial_amount = 1 + (-balance - amount*it_exclude_order->price - 1) / (it_exclude_order->price - order_price);

                        if (partial_amount <= used_amount)
                            break;

                        balance -= used_amount * order_price;
                        amount += used_amount;

                        used_amount = (--it_order)->amount;
                        order_price = it_order->price;
                    }

                    return ExecuteOrders(it_exclude_order, partial_amount + amount);
                }

            }

            if (it_exclude_order == setOrderBuy.rbegin())
                break;

            --it_exclude_order;
        }
    }
}

void Liquidation::ExecuteOrders(set<Order>::reverse_iterator it_partial_order, int64_t partial_amount)
{
    set<Order>::iterator it_remove_order;
    int64_t remove_amount;

    int64_t partial_price = tp_buy_price;

    if (!setOrderBuy.empty())
    {
        // корректируем баланс покупателей по заявкам

        auto it_order = it_partial_order;

        int64_t order_amount;
        if (it_order == setOrderBuy.rend())
            order_amount = (--it_order)->amount;
        else
        {
            order_amount = partial_amount;
            partial_price = it_order->price;
        }

        it_remove_order = --(it_order.base());  // сохраняем итератор на первую заявку для удаления (в порядке возрастания цены)
        remove_amount = order_amount;           // сохраняем кол-во btc проданных по этой заявке

        while (true)
        {
            it_order->client->BuyBTCUSD(it_order->price, order_amount);

            if (it_order == setOrderBuy.rbegin())
                break;

            order_amount = (--it_order)->amount;
        }
    }


    if (it_partial_order == setOrderBuy.rend())
    {
        // корректируем баланс тейк-профит-"покупателей"

        int64_t tp_amount = 0;
        while(tp_amount < partial_amount)
        {
            auto it_tp = setTPUSD.begin();
            assert(it_tp->usd > 0);
            int64_t amount = min(partial_amount - tp_amount, it_tp->usd);
            it_tp->client->BuyBTCUSD(tp_buy_price, amount);
            tp_amount += amount;
        }
    }


    // ликвидируем пользователей

    Client* current_client = nullptr;
    int64_t excluded_partial_amount = 0;
    for (auto it_step = vecLiquidated.begin(); ; ++it_step)
    {
        assert(it_step != vecLiquidated.end());

        if (current_client != it_step->balance.client)
        {
            if (current_client)
                if (current_client->usd < 0)
                    setDebtUSD.emplace(*current_client);
//                else if (current_client->btc < 0)
//                    setDebtBTC.emplace(*current_client);

            current_client = it_step->balance.client;

            if (current_client->usd < 0)
                setDebtUSD.erase(*current_client);
//                else if (current_client->btc < 0)
//                    setDebtBTC.erase(*current_client);

            current_client->usd = 0;
            current_client->btc = 0;
        }

        if (it_step->itOrder == it_partial_order)
        {
            excluded_partial_amount += it_step->amount;
            if (excluded_partial_amount >= partial_amount)
            {
                int64_t amount = partial_amount - excluded_partial_amount + it_step->amount;
                *(Balance*)current_client = it_step->balance;
                current_client->btc -= amount;
                current_client->usd += amount * partial_price;
                if (current_client->usd < 0)
                    setDebtUSD.emplace(*current_client);
//                else if (current_client->btc < 0)
//                    setDebtBTC.emplace(*current_client);
                break;
            }
        }
    }

    if (setOrderBuy.empty())
        return;


    // корректируем заявки

//        Order partial_order(0,0,nullptr);

//        // удаляем исполненные заявки
//        auto it_remove_order = it_partial_order.base();
//        if (it_partial_order != setOrderBuy.rend())
//        {
//            --it_remove_order;
//            partial_order = *it_remove_order;
//            partial_order.amount -= partial_amount;
//            assert(partial_order.amount >= 0);
//        }
//        setOrderBuy.erase(it_remove_order, setOrderBuy.end());

//        // добавляем частично исполненную
//        if (partial_order.amount > 0)
//            setOrderBuy.insert(partial_order);

    Order partial_order(*it_remove_order);
    partial_order.amount -= remove_amount;
    assert(partial_order.amount >= 0);

    // удаляем исполненные заявки
    setOrderBuy.erase(it_remove_order, setOrderBuy.end());

    // добавляем частично исполненную
    if (partial_order.amount > 0)
        setOrderBuy.insert(partial_order);
}


void Liquidation::Process()
{
    if (setDebtUSD.empty())
        return;

    set<Client*> setLiquidated;                             // набор ликвидированных пользователей
    set<BalanceUSD> copyDebtUSD(setDebtUSD);                // основной список должников
    map<Client*, Balance> mapDebtorBalance;                 // текущий баланс пользователей в ходе симуляции

    for (const auto& debt: setDebtUSD)
    {
        assert(debt.usd < 0 && debt.btc > 0);
        mapDebtorBalance[debt.client] = debt;
    }

    int64_t total_users_balance = 0;
    int64_t partial_tp_amount = 0;
    int64_t tp_buy_amount = 0;

    int64_t order_volume = 0;                               // счетчик объема расходуемых заявок
    int64_t max_order_volume = 0;                           // максимальная по суммарному объему заявка
    int64_t max_order_amount = 0;                           // кол-во btc запрашиваемых в максимальной по объему заявке

    int64_t order_amount;
    int64_t order_price;

    auto it_order = setOrderBuy.rbegin();
    if (it_order != setOrderBuy.rend())
    {
        order_amount = it_order->amount;
        order_price = it_order->price;
    }

    // симулируем ликвидацию всех должников
    while (!copyDebtUSD.empty() && copyDebtUSD.begin()->usd < 0)
    {
        auto it_user = copyDebtUSD.begin();
        Client* client = it_user->client;
        setLiquidated.insert(client);
        BalanceUSD user_balance(*it_user);                      // keep current user balance

        total_users_balance += it_user->usd;
        int64_t user_assets = it_user->btc;
        copyDebtUSD.erase(it_user);
        mapDebtorBalance.erase(client);                         // this is unessential

        while (it_order != setOrderBuy.rend() && user_assets > 0)
        {
            // не можем продавать ликвидированному должнику
            if (!setLiquidated.count(it_order->client))
            {
                // корректируем состояние должников
                int64_t amount = min(user_assets, order_amount);
                int64_t volume = amount*order_price;
                total_users_balance += volume;
                user_assets -= amount;
                order_amount -= amount;

                vecLiquidated.emplace_back(user_balance, it_order, amount);
                user_balance.btc = user_assets;
                user_balance.usd += volume;

                order_volume += volume;
                if (order_volume > max_order_volume)
                {
                    max_order_volume = order_volume;
                    max_order_amount = it_order->amount - order_amount;
                }

                // находим либо добавляем текущий баланс покупателя во вспомогательный список
                auto buyer = mapDebtorBalance.emplace(it_order->client, *it_order->client);

                if (!buyer.second)
                {
                    // клиент уже был в списке
                    // находим покупателя в основном списке по его текущему балансу
                    set<BalanceUSD>::iterator it_buyer_debt;
                    it_buyer_debt = copyDebtUSD.find(BalanceUSD(buyer.first->second, it_order->client));
                    assert(it_buyer_debt != copyDebtUSD.end());
                    // удаляем старый баланс покупателя из основного списка должников
                    copyDebtUSD.erase(it_buyer_debt);
                }

                // корректируем баланс покупателя для вспомогательного списка
                buyer.first->second.usd -= amount * order_price;
                buyer.first->second.btc += amount;

                // добавляем новый баланс покупателя в основной список должников
                copyDebtUSD.emplace(buyer.first->second, it_order->client);
            }
            else
                order_amount = 0;

            if (order_amount == 0)
            {
                if (++it_order == setOrderBuy.rend())
                    break;

                order_volume = 0;
                order_amount = it_order->amount;
                order_price = it_order->price;
            }
        }

        int64_t order_used_amount;                 // кол-во проданных монет по текущей заявке
        if (it_order == setOrderBuy.rend())
        {
            // дальше ликвидируемся по принудительным тейк-профитам держателей usd
            tp_buy_amount += user_assets;
            order_used_amount = tp_buy_amount;
            total_users_balance += user_assets * tp_buy_price;
            vecLiquidated.emplace_back(user_balance, it_order, user_assets);
            // user_balance не корректируем, поскольку он нам больше не нужен
        }
        else
            order_used_amount = it_order->amount - order_amount;

        // заявок должно хватить на ликвидацию всех пользователей
        assert(total_users_balance >= 0);

        // ликвидация может быть инициирована по одному из двух событий:
        //   - критическое снижение цены для принудительных тейк-профитов, при которой будет невозможно ликвидировать все позиции
        //   - наличие критической заявки, без которой будет невозможно ликвидировать все позиции

        int total_users_balance_new = total_users_balance - tp_buy_amount * tp_buy_price_delta;
        if (total_users_balance_new < 0)
        {
            // критическое снижение цены - будет ликвидация по тейк-профитам
            // считаем объем тейк-профитов, необходимый для ликвидации текущего пользователя
            int64_t amount = 1 + (-total_users_balance_new - 1) / tp_buy_price_delta;
            assert(tp_buy_amount >= amount);
            partial_tp_amount = max(amount, partial_tp_amount);
        }

        // если критическая заявка потенциально возможна, то сохраняем состояние для дальнейшей проверки
        if (total_users_balance - max_order_volume + max_order_amount * tp_buy_price < 0)
        {
            // сохраняем состояние ликвидации
            vecState.emplace_back(client, it_order, order_used_amount, total_users_balance);
        }
    }

    // vecState содержит потенциальные состояния для ликвидации по снятию заявки
    // partial_tp_amount - объем тейк-профитов, необходимый для ликвидации по снижению цены

    if (partial_tp_amount > 0)
        return ExecuteOrders(setOrderBuy.rend(), partial_tp_amount);

    CheckCriticalOrders();
}

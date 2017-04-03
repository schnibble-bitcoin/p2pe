#include "order.h"
#include "client.h"

bool Order::operator <(const Order& right) const
{
    return (price == right.price)? *client < *right.client : price < right.price;
}

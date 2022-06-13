#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include <functional>
using namespace std;
string checkthisstring;
class Order {
public:
    string orderid;
    string buy_or_sell_type;
    unsigned int quantity;
    long price;
    unsigned int display_size;
};

vector<string> split_string(const string& input) {
    vector<string> out;
    istringstream f(input);
    string s;
    while (getline(f, s, ' ')) {
        out.push_back(s);
    }
    return out;
};

list<Order> buy_orders;
//add padding here to have books on different cache lines
list<Order> sell_orders;
bool nodebug = false;
void print_ob(bool print = false) {
    std::ostringstream cout2;
    if (nodebug && !print) {
        return;
    }
    cout << "B: ";
    cout2 << "B: ";
    for (auto& e : buy_orders) {
        cout << e.quantity << "@" << e.price << "#" << e.orderid << " ";
        cout2 << e.quantity << "@" << e.price << "#" << e.orderid << " ";
    }
    cout << endl;
    cout2 << endl;
    cout << "S: ";
    cout2 << "S: ";
    for (auto& e : sell_orders) {
        cout << e.quantity << "@" << e.price << "#" << e.orderid << " ";
        cout2 << e.quantity << "@" << e.price << "#" << e.orderid << " ";
    }
    cout << endl;
    cout2 << endl << endl;
    checkthisstring += cout2.str();
};

void insert_buy(Order& order) {
    if (buy_orders.empty()) {
        buy_orders.push_front(order);
    }
    else {
        auto found = std::find_if(begin(buy_orders), end(buy_orders), [&order](auto& e) {
            return order.price > e.price;
            });
        buy_orders.insert(found, order);
    }
}
//implement iceberg
// fix fok implementation to be faster
// 

//implement different matching algo
//fifo
//pro rata price*quantity
//top if there is someone with quantity >= of all quantity for given price level he gets filled first(if he his req has the same price)
//lead market maker = vip programme - no matter where in the queue your order is if the price match you will get x % of current sell/buy offer at given price to fill your order
void insert_sell(Order& order) {
    if (sell_orders.empty()) {
        sell_orders.push_front(order);
    }
    else {
        //it should be rbegin here for better performance(more often new price added to the end because of bussiness requirements)
        auto found = std::find_if(begin(sell_orders), end(sell_orders), [&order](auto& e) {
            return order.price < e.price;
            });
        sell_orders.insert(found, order);
    }
}
auto find_order_sell_book(string& orderid) {
    return std::find_if(begin(sell_orders), end(sell_orders), [&orderid](auto& e) {
        return e.orderid == orderid;
        });
}
auto find_order_buy_book(string& orderid) {
    return std::find_if(begin(buy_orders), end(buy_orders), [&orderid](auto& e) {
        return e.orderid == orderid;
        });
}
void cancel_order(string& orderid) {
    auto size = sell_orders.size();
    sell_orders.remove_if([&orderid](auto& e) {
        return e.orderid == orderid;
        });
    if (size != sell_orders.size()) {
        return;
    }
    buy_orders.remove_if([&orderid](auto& e) {
        return e.orderid == orderid;
        });
}

void handle_order_lo(Order& loorder) {
    long output = 0;
    if (loorder.buy_or_sell_type == "B") {
        long price = loorder.price;
        unsigned int quantity = loorder.quantity;
        string& buy_or_sell = loorder.buy_or_sell_type;
        string& orderid = loorder.orderid; // keep the cache hot
        if (!sell_orders.empty() && sell_orders.front().price <= price) { //buy sell match
            if (sell_orders.front().quantity == quantity) { // perfect match 1 1
                output += sell_orders.front().price * sell_orders.front().quantity;
                sell_orders.pop_front();
            }
            else if (sell_orders.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !sell_orders.empty() && sell_orders.front().price <= price) {
                    if (sell_orders.front().quantity == quantity) {
                        output += sell_orders.front().price * sell_orders.front().quantity;
                        sell_orders.pop_front();
                    }
                    else if (sell_orders.front().quantity < quantity) { //order from ob filled, input order not fully
                        output += sell_orders.front().price * sell_orders.front().quantity;
                        quantity -= sell_orders.front().quantity;
                        sell_orders.pop_front();
                    }
                    else { //order from ob filled partialy
                        output += sell_orders.front().price * quantity;
                        sell_orders.front().quantity -= quantity;
                        //TODO: update str
                        quantity = 0;
                    }
                }
                if (quantity != 0) {
                    Order buyorder;
                    buyorder.quantity = quantity;
                    buyorder.orderid = orderid;
                    buyorder.buy_or_sell_type = buy_or_sell;
                    buyorder.price = price;
                    insert_buy(buyorder);
                }
            }
            else { // order quantity in ob > input order quantity
                output += sell_orders.front().price * quantity;
                sell_orders.front().quantity -= quantity;
                //TODO: update str
            }
        }
        else { //no match add order to ob
            Order buyorder;
            buyorder.quantity = quantity;
            buyorder.orderid = orderid;
            buyorder.buy_or_sell_type = buy_or_sell;
            buyorder.price = price;
            insert_buy(buyorder);
        }
    }
    else if (loorder.buy_or_sell_type == "S") {
        long price = loorder.price;
        unsigned int quantity = loorder.quantity;
        string& buy_or_sell = loorder.buy_or_sell_type;
        string& orderid = loorder.orderid; // keep the cache hot
        if (!buy_orders.empty() && buy_orders.front().price >= price) { //buy sell match
            if (buy_orders.front().quantity == quantity) { // perfect match 1 1
                output += buy_orders.front().price * buy_orders.front().quantity;
                buy_orders.pop_front();
            }
            else if (buy_orders.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !buy_orders.empty() && buy_orders.front().price >= price) {
                    if (buy_orders.front().quantity == quantity) {
                        output += buy_orders.front().price * buy_orders.front().quantity;
                        buy_orders.pop_front();
                    }
                    else if (buy_orders.front().quantity < quantity) { //order from ob filled, input order not fully
                        output += buy_orders.front().price * buy_orders.front().quantity;
                        quantity -= buy_orders.front().quantity;
                        buy_orders.pop_front();
                    }
                    else { //order from ob filled partialy
                        output += buy_orders.front().price * quantity;
                        buy_orders.front().quantity -= quantity;
                        //TODO: update str
                        quantity = 0;

                    }
                }
                if (quantity != 0) {
                    Order sellorder;
                    sellorder.quantity = quantity;
                    sellorder.orderid = orderid;
                    sellorder.buy_or_sell_type = buy_or_sell;
                    sellorder.price = price;
                    insert_sell(sellorder);
                }
            }
            else { // order quantity in ob > input order quantity
                output += buy_orders.front().price * quantity;
                buy_orders.front().quantity -= quantity;

                //TODO: update str
            }
        }
        else {
            Order sellorder;
            sellorder.quantity = quantity;
            sellorder.orderid = orderid;
            sellorder.buy_or_sell_type = buy_or_sell;
            sellorder.price = price;
            insert_sell(sellorder);
        }
    }
    cout << output << endl;
    print_ob();
}

void handle_order_ice(Order& loorder) {
    long output = 0;
    if (loorder.buy_or_sell_type == "B") {
        long price = loorder.price;
        unsigned int quantity = loorder.quantity;
        string& buy_or_sell = loorder.buy_or_sell_type;
        string& orderid = loorder.orderid; // keep the cache hot
        unsigned int display_size = loorder.display_size;
        if (!sell_orders.empty() && sell_orders.front().price <= price) { //buy sell match
            if (sell_orders.front().quantity == quantity) { // perfect match 1 1
                output += sell_orders.front().price * sell_orders.front().quantity;
                sell_orders.pop_front();
            }
            else if (sell_orders.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !sell_orders.empty() && sell_orders.front().price <= price) {
                    if (sell_orders.front().quantity == quantity) {
                        output += sell_orders.front().price * sell_orders.front().quantity;
                        sell_orders.pop_front();
                    }
                    else if (sell_orders.front().quantity < quantity) { //order from ob filled, input order not fully
                        output += sell_orders.front().price * sell_orders.front().quantity;
                        quantity -= sell_orders.front().quantity;
                        sell_orders.pop_front();
                    }
                    else { //order from ob filled partialy
                        output += sell_orders.front().price * quantity;
                        sell_orders.front().quantity -= quantity;
                        //TODO: update str
                        quantity = 0;
                        //TODO: insert new order to the book
                        if (display_size - loorder.quantity > 0) {
                            Order new_order;
                            new_order.quantity = (display_size - loorder.quantity) % 101;
                            new_order.price = price;
                            new_order.buy_or_sell_type = "B";

                            new_order.display_size = display_size - loorder.quantity;
                            handle_order_lo(loorder);
                        }
                    }
                }
                if (quantity != 0) {
                    Order buyorder;
                    buyorder.quantity = quantity;
                    buyorder.orderid = orderid;
                    buyorder.buy_or_sell_type = buy_or_sell;
                    buyorder.price = price;
                    buyorder.display_size = display_size;
                    insert_buy(buyorder);
                }
            }
            else { // order quantity in ob > input order quantity
                output += sell_orders.front().price * quantity;
                sell_orders.front().quantity -= quantity;
                //TODO: update str
            }
        }
        else { //no match add order to ob
            Order buyorder;
            buyorder.quantity = quantity;
            buyorder.orderid = orderid;
            buyorder.buy_or_sell_type = buy_or_sell;
            buyorder.price = price;
            buyorder.display_size = display_size;
            insert_buy(buyorder);
        }
    }
    else if (loorder.buy_or_sell_type == "S") {
        long price = loorder.price;
        unsigned int quantity = loorder.quantity;
        string& buy_or_sell = loorder.buy_or_sell_type;
        string& orderid = loorder.orderid; // keep the cache hot
        unsigned int display_size = loorder.display_size;
        if (!buy_orders.empty() && buy_orders.front().price >= price) { //buy sell match
            if (buy_orders.front().quantity == quantity) { // perfect match 1 1
                output += buy_orders.front().price * buy_orders.front().quantity;
                buy_orders.pop_front();
            }
            else if (buy_orders.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !buy_orders.empty() && buy_orders.front().price >= price) {
                    if (buy_orders.front().quantity == quantity) {
                        output += buy_orders.front().price * buy_orders.front().quantity;
                        buy_orders.pop_front();
                    }
                    else if (buy_orders.front().quantity < quantity) { //order from ob filled, input order not fully
                        output += buy_orders.front().price * buy_orders.front().quantity;
                        quantity -= buy_orders.front().quantity;
                        buy_orders.pop_front();
                    }
                    else { //order from ob filled partialy
                        output += buy_orders.front().price * quantity;
                        buy_orders.front().quantity -= quantity;
                        //TODO: update str
                        quantity = 0;
                        //TODO : create new order
                        if (display_size - loorder.quantity > 0) {
                            Order new_order;
                            new_order.quantity = (display_size - loorder.quantity) % 101;
                            new_order.price = price;
                            new_order.buy_or_sell_type = "S";

                            new_order.display_size = display_size - loorder.quantity;
                            handle_order_lo(loorder);
                        }
                    }
                }
                if (quantity != 0) {
                    Order sellorder;
                    sellorder.quantity = quantity;
                    sellorder.orderid = orderid;
                    sellorder.buy_or_sell_type = buy_or_sell;
                    sellorder.price = price;
                    sellorder.display_size = display_size;
                    insert_sell(sellorder);
                }
            }
            else { // order quantity in ob > input order quantity
                output += buy_orders.front().price * quantity;
                buy_orders.front().quantity -= quantity;

                //TODO: update str
            }
        }
        else {
            Order sellorder;
            sellorder.quantity = quantity;
            sellorder.orderid = orderid;
            sellorder.buy_or_sell_type = buy_or_sell;
            sellorder.price = price;
            sellorder.display_size = display_size;
            insert_sell(sellorder);
        }
    }
    cout << output << endl;
    print_ob();
};

void handle_order_mo(Order loorder) {
    long output = 0;
    if (loorder.buy_or_sell_type == "B") {
        unsigned int quantity = loorder.quantity;
        string& buy_or_sell = loorder.buy_or_sell_type;
        string& orderid = loorder.orderid; // keep the cache hot
        if (!sell_orders.empty()) { //buy sell match
            if (sell_orders.front().quantity == quantity) { // perfect match 1 1
                output += sell_orders.front().price * sell_orders.front().quantity;
                sell_orders.pop_front();
            }
            else if (sell_orders.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !sell_orders.empty()) {
                    if (sell_orders.front().quantity == quantity) {
                        output += sell_orders.front().price * sell_orders.front().quantity;
                        sell_orders.pop_front();
                    }
                    else if (sell_orders.front().quantity < quantity) { //order from ob filled, input order not fully
                        output += sell_orders.front().price * sell_orders.front().quantity;
                        quantity -= sell_orders.front().quantity;
                        sell_orders.pop_front();
                    }
                    else { //order from ob filled partialy
                        output += sell_orders.front().price * quantity;
                        sell_orders.front().quantity -= quantity;
                        //TODO: update str
                        quantity = 0;
                    }
                }
                if (quantity != 0) {
                    //cancel remaining
                }
            }
            else { // order quantity in ob > input order quantity
                output += sell_orders.front().price * quantity;
                sell_orders.front().quantity -= quantity;
                //TODO: update str
            }
        }
        else { //no match add order to ob
            //noop
        }
    }
    else if (loorder.buy_or_sell_type == "S") {
        long price = loorder.price;
        unsigned int quantity = loorder.quantity;
        string& buy_or_sell = loorder.buy_or_sell_type;
        string& orderid = loorder.orderid; // keep the cache hot
        if (!buy_orders.empty()) { //buy sell match
            if (buy_orders.front().quantity == quantity) { // perfect match 1 1
                output += buy_orders.front().price * buy_orders.front().quantity;
                buy_orders.pop_front();
            }
            else if (buy_orders.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !buy_orders.empty()) {
                    if (buy_orders.front().quantity == quantity) {
                        output += buy_orders.front().price * buy_orders.front().quantity;
                        buy_orders.pop_front();
                    }
                    else if (buy_orders.front().quantity < quantity) { //order from ob filled, input order not fully
                        output += buy_orders.front().price * buy_orders.front().quantity;
                        quantity -= buy_orders.front().quantity;
                        buy_orders.pop_front();
                    }
                    else { //order from ob filled partialy
                        output += buy_orders.front().price * quantity;
                        buy_orders.front().quantity -= quantity;
                        //TODO: update str
                        quantity = 0;
                    }
                }
                if (quantity != 0) {
                    //cancel remaining
                }
            }
            else { // order quantity in ob > input order quantity
                output += buy_orders.front().price * quantity;
                buy_orders.front().quantity -= quantity;

                //TODO: update str
            }
        }
        else {
            //noop;
        }
    }
    cout << output << endl;
    print_ob();
}

void handle_order_ioc(Order loorder) {
    long output = 0;
    if (loorder.buy_or_sell_type == "B") {
        long price = loorder.price;
        unsigned int quantity = loorder.quantity;
        string& buy_or_sell = loorder.buy_or_sell_type;
        string& orderid = loorder.orderid; // keep the cache hot
        if (!sell_orders.empty() && sell_orders.front().price <= price) { //buy sell match
            if (sell_orders.front().quantity == quantity) { // perfect match 1 1
                output += sell_orders.front().price * sell_orders.front().quantity;
                sell_orders.pop_front();
            }
            else if (sell_orders.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !sell_orders.empty() && sell_orders.front().price <= price) {
                    if (sell_orders.front().quantity == quantity) {
                        output += sell_orders.front().price * sell_orders.front().quantity;
                        sell_orders.pop_front();
                    }
                    else if (sell_orders.front().quantity < quantity) { //order from ob filled, input order not fully
                        output += sell_orders.front().price * sell_orders.front().quantity;
                        quantity -= sell_orders.front().quantity;
                        sell_orders.pop_front();
                    }
                    else { //order from ob filled partialy
                        output += sell_orders.front().price * quantity;
                        sell_orders.front().quantity -= quantity;
                        //TODO: update str
                        quantity = 0;
                    }
                }
                if (quantity != 0) {
                    //canceled
                }
            }
            else { // order quantity in ob > input order quantity
                output += sell_orders.front().price * quantity;
                sell_orders.front().quantity -= quantity;
                //TODO: update str
            }
        }
        else { //no match add order to ob
            //should this be here? ambigious definition of ioc
            Order buyorder;
            buyorder.quantity = quantity;
            buyorder.orderid = orderid;
            buyorder.buy_or_sell_type = buy_or_sell;
            buyorder.price = price;
            insert_buy(buyorder);
        }
    }
    else if (loorder.buy_or_sell_type == "S") {
        long price = loorder.price;
        unsigned int quantity = loorder.quantity;
        string& buy_or_sell = loorder.buy_or_sell_type;
        string& orderid = loorder.orderid; // keep the cache hot
        if (!buy_orders.empty() && buy_orders.front().price >= price) { //buy sell match
            if (buy_orders.front().quantity == quantity) { // perfect match 1 1
                output += buy_orders.front().price * buy_orders.front().quantity;
                buy_orders.pop_front();
            }
            else if (buy_orders.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !buy_orders.empty() && buy_orders.front().price >= price) {
                    if (buy_orders.front().quantity == quantity) {
                        output += buy_orders.front().price * buy_orders.front().quantity;
                        buy_orders.pop_front();
                    }
                    else if (buy_orders.front().quantity < quantity) { //order from ob filled, input order not fully
                        output += buy_orders.front().price * buy_orders.front().quantity;
                        quantity -= buy_orders.front().quantity;
                        buy_orders.pop_front();
                    }
                    else { //order from ob filled partialy
                        output += buy_orders.front().price * quantity;
                        buy_orders.front().quantity -= quantity;
                        //TODO: update str
                        quantity = 0;
                    }
                }
                if (quantity != 0) {
                    //canceled
                }
            }
            else { // order quantity in ob > input order quantity
                output += buy_orders.front().price * quantity;
                buy_orders.front().quantity -= quantity;

                //TODO: update str
            }
        }
        else {
            //should this be here? ambigious definition of ioc
            Order sellorder;
            sellorder.quantity = quantity;
            sellorder.orderid = orderid;
            sellorder.buy_or_sell_type = buy_or_sell;
            sellorder.price = price;
            insert_sell(sellorder);
        }
    }
    cout << output << endl;
    print_ob();
}


void ob_order_filled_input_order_filled_partially(list<Order>& book, long& output, unsigned int& quantity) {
    output += book.front().price * book.front().quantity;
    quantity -= book.front().quantity;
    book.pop_front();
};

void buy_sell_perfect_match(list<Order>& book, long& output) {
    output += book.front().price * book.front().quantity;
    book.pop_front();
};

void order_quantity_in_ob_gt_in_input_order(list<Order>& book, long& output, unsigned int& quantity) {
    output += book.front().price * quantity;
    book.front().quantity -= quantity;
};

void fill_ob_order_partially(list<Order>& book, long& output, unsigned int& quantity) {
    output += book.front().price * quantity;
    book.front().quantity -= quantity;
    quantity = 0;
};

class Oper {
public:
    long output_to_add{ 0 };
    unsigned int quantity_to_deduct{ 0 };
    bool do_ob_pop_front{ false };
};
void handle_order_fok(Order loorder) {
    long output = 0;
    long price = loorder.price;
    unsigned int quantity = loorder.quantity;
    string& buy_or_sell = loorder.buy_or_sell_type;
    string& orderid = loorder.orderid; // keep the cache hot
    //vector<Oper> opers;



    if (loorder.buy_or_sell_type == "B") {
        list<Order> sell_orders2 = sell_orders;

        if (!sell_orders2.empty() && sell_orders.front().price <= price) { //buy sell match
            if (sell_orders2.front().quantity == quantity) { // perfect match 1 1
                //Oper oper;
               // oper.output_to_add=sell_orders2.front().price * sell_orders2.front().quantity;
                //oper.do_ob_pop_front=true;
               // opers.push_back(oper);

                output += sell_orders2.front().price * sell_orders2.front().quantity;
                sell_orders2.pop_front();
            }
            else if (sell_orders2.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !sell_orders2.empty() && sell_orders2.front().price <= price) {
                    if (sell_orders2.front().quantity == quantity) {
                        // opers.push_back([&output](){                    
                         //    buy_sell_perfect_match(sell_orders2, output);
                         //});

                        output += sell_orders2.front().price * sell_orders2.front().quantity;
                        sell_orders2.pop_front();
                    }
                    else if (sell_orders2.front().quantity < quantity) { //order from ob filled, input order not fully
                        //opers.push_back([&output, &quantity](){
                        //    ob_order_filled_input_order_filled_partially(sell_orders2, output, quantity);
                        //});

                        output += sell_orders2.front().price * sell_orders2.front().quantity;
                        quantity -= sell_orders2.front().quantity;
                        sell_orders2.pop_front();
                    }
                    else { //order from ob filled partialy
                        //opers.push_back([&output, &quantity](){
                        //    fill_ob_order_partially(sell_orders2, output, quantity);
                        //});

                        output += sell_orders2.front().price * quantity;
                        sell_orders2.front().quantity -= quantity;
                        quantity = 0;
                    }
                }
                if (quantity != 0) {
                    //canceled
                    //noop revert whole transaction!
                    cout << "0" << endl;
                    return;
                }
            }
            else { // order quantity in ob > input order quantity
                //opers.push_back([&output, &quantity](){
                //    order_quantity_in_ob_gt_in_input_order(sell_orders2,output,quantity);
                //});

                output += sell_orders2.front().price * quantity;
                sell_orders2.front().quantity -= quantity;

            }
        }
        else { //no match add order to ob
            //noop
        }
    }
    else if (loorder.buy_or_sell_type == "S") {
        list<Order> buy_orders2 = buy_orders;
        if (!buy_orders2.empty() && buy_orders.front().price >= price) { //buy sell match
            if (buy_orders2.front().quantity == quantity) { // perfect match 1 1
               // opers.push_back([&output]() {
                //    buy_sell_perfect_match(buy_orders2, output);
                //});  

                output += buy_orders2.front().price * buy_orders2.front().quantity;
                buy_orders2.pop_front();
            }
            else if (buy_orders2.front().quantity < quantity) { //quantity of 1 order in book not enough
                while (quantity != 0 && !buy_orders2.empty() && buy_orders2.front().price >= price) {
                    if (buy_orders2.front().quantity == quantity) {
                        //opers.push_back([&output](){                    
                       //    buy_sell_perfect_match(buy_orders2, output);
                        //}); 

                        output += buy_orders2.front().price * buy_orders2.front().quantity;
                        buy_orders2.pop_front();
                    }
                    else if (buy_orders2.front().quantity < quantity) { //order from ob filled, input order not fully
                        //opers.push_back([&output, &quantity](){
                        //    ob_order_filled_input_order_filled_partially(buy_orders2, output, quantity);
                        //});

                        output += buy_orders2.front().price * buy_orders2.front().quantity;
                        quantity -= buy_orders2.front().quantity;
                        buy_orders2.pop_front();
                    }
                    else { //order from ob filled partialy
                        //opers.push_back([&output, &quantity](){
                        //    fill_ob_order_partially(buy_orders2, output, quantity);
                        //});

                        output += buy_orders2.front().price * quantity;
                        buy_orders2.front().quantity -= quantity;
                        quantity = 0;
                    }
                }
                if (quantity != 0) {
                    //canceled
                    //TODO
                    //opers.clear();
                    cout << "0" << endl;
                    return;
                }
            }
            else { // order quantity in ob > input order quantity
                //opers.push_back([&output, &quantity](){
                //    order_quantity_in_ob_gt_in_input_order(buy_orders2,output,quantity);
                //}); 

                output += buy_orders2.front().price * quantity;
                buy_orders2.front().quantity -= quantity;
            }
            //commit transaction
            buy_orders = buy_orders2;
        }
        else { //no match add order to ob
            //noop
        }
    }

    //for(auto& o : opers) {
    //    o();
    //}

    cout << output << endl;
    print_ob();
}
void handle_order_crp(Order loorder) {
    long output = 0;
    long price = loorder.price;
    unsigned int quantity = loorder.quantity;
    string& orderid = loorder.orderid;
    auto found = find_order_buy_book(orderid);
    if (found != end(buy_orders)) {
        //found
        long old_price = found->price;
        string buy_type = found->buy_or_sell_type;
        unsigned int old_quantity = found->quantity;
        if (old_price == price && old_quantity > quantity) {
            //inplace change            
            found->quantity = quantity;
            //cout << "0" << endl;
        }
        else {
            cancel_order(orderid);
            loorder.buy_or_sell_type = "B";
            insert_buy(loorder);
            //cout << "0" << endl;
        }
    }
    else {
        auto found = find_order_sell_book(orderid);
        if (found != end(sell_orders)) {
            //found
            long old_price = found->price;
            string buy_type = found->buy_or_sell_type;
            unsigned int old_quantity = found->quantity;
            if (old_price == price && old_quantity > quantity) {
                //inplace change            
                found->quantity = quantity;
                //cout << "0" << endl;
            }
            else {
                cancel_order(orderid);
                loorder.buy_or_sell_type = "S";
                insert_sell(loorder);
                //cout << "0" << endl;
            }
            //found
        }
    }



}
void process(const string& input_line) {
    if (!nodebug) {
        cout << "cmd:" << input_line << endl;
    }

    auto splitted = split_string(input_line);
    if (splitted[0] == "SUB") {
        if (splitted[1] == "LO") {
            Order loorder;
            loorder.buy_or_sell_type = splitted[2];
            loorder.price = stol(splitted[5]);
            loorder.quantity = stoul(splitted[4]);
            loorder.orderid = splitted[3];
            handle_order_lo(loorder);
        }
        else if (splitted[1] == "MO") {
            Order moorder;
            moorder.orderid = splitted[3];
            moorder.quantity = stoul(splitted[4]);
            moorder.buy_or_sell_type = splitted[2];
            handle_order_mo(moorder);
        }
        else if (splitted[1] == "IOC") {
            Order loorder;
            loorder.buy_or_sell_type = splitted[2];
            loorder.price = stol(splitted[5]);
            loorder.quantity = stoul(splitted[4]);
            loorder.orderid = splitted[3];
            handle_order_ioc(loorder);
        }
        else if (splitted[1] == "FOK") {
            Order loorder;
            loorder.buy_or_sell_type = splitted[2];
            loorder.price = stol(splitted[5]);
            loorder.quantity = stoul(splitted[4]);
            loorder.orderid = splitted[3];
            handle_order_fok(loorder);
        }
        else if (splitted[1] == "FOK") {
            Order loorder;
            loorder.buy_or_sell_type = splitted[2];
            loorder.price = stol(splitted[5]);
            loorder.quantity = stoul(splitted[4]);
            loorder.orderid = splitted[3];
            handle_order_fok(loorder);
        }
        else if (splitted[1] == "ICE") {
            Order loorder;
            loorder.buy_or_sell_type = splitted[2];
            loorder.price = stol(splitted[5]);
            loorder.quantity = stoul(splitted[6]);
            loorder.display_size = stoull(splitted[4]);
            loorder.orderid = splitted[3];
            handle_order_ice(loorder);
        }
    }
    else if (splitted[0] == "CXL") {
        cancel_order(splitted[1]);
        print_ob();
        //
    }
    else if (splitted[0] == "CRP") {
        Order loorder;
        loorder.price = stol(splitted[3]);
        loorder.quantity = stoul(splitted[2]);
        loorder.orderid = splitted[1];
        handle_order_crp(loorder);

    }
    else if (splitted[0] == "END") {
        print_ob(true);
        //print result
    }
};

int main() {
    vector<string> lines;
    lines.push_back("SUB LO B Ffuj 200 13");
    lines.push_back("SUB LO S Ffuj 200 17");
    lines.push_back("SUB LO S Ffuj 200 17");
    lines.push_back("SUB LO S Ffuj 200 18");
        lines.push_back("SUB LO B Yy7P 150 11");
            lines.push_back("SUB LO B YuFU 100 13");
                lines.push_back("SUB LO S IpD8 150 14");
                    lines.push_back("SUB LO S y93N 190 15");
                        lines.push_back("SUB LO B Y5wb 230 14");
                            lines.push_back("SUB MO B IZLO 250");
                                lines.push_back("CXL Ffuj");
                                    lines.push_back("CXL 49Ze");
                                        lines.push_back("END");
    for (auto& line : lines) {
    //for (string line; getline(cin, line);) {
        process(line);
        if (line[0] == 'E') {
            break;
        }

    }

    cout << "TERA TO=====\n\n" << checkthisstring << "\n\n";
    string expected_string = checkthisstring;
    if (expected_string != checkthisstring) {
        cout << "BAD";
    }
    
    //return 0;
}

/*
SUB LO B Ffuj 200 13
SUB LO B Yy7P 150 11
SUB LO B YuFU 100 13
SUB LO S IpD8 150 14
SUB LO S y93N 190 15
SUB LO B Y5wb 230 14
SUB MO B IZLO 250
CXL Ffuj
CXL 49Ze
END
*/

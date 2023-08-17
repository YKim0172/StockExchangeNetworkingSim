#include "StockExchange.h"
#include <iostream>

bool StockExchange::createOrder(bool isBuying, double price, uint32_t quantity, uint32_t user_id, std::string ticker)
{
    //first locate ticker
    auto tickersListItr = tickersList.find(ticker);
    if (tickersListItr != tickersList.end()) {  //if ticker already exists / We know that the ticker must exist for trade to even be possible
        std::map<double, std::pair<std::deque<orderInfo>, std::deque<orderInfo>>> priceToOrderBook = tickersList[ticker];
        //now check if the price exists
        auto priceToOrderBookItr = priceToOrderBook.find(price);
        if (priceToOrderBookItr != priceToOrderBook.end()) {  //if the price is already listed /this is when trades can happen
            orderInfo newOrder = { user_id, quantity };
            if (isBuying) {  //if bid
                //check to see if trades can be made
                //check to see if there are any ASK orders
                if (tickersList[ticker][price].second.size() == 0) { //just add 
                    tickersList[ticker][price].first.push_back(newOrder);
                }
                else {  //trade can happen so please
                    std::deque<orderInfo>& listOfAsks = tickersList[ticker][price].second;
                    while (listOfAsks.size() != 0 && quantity != 0) {
                        //go through each of the sales and start knocking off
                        orderInfo& newestOrder = listOfAsks.front();
                        if (quantity >= newestOrder.quantity) {  //get rid of newest Order
                            std::cout << "Order Executed: User " << newestOrder.user_id << "'s ASK for " << ticker << " @ " << price << " for " << newestOrder.quantity << " shares.\n";
                            std::cout << "Order Executed: User " << user_id << "'s BID for " << ticker << " @ " << price << " for " << newestOrder.quantity << " shares.\n";
                            quantity -= newestOrder.quantity;
                            //dont add, just delete/pop
                            tickersList[ticker][price].second.pop_front();
                        }
                        else {
                            //for the order already in
                            std::cout << "Order Executed: User " << newestOrder.user_id << "'s ASK for " << ticker << " @ " << price << " for " << quantity << " shares.\n";
                            std::cout << "Order Executed: User " << user_id << "'s BID for " << ticker << " @ " << price << " for " << quantity << " shares.\n";
                            
                            //update entry already in
                            newestOrder.quantity -= quantity;
                            break;
                        }
                    }
                    if (quantity != 0 && listOfAsks.size() == 0) {
                        newOrder.quantity = quantity;
                        tickersList[ticker][price].first.push_back(newOrder);
                    }
                }
            }
            else {  //if ask
                if (tickersList[ticker][price].first.size() == 0) { //just add 
                    tickersList[ticker][price].second.push_back(newOrder);
                }
                else {
                    std::deque<orderInfo>& listOfBids = tickersList[ticker][price].first;
                    while (listOfBids.size() != 0 && quantity != 0) {
                        //go through each of the sales and start knocking off
                        orderInfo& newestOrder = listOfBids.front();
                        if (quantity >= newestOrder.quantity) {  //get rid of newest Order
                            std::cout << "Order Executed: User " << user_id << "'s ASK for " << ticker << " @ " << price << " for " << newestOrder.quantity << " shares.\n";
                            std::cout << "Order Executed: User " << newestOrder.user_id << "'s BID for " << ticker << " @ " << price << " for " << newestOrder.quantity << " shares.\n";
                            quantity -= newestOrder.quantity;
                            //dont add, just delete/pop
                            tickersList[ticker][price].first.pop_front();
                        }
                        else {
                            //for the order already in
                            std::cout << "Order Executed: User " << user_id << "'s ASK for " << ticker << " @ " << price << " for " << quantity << " shares.\n";
                            std::cout << "Order Executed: User " << newestOrder.user_id << "'s BID for " << ticker << " @ " << price << " for " << quantity << " shares.\n";

                            //update entry already in
                            newestOrder.quantity -= quantity;
                            break;
                        }
                    }
                    if (quantity != 0 && listOfBids.size() == 0) {
                        newOrder.quantity = quantity;
                        tickersList[ticker][price].second.push_back(newOrder);
                    }
                }
            }
        }
        else {  //if the entered price is new
            orderInfo newOrder = { user_id, quantity };
            //add new entry
            std::pair<std::deque<orderInfo>, std::deque<orderInfo>> bidsAsks;
            if (isBuying) {  //put order in bids
                bidsAsks.first.push_back(newOrder);
            }
            else {
                bidsAsks.second.push_back(newOrder);
            }
            tickersList[ticker][price] = bidsAsks;
        }


    }
    else {  //create new entry for ticker
        //create the struct first
        orderInfo newOrder = { user_id, quantity };
        //add new entry
        std::pair<std::deque<orderInfo>, std::deque<orderInfo>> bidsAsks;
        if (isBuying) {  //put order in bids
            bidsAsks.first.push_back(newOrder);
        }
        else {
            bidsAsks.second.push_back(newOrder);
        }
        //we got the pair, so create the map
        std::map<double, std::pair<std::deque<orderInfo>, std::deque<orderInfo>>> priceToOrderBook;
        priceToOrderBook[price] = bidsAsks;
        tickersList[ticker] = priceToOrderBook;
        //check to see if stored

        std::pair< std::deque<orderInfo>, std::deque<orderInfo>> orderBook = tickersList[ticker][price];
        std::cout << orderBook.first.size();
    }
    return true;
}

void StockExchange::printAllData()
{
    for (auto tickerMapPair : tickersList) {
        //pair of ticker, map
        std::cout << "NAME OF TICKER: " << tickerMapPair.first << "\n";
        for (auto priceOrderBookPair: tickerMapPair.second) {
            std::cout << "PRICE: " << priceOrderBookPair.first << "\n";
            std::cout << "BIDS: " << "\n";
            for (auto itr = priceOrderBookPair.second.first.begin(); itr != priceOrderBookPair.second.first.end(); itr++) {
                std::cout << "QUANTITY: " << (*itr).quantity << " USER ID: " << (*itr).user_id << "\n";
            }

            std::cout << "ASKS: " << "\n";
            for (auto itr = priceOrderBookPair.second.second.begin(); itr != priceOrderBookPair.second.second.end(); itr++) {
                std::cout << "QUANTITY: " << (*itr).quantity << " USER ID: " << (*itr).user_id << "\n";
            }
        }
    }
}

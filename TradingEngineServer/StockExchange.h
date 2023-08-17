#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <deque>
#include <string>

class StockExchange {
public:
	StockExchange() : tickersList() {}
	bool createOrder(bool isBuying, double price, uint32_t quantity, uint32_t user_id, std::string ticker);
	void printAllData();
private:
	struct orderInfo {
		uint32_t user_id;
		uint32_t quantity;
	};
	std::unordered_map<std::string, std::map<double, std::pair<std::deque<orderInfo>, std::deque<orderInfo>>>> tickersList;


};


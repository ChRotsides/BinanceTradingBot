

#include <string>
#include <queue>
#include "./binanceApi/BAPI.hpp"
#include <jsoncpp/json/json.h>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <vector>
#ifndef VIRTTRADER_H
#define VIRTTRADER_H
// #define VERBOSE

typedef struct trade
{
    float price;
    float quantity_bought;
};

typedef struct price_t{
    float ask;
    float bid;
};


class virtTrader{

public:
    int64_t uniqueID;
    float starting_quote_balance;
    float current_quote_balance;
    float current_base_balance;
    float pos_size=1;
    float perc_diff_buy=1.01;
    float perc_diff_sell_gain=1.01;
    float perc_diff_sell_loss=1.0066;
    float diff_from_last_trade=1.02;
    int trend_minutes=10;
    int average_minutes=180;
    float tradeLimit=20;
    int active_trades_limit=1;
    float min_trade_bal=20;
    std::string symbol;
    std::string base_asset;
    std::string quote_asset;
    int base_precision;
    int quote_precision;
    float step_size;
    std::deque<float> past_prices;
    std::string dir_name;
    std::deque<trade> active_trades;
    BAPI bapi;
    Json::Reader reader;
    std::ofstream log;

    virtTrader(std::string baseA,std::string quoteA);
    ~virtTrader();
    std::string getDate(int64_t time_epoch);
    void createdir();
    void getPastPrices();
    float getAvailableBalance(std::string asset);
    void getFillAverage(Json::Value res,float &avgFillPrice,float &qty);
    price_t getPrice();
    void makeBuyOrder(price_t prc);
    void makeSellOrder(trade tr);
    float getMA();
    float getTrend(price_t prc);
    bool adequateDifferenceFromLastTrade(price_t prc);
    void checkAndExecute();
    void flushLog();

};

#endif
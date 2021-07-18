#include "./binanceApi/BAPI.hpp"
#include <fstream>
#include <vector>
#include <chrono>
#include <sstream>
#include <deque>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits>
#include <unistd.h>
volatile bool cont=1;
struct trade
{
    float price;
    float quantity_bought;
};

struct price_t{
    float ask;
    float bid;
};
void* exit_routine(void *ptr){

    while(1){
        char c;
        scanf("%c",&c);
        if(c=='q'){
            std::cout<<"The Program will exit in about a minute\n";
            cont=0;
            break;
        }
    }

    return NULL;
}

// #define VERBOSE

class virtTrader{

public:
    int64_t uniqueID;
    float starting_quote_balance;
    float current_quote_balance;
    float current_base_balance;
    float pos_size=1;
    float perc_diff_buy=1.01;
    float perc_diff_sell_gain=1.01;
    float perc_diff_sell_loss=1.0033;
    float diff_from_last_trade=1.02;
    int trend_minutes=10;
    int average_minutes=180;
    float tradeLimit=20;
    int active_trades_limit=1;
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

    virtTrader(std::string baseA,std::string quoteA){
        uniqueID=time(NULL);
        base_asset=baseA;
        quote_asset=quoteA;
        symbol=base_asset+quote_asset;
        createdir();
        log.open("./"+dir_name+"/"+"latest_"+std::to_string(uniqueID)+".log",std::ios::out|std::ios::app);
        
        std::string exchange_info_str=bapi.getExchangeInfo(symbol);
        Json::Value exchange_info;
        if(!reader.parse(exchange_info_str,exchange_info)){
            std::cerr<<"Unable to parse Exchange Info\n";
        }

        base_precision=(exchange_info["symbols"][0]["baseAssetPrecision"]).asInt();
        quote_precision=(exchange_info["symbols"][0]["quotePrecision"]).asInt();
        step_size=std::stof((exchange_info["symbols"][0]["filters"][2]["stepSize"]).asString());
        getPastPrices();
        // std::cout<<"Base: "<<base_precision<<" Quote: "<<quote_precision<<" step_size: "<<step_size<<"\n";
        
    }
    ~virtTrader(){
        log.close();
    }

    std::string getDate(int64_t time_epoch){
        std::stringstream ss;
        struct tm *tm = localtime(&time_epoch);
        char date[20];
        strftime(date, sizeof(date), "%d-%m-%Y", tm);
        ss<<date;
        return ss.str();
    }

    void createdir(){
        std::string date=getDate(uniqueID);
        dir_name="logs/"+symbol+"_"+date+"_logs";
        if(opendir(dir_name.c_str())==NULL){
            mkdir(dir_name.c_str(),0777);
        }
    }

    void getPastPrices(){
        
        int64_t now=time(NULL)*1000;
        int64_t then =now-(average_minutes+20)*MILLIS_IN_A_MINUTE;
        std::string old_data_str=bapi.getKlines(symbol,"1m",then,now);
        Json::Value old_data_json;

        if(!reader.parse(old_data_str,old_data_json)){
            std::cerr<<"Failed to parse old data\n";
        }
        for(int i=0; i<old_data_json.size(); i++){
            past_prices.push_back((std::stof(old_data_json[i][1].asString())+std::stof(old_data_json[i][4].asString()))/2);
        }
    }

    float getAvailableBalance(std::string asset){
        std::string acc_info_str=bapi.getAccountInfo();
        Json::Value acc_info;
        float bal;
        if(!reader.parse(acc_info_str,acc_info)){
            std::cerr<<"Unable to parse account info\n";
        }
        for(int i=0; i<acc_info["balances"].size(); i++){
            if(acc_info["balances"][i]["asset"]==asset){
                bal=std::stof(acc_info["balances"][i]["free"].asString());
            }
        }
        return bal;
    }
    void getFillAverage(Json::Value res,float &avgFillPrice,float &qty){
        float sum_qxp=0;
        float sum_q=0;
        for(int i=0; i<res["fills"].size(); i++){

            float p=std::stof(res["fills"][i]["price"].asString());
            float q=std::stof(res["fills"][i]["qty"].asString());

            sum_qxp+=p*q;
            sum_q+=q;
        }
        //results
        avgFillPrice=sum_qxp/sum_q;
        qty=sum_q;
    }

    price_t getPrice(){

        std::string book_ticker_str=bapi.getBook(symbol);
        Json::Value book_ticker;
        if(!reader.parse(book_ticker_str,book_ticker)){
            std::cerr<<"Unable to read the Book ticker\n";
        }


        #ifdef VERBOSE
            std::cout<<book_ticker<<"\n";
        #endif

        price_t prc;
        prc.ask=std::stof(book_ticker["askPrice"].asString());
        prc.bid=std::stof(book_ticker["bidPrice"].asString());

        return prc;
    }
     
    void makeBuyOrder(price_t prc){
        std::cout<<quote_asset<<"\n";
        current_quote_balance=getAvailableBalance(quote_asset);
        trade new_trade;
        float bal_to_use;
        if(current_quote_balance<tradeLimit){
            bal_to_use=current_quote_balance;
        }else{
            bal_to_use=tradeLimit;
        }
        

        prc.ask+=prc.ask*0.001;//slippage
        float quantity_to_buy=bal_to_use/prc.ask;


        #ifdef VERBOSE
            std::cout<<quantity_to_buy<<"\n";
        #endif


        int conform_to_step=quantity_to_buy/step_size;
        quantity_to_buy=conform_to_step*step_size;
        std::stringstream ss;
        ss<<quantity_to_buy;
        

        std::string order_res_str=bapi.placeOrder(symbol,"BUY","MARKET",ss.str());
        Json::Value order_res;

        if(!reader.parse(order_res_str,order_res)){
            std::cerr<<order_res<<"\n";
            std::cerr<<"Placed Order but Cannot Parse data\n";
        }
        #ifdef VERBOSE
            std::cout<<order_res<<"\n";
        #endif

        if(order_res["status"]!="FILLED"){
            std::cerr<<order_res<<"\n";
            std::cout<<"Order did not fill\n";
            return;
        }
        float fill_avg_price;
        float sum_q;
        getFillAverage(order_res,fill_avg_price,sum_q);

        new_trade.quantity_bought=sum_q;
        new_trade.price=fill_avg_price;
        
        active_trades.push_back(new_trade);
        
        std::cout<<"#Buy "<<"Symbol: "<<symbol<<" Buy_price: "<<new_trade.price<<" Quantity: "<<new_trade.quantity_bought<<" Time: "<<time(NULL)<<"\n";
        log<<"#Buy "<<"Symbol: "<<symbol<<" Buy_price: "<<new_trade.price<<" Quantity: "<<new_trade.quantity_bought<<" Time: "<<time(NULL)<<"\n";
    }
    void makeSellOrder(trade tr){
        std::stringstream ss;
        ss<<tr.quantity_bought;
        std::cout<<"Selling q: "<<ss.str()<<"\n";
        std::string order_res_str=bapi.placeOrder(symbol,"SELL","MARKET",ss.str());
        Json::Value order_res;

        if(!reader.parse(order_res_str,order_res)){
            std::cerr<<order_res<<"\n";
            std::cerr<<"Placed Order but Cannot Parse data\n";
        }
        if(order_res["status"]!="FILLED"){
            std::cerr<<order_res<<"\n";
            std::cout<<"Order did not fill\n";
            return;
        }
        float fill_avg_price;
        float sum_q=0;
        
        getFillAverage(order_res,fill_avg_price,sum_q);

        std::cout<<"#Sell "<<"Symbol: "<<symbol<<" Sell_price: "<<fill_avg_price<<" Quantity: "<<sum_q<<" Time: "<<time(NULL)<<"\n";
        log<<"#Sell "<<"Symbol: "<<symbol<<" Sell_price: "<<fill_avg_price<<" Quantity: "<<sum_q<<" Time: "<<time(NULL)<<"\n";
    }

    float getMA(){
        float ma=0;
        int range=0;
        for(int j=past_prices.size()-1; j>=past_prices.size()-average_minutes; j--){
            ma+=past_prices[j];
            range++;
        }
        return ma/(float)range;
    }

    float getTrend(price_t prc){
        float current_trend=1;
        //TODO
        // Test and implent better way for trend
        if(past_prices.size()>trend_minutes){
            current_trend=prc.ask/past_prices[past_prices.size()-trend_minutes];//Get short term trend min trend
        }else{
            current_trend=0;
        }
        return current_trend;
    }
    bool adequateDifferenceFromLastTrade(price_t prc){
        if(active_trades.size()>0)
            return (active_trades.back().price/prc.ask)>diff_from_last_trade;

        return true;
    }

    void checkAndExecute(){
        
        price_t prc=getPrice();
        float MA=getMA();
        float trend=getTrend(prc);

        
        if(MA/prc.ask>perc_diff_buy && trend>=1 && active_trades.size()<active_trades_limit && adequateDifferenceFromLastTrade(prc)){
            makeBuyOrder(prc);
        }else{
            for(int j=active_trades.size()-1; j>=0; j++){
                trade tr=active_trades[j];
                
                if(prc.bid/tr.price>=perc_diff_sell_gain || tr.price/prc.bid>=perc_diff_sell_loss){
                    makeSellOrder(tr);
                    active_trades.erase(active_trades.begin()+j);
                }        
            }
        }
        past_prices.push_back((prc.bid+prc.ask)/2);
        past_prices.pop_front();
        log<<"\n# "<<"Current Ask: "<<prc.ask<<" , Current bid: "<<prc.bid<<" , "<<"Trend: "<<trend<<" , "<<average_minutes<<"m Average: "<<MA<<" , "<<"Diff from MA: "<<MA/prc.ask<<" , "<<"Time: "<<time(NULL)<<"\n";
        #ifdef VERBOSE
            std::cout<<"\n# "<<"Current Ask: "<<prc.ask<<" Current bid: "<<prc.bid<<" , "<<"Trend: "<<trend<<" , "<<average_minutes<<"m Average: "<<MA<<" , "<<"Diff from MA: "<<MA/prc.ask<<" , "<<"Time: "<<time(NULL)<<"\n";
        #endif
        flushLog();
    }

    void flushLog(){
        log.flush();
    }

};

int main(int args, char* argv[]){

    if(argv[1]==NULL||argv[2]==NULL){
        std::cerr<<"Argument 0 and 1 must be a valid crypto\n";
        return 1;
    }
    pthread_t tid;

    pthread_create(&tid,NULL,exit_routine,NULL);
    
    virtTrader trader(argv[1],argv[2]);
    std::cout<<"Trading "<<trader.symbol<<"\n";
    trader.tradeLimit=20;
    trader.active_trades_limit=1;
    

    while(cont){
        trader.checkAndExecute();
        sleep(60);
    }

    pthread_join(tid,NULL);


    return 0;
}
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
#include "virtTrader.hpp"
volatile bool cont=1;

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
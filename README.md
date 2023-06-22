# BinanceTradingBot
This bot is a simple application that uses the Binance API to automate cryptocurrency trading. It can be used to continuously monitor and execute trades for a specific cryptocurrency pair on the Binance exchange.

## Dependencies
This application relies on several libraries:

- Binance API
- PThread
- Standard C++ Libraries

Create the executable by running ``` g++ -g main.cpp ./binanceApi/BAPI.cpp -o main.out -lcrypto -lcurl -lssl -ljsoncpp -lpthread ```

## Usage
To run the bot, you need to pass two arguments to the program at launch. These are the two cryptocurrencies that form the trading pair you want to work with. The first argument is the base cryptocurrency and the second is the quote cryptocurrency.

For example, if you want to trade Bitcoin against USDT, you can run:
``` ./CryptoTraderBot BTC USDT ```
This will start the bot, which will then begin monitoring and executing trades based on the logic defined in the virtTrader class.

The bot checks for potential trades every minute. You can adjust this by changing the sleep duration in the main loop.

This bot has a safe shutdown feature: Press 'q' at any time to signal the bot to stop trading. Please note that it may take up to a minute for all activities to cease and for the bot to shut down safely.

## Limitations
The bot currently has a limit of 20 trades and only 1 active trade can be held at a time. You can adjust these limitations by modifying tradeLimit and active_trades_limit variables in the main function.

## Disclaimer
This bot is intended for educational purposes only. Use at your own risk. Cryptocurrency trading carries financial risk and the creator of this bot is not responsible for any losses that may occur from its use.

## Contribute
Any contributions you make are greatly appreciated.

If you have any questions about the contributions, please feel free to contact us.

## License
Distributed under the MIT License. See LICENSE for more information.


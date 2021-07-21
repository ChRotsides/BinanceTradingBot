CC=g++
CPPFLAGS=
LDLIBS=-ljsoncpp -lpthread -lssl -lcrypto -lcurl

main.out: main.cpp BAPI.o virtTrader.o
	$(CC) $(CPPFLAGS) main.cpp BAPI.o virtTrader.o -o main.out $(LDLIBS)

virtTrader.o: virtTrader.cpp
	$(CC) -c $(CPPFLAGS) virtTrader.cpp -o virtTrader.o $(LDLIBS)

BAPI.o: ./binanceApi/BAPI.cpp
	$(CC) -c $(CPPFLAGS) ./binanceApi/BAPI.cpp -o BAPI.o $(LDLIBS)

clean:
	rm *.o *.out
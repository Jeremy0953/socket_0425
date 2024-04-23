all: serverM.cpp serverU.cpp serverD.cpp serverS.cpp client.cpp
	g++ -std=c++17 -o serverM serverM.cpp
	g++ -std=c++17 -o serverD serverD.cpp
	g++ -std=c++17 -o serverU serverU.cpp
	g++ -std=c++17 -o serverS serverS.cpp
	g++ -std=c++17 -o client client.cpp 

.PHONY : clean
clean :
	-rm serverD serverM serverS serverU client

.PHONY: client
client:
	./client

.PHONY: serverM
serverM:
	./serverM

.PHONY: serverS
serverS:
	./serverS

.PHONY: serverU
serverU:
	./serverU

.PHONY: serverD
serverD:
	./serverD
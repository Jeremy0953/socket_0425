all: serverM serverU serverD serverS client

serverM: serverM.cpp
	g++ -std=c++17 -o serverM serverM.cpp

serverD: serverD.cpp
	g++ -std=c++17 -o serverD serverD.cpp

serverU: serverU.cpp
	g++ -std=c++17 -o serverU serverU.cpp

serverS: serverS.cpp
	g++ -std=c++17 -o serverS serverS.cpp

client: client.cpp
	g++ -std=c++17 -o client client.cpp

extra: serverM_extra serverU_extra serverD_extra serverS_extra client_extra gen

serverM_extra: serverM.cpp
	g++ -std=c++17 -o serverM extraServerM.cpp -lssl -lcrypto

serverD_extra: serverD.cpp
	g++ -std=c++17 -o serverD serverD.cpp -lssl -lcrypto

serverU_extra: serverU.cpp
	g++ -std=c++17 -o serverU serverU.cpp -lssl -lcrypto

serverS_extra: serverS.cpp
	g++ -std=c++17 -o serverS serverS.cpp -lssl -lcrypto

client_extra: client.cpp
	g++ -std=c++17 -o client extraClient.cpp -lssl -lcrypto

gen: gen.cpp
	g++ -std=c++17 -o gen gen.cpp -lssl -lcrypto

.PHONY: clean
clean:
	-rm serverD serverM serverS serverU client

.PHONY: run_client
run_client:
	./client

.PHONY: run_serverM
run_serverM:
	./serverM

.PHONY: run_serverS
run_serverS:
	./serverS

.PHONY: run_serverU
run_serverU:
	./serverU

.PHONY: run_serverD
run_serverD:
	./serverD

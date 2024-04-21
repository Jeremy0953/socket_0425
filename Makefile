all: serverM.cpp serverH.cpp serverL.cpp serverS.cpp client.cpp
	g++ -std=c++11 -o serverM serverM.cpp
	g++ -std=c++11 -o serverL serverL.cpp
	g++ -std=c++11 -o serverH serverH.cpp
	g++ -std=c++11 -o serverS serverS.cpp
	g++ -std=c++11 -o client client.cpp 


.PHONY: client
client:
	./client

.PHONY: serverM
serverM:
	./serverM

.PHONY: serverS
serverC:
	./serverS

.PHONY: serverH
serverEE:
	./serverH

.PHONY: serverL
serverCS:
	./serverL
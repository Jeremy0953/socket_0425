#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <vector>
#include <iostream> 
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <unordered_map>
#define LOCAL_HOST "127.0.0.1" 
#define MAXDATASIZE 500
#define SERVERS "41153"
#define SERVERL "42153"
#define SERVERH "43153"
#define SERVERM_TCP "45153"
#define SERVERM "44153"
#define SERVERH_UINT 43153
#define SERVERL_UINT 42153
#define SERVERS_UINT 41153
#define BACKLOG 10 
using namespace std;

//TCP
int sockfd, new_fd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
struct sockaddr_storage their_addr;
char buf[MAXDATASIZE];
socklen_t addr_len;
char s[INET6_ADDRSTRLEN];
struct sigaction sa;
int yes = 1;
char clientMessage[MAXDATASIZE];
char res_from_c[MAXDATASIZE];
char info[MAXDATASIZE];

//UDP
struct addrinfo hints_U, *servinfo_U,*p_U;
struct addrinfo *serverCinfo,*serverCSinfo,*serverEEinfo;
int sockfd_U;
struct sockaddr_storage their_addr_U;
int rv_U;
socklen_t addr_len_U;
int numbytes_U;

bool isGuest = false;
bool isReserve = false;
unordered_map<string,uint16_t> route_table;
// From Beej’s guide to network programming
void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
// From Beej’s guide to network programming
void sigchld_handler(int s)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}
// From Beej’s guide to network programming
void createTCP(){

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(LOCAL_HOST, SERVERM_TCP, &hints, &servinfo)) != 0) {
		exit(1);
	}
   
    for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			continue;
		}
		break;
	}

    if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
}
// From Beej’s guide to network programming
void listenTCP(){
	if (listen(sockfd, BACKLOG) == -1) {
			exit(1);
		}

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		exit(1);
	}
}
// From Beej’s guide to network programming
void createUDP(){

	memset(&hints_U, 0, sizeof hints_U);
	hints_U.ai_family = AF_UNSPEC;
	hints_U.ai_socktype = SOCK_DGRAM;

	if ((rv_U = getaddrinfo(LOCAL_HOST, SERVERM, &hints_U, &servinfo_U)) != 0){
		exit(1);
	}
    for(p_U = servinfo_U; p_U != NULL; p_U = p_U->ai_next){
		if((sockfd_U = socket(p_U->ai_family,p_U->ai_socktype,p_U->ai_protocol))==-1){
			continue;
		}
		if(bind(sockfd_U, p_U->ai_addr, p_U->ai_addrlen) == -1){
			close(sockfd_U);
			continue;
		}
		break;
	}
    if(p_U==NULL){
		exit(1);
	}
	addr_len_U = sizeof their_addr_U;
} 
void receiveRouteTable() {
    int count = 0;
    while(count<3) {
        memset(buf,0,MAXDATASIZE);
        if((numbytes = recvfrom(sockfd_U, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
                exit(1);
            }
        cout << "The main server received the response from serverCS using UDP over port " << SERVERM << "." << endl;
        strncpy(info, buf, MAXDATASIZE);
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)&their_addr;
        uint16_t port = ntohs(ipv4->sin_port);
        if(buf[0] == '2'){
            count++;
            char backenCode;
            switch (port)
            {
            case SERVERH_UINT:
                backenCode = 'U';
                break;
            case SERVERL_UINT:
                backenCode= 'D';
                break;
            case SERVERS_UINT:
                backenCode = 'S';
                break;
            default:
                perror("port error");
                break;
            }
            cout<<"The main server has received the room status from Server "<<backenCode<<" using UDP over port "<<SERVERM<<".";
            continue;
        }
        route_table[string(buf)] = port; 
    }
}
// From Beej’s guide to network programming
void send_to_backen(const char* backen_port, string input){
    memset(&hints_U, 0, sizeof hints_U);
	hints_U.ai_family = AF_UNSPEC;
	hints_U.ai_socktype = SOCK_DGRAM;

    if ((rv_U = getaddrinfo(LOCAL_HOST, backen_port, &hints_U, &serverCinfo)) != 0){
		exit(1);
	}

	if ((numbytes_U = sendto(sockfd_U, input.c_str(), strlen(input.c_str())+1, 0, serverCinfo->ai_addr, serverCinfo->ai_addrlen)) == -1){
		exit(1);
	}

}

void loadmember(map<string,string> &maps){
    ifstream inputFile("member.txt");
    if (!inputFile.is_open()) {
        std::cerr << "Unable to open the file." << std::endl;
        perror("no member.txt");
    }
    string line;
    while (std::getline(inputFile, line)) {
        istringstream iss(line);
        string key, value;
        if (getline(iss, key, ',') && getline(iss, value, ',')) {
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        std::istringstream valueStream(value);
        if (valueStream >> value) {
            maps[key] = value;
        }
    }
    }
    std::cout<<"Main Server loaded the member list."<<endl;
    inputFile.close();
}

void load_list(string buf,map<string,int> &maps){
    std::vector<std::string> tokens;
    std::istringstream tokenStream(buf);
    std::string token;
    while (getline(tokenStream, token, ',')) {
        tokens.push_back(token);
    }
    for(int i = 0; i<tokens.size(); i = i+2) {
        maps[tokens[i]] = stoi(tokens[i+1]);
    }
}
void processUDP(int sockfd_u, map<string,int> science, map<string,int> literature, map<string,int> history){
    char buffer[MAXDATASIZE];
    sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);
    ssize_t bytesRead = recvfrom(sockfd_u, buffer, sizeof(buffer), 0,
                                 reinterpret_cast<sockaddr*>(&clientAddress), &clientAddrLen);
    string res;
    if (bytesRead == -1) {
        perror("recvfrom");
    } else {
        buffer[bytesRead] = '\0';
        res = string(buf);
        if(ntohs(clientAddress.sin_port)==atoi(SERVERS)){
            load_list(res,science);
            std::cout<<"Main Server received the book code list from serverS using UDP over port "<<SERVERM<<"."<<endl;
        }else if(ntohs(clientAddress.sin_port)==atoi(SERVERL)){
            load_list(res,literature);
            std::cout<<"Main Server received the book code list from serverL using UDP over port "<<SERVERM<<"."<<endl;
        }else if(ntohs(clientAddress.sin_port)==atoi(SERVERH)){
            load_list(res,history);
            std::cout<<"Main Server received the book code list from serverH using UDP over port "<<SERVERM<<"."<<endl;
        }else{
            perror("port error");
        }
    }
}

void handleClient(int clientSocket, map<string, string>& maps);

int main() {
    cout << "Main Server is up and running." << endl;
    createUDP();
    createTCP();
    listenTCP();
    map<string, string> maps;
    loadmember(maps);
    receiveRouteTable();

    while (true) { 
        addr_len = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_len);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) { // Child process
            close(sockfd); // 关闭监听socket，子进程不需要

            // 在子进程中处理客户端请求
            handleClient(new_fd, maps);
            exit(0); // 处理完成后，子进程退出
        } else if (pid > 0) { // Parent process
            close(new_fd); // 父进程关闭已接受的socket
        } else {
            perror("fork");
            close(new_fd);
        }
    }

    return 0; // 正常情况下不会执行到这里
}

void handleClient(int clientSocket, map<string, string>& userMap) {
    char buffer[MAXDATASIZE];
    string username, password;
    while (true) {

        // Step 1: Authenticate
        // Receive username
        if (recv(clientSocket, buffer, MAXDATASIZE, 0) > 0) {
            username = string(buffer);
        }

        // Clear buffer and receive password
        memset(buffer, 0, MAXDATASIZE);
        if (recv(clientSocket, buffer, MAXDATASIZE, 0) > 0) {
            password = string(buffer);
        }
        if (password.length() == 0) {
            isGuest = true;
        }
        if(isGuest){
            cout<<"The main server received the guest request for "<< username <<" using TCP over port "<<SERVERM_TCP<<". The main server accepts "<<username<<" as a guest."<<endl;
        }else{
            cout<<"The main server received the authentication for "<< username <<" using TCP over port "<<SERVERM_TCP<<"."<<endl;
        }
        // Authentication check
        auto it = userMap.find(username);
        if (it!=userMap.end()) {
            if(isGuest) {
                // Send authentication success
                strcpy(buffer, "3\0"); // Assuming '3' means guest
                send(clientSocket, buffer, strlen(buffer), 0);
                cout<<"The main server sent the guest response to the client."<<endl;
                break;
            }else {
                if (it->second == password) {
                    // Send authentication success
                    strcpy(buffer, "2\0"); // Assuming '2' means auth success
                    send(clientSocket, buffer, strlen(buffer), 0);
                    cout<<"The main server sent the authentication result to the client."<<endl;
                    break;
                } else {
                    // Send authentication failure
                    strcpy(buffer, "1\0"); // Assuming '1' means auth failure
                    send(clientSocket, buffer, strlen(buffer), 0);
                    cout<<"The main server sent the authentication result to the client."<<endl;
                }
            }
        } else {
            // Send authentication failure
            strcpy(buffer, "0\0"); // Assuming '0' means username not found
            send(clientSocket, buffer, strlen(buffer), 0);
            if(isGuest)
                cout<<"The main server sent the guest response to the client."<<endl;
            else
                cout<<"The main server sent the authentication result to the client."<<endl;
        }
        
    }
    // Step 2: Handle queries
    while (true) {
        memset(buffer, 0, MAXDATASIZE);
        int bytesReceived = recv(clientSocket, buffer, MAXDATASIZE, 0);
        if (bytesReceived <= 0) {
            break; // Break the loop if error or connection closed
        }
        if (buffer[0] == 'a') {
            isReserve = false;
        }
        if (buffer[0] == 'r') {
            isReserve = true;
        }
        if(isReserve&&isGuest) {
            memset(info, '\0', sizeof(info));
            info[0] = '3';
            cout<<username<<" cannot make a reservation."<<endl;
            if ((numbytes = send(clientSocket,info, strlen(info)+1, 0)) == -1) {
                            perror("send");
                            exit(1);
                        }
            cout<<"The main server sent the error message to the client."<<endl;
            continue;
        }
        string code = string(buffer);
        string bookcode = string(buffer+1);
        if(isReserve)
            cout<<"The main server has received the reservation request on Room "<<bookcode<<" from "<<username<<" using TCP over port "<<SERVERM_TCP<<"."<<endl;
        else
            cout<<"The main server has received the availability request on Room "<<bookcode<<" from "<<username<<" using TCP over port "<<SERVERM_TCP<<"."<<endl;
        memset(info, '\0', sizeof(info));
        auto it = route_table.find(bookcode);
        if (it != route_table.end()) {
            switch(it->second) {
                case SERVERH_UINT:
                    send_to_backen(SERVERH,code);
                    cout<<"The main server sent a request to Server U"<<endl;
                    break;
                case SERVERL_UINT:
                    send_to_backen(SERVERL, code);
                    cout<<"The main server sent a request to Server D"<<endl;
                    break;
                case SERVERS_UINT:
                    send_to_backen(SERVERS, code);
                    cout<<"The main server sent a request to Server S"<<endl;
                    break;
                default:
                    break;
            }
        } else {
            if (code.at(1) == 'S'){
            send_to_backen(SERVERS,code);
            cout<<"The main server sent a request to Server S"<<endl;
            }else if (code.at(1) == 'U'){
                send_to_backen(SERVERH,code);
                cout<<"The main server sent a request to Server U"<<endl;
            }else if (code.at(1) == 'D'){
                send_to_backen(SERVERL,code);
                cout<<"The main server sent a request to Server D"<<endl;
            }else{
                cout<<"Did not find "<<buffer+1<<" in the book code list."<<endl;
                memset(info, '\0', MAXDATASIZE);
                        info[1] = '2';
                        if ((numbytes = send(clientSocket,info, strlen(info)+1, 0)) == -1) {
                            perror("send");
                            exit(1);
                        }
                continue;
            }
        }
        memset(buf,0,MAXDATASIZE);
        if((numbytes = recvfrom(sockfd_U, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
                exit(1);
            }
        cout << "The main server received the response from serverCS using UDP over port " 
        << SERVERM << "." << endl;
        strncpy(info, buf, MAXDATASIZE);
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)&their_addr;
        uint16_t port = ntohs(ipv4->sin_port);
        char backenCode;
        switch (port)
        {
        case SERVERH_UINT:
            backenCode = 'U';
            break;
        case SERVERL_UINT:
            backenCode = 'D';
            break;
        case SERVERS_UINT:
            backenCode = 'S';
            break;
        default:
            perror("port error");
            break;
        }
        if(info[1] == 'r'){
            if(info[0]=='0'){
                cout<<"The main server received the response and the updated room status from Server "<<backenCode<<" using UDP over port "<<SERVERM<<"."<<endl;
                cout<<"The room status of Room "<<bookcode<<" has been updated."<<endl;
            }else if(info[0]=='1'){
                cout<<"The main server received the response from Server "<<backenCode<<" using UDP over port "<<SERVERM<<"."<<endl;
            }else if(info[0]=='2'){
                cout<<"The main server received the response from Server "<<backenCode<<" using UDP over port "<<SERVERM<<"."<<endl;
            }else{
                cout<<"info"<<info<<endl;
                perror("error udp message");
            }
        }
        if(info[1] == 'a'){
            cout<<"The main server received the response from Server "<<backenCode<<" using UDP over port "<<SERVERM<<"."<<endl;
        }
        
        if ((numbytes = send(new_fd,info, MAXDATASIZE, 0)) == -1) {
            perror("send");
            exit(1);
        }
        if(info[1]=='r'){
            cout<<"The main server sent the reservation result to the client."<<endl;
        }
        if(info[1] == 'a'){
            cout<<"The main server sent the availability information to the client."<<endl;
        }
        cout << "Main Server sent the book status to the client." << endl;
    }

    close(clientSocket);
}


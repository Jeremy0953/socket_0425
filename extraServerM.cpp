// C Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <cctype>

// C++ Standard Library
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

// System/Network headers
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

constexpr int MAXDATASIZE = 500;
constexpr int BACKLOG = 10;
constexpr int SERVERU_UINT = 43326;
constexpr int SERVERD_UINT = 42326;
constexpr int SERVERS_UINT = 41326;

#define LOCAL_HOST  "127.0.0.1"
#define SERVERS  "41326"
#define SERVERD  "42326"
#define SERVERU  "43326"
#define SERVERM_TCP  "45326"
#define SERVERM  "44326"
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

enum encrypt_type {ORI,MD5,SHA256};
encrypt_type ENCRYP_TYPE;
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
        if((numbytes = recvfrom(sockfd_U, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len_U))==-1){
                exit(1);
            }
        strncpy(info, buf, MAXDATASIZE);
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)&their_addr;
        uint16_t port = ntohs(ipv4->sin_port);
        if(buf[0] == '2'){
            count++;
            char backenCode;
            switch (port)
            {
            case SERVERU_UINT:
                backenCode = 'U';
                break;
            case SERVERD_UINT:
                backenCode= 'D';
                break;
            case SERVERS_UINT:
                backenCode = 'S';
                break;
            default:
                perror("port error");
                break;
            }
            cout<<"The main server has received the room status from Server "<<backenCode<<" using UDP over port "<<SERVERM<<"."<<endl;
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

void loadmember(std::map<std::string, std::string>& maps) {
    std::string inputFilePath;
    switch(ENCRYP_TYPE){
        case ORI:
            inputFilePath = "member.txt";
            break;
        case SHA256:
            inputFilePath = "member_sha256.txt";
            break;
        case MD5:
            inputFilePath = "member_md5.txt";
            break;
        default:
            break;
    }
    std::ifstream inputFile(inputFilePath);
    if (!inputFile) {
        std::cerr << "Unable to open the file 'member.txt'.\n";
        return; // Return early if file cannot be opened
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, ',') && std::getline(iss, value, ',')) {
            // Trim leading and trailing whitespace using a lambda and std::find_if
            auto trim = [](const std::string &s) {
                auto wsfront = std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
                auto wsback = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base();
                return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
            };

            maps[trim(key)] = trim(value);
        }
    }
    //std::cout << "Main Server loaded the member list.\n";
}


void handleClient(int clientSocket, map<string, string>& maps);

int main(int argc, char*argv[]) {
    if(argc == 1) {
        printf("no arguments, use character shifting protocol\n"); 
        ENCRYP_TYPE = ORI;
    } else if( argc == 2 ) {
        printf("Using encryption protocol %s.\n", argv[1]); 
        if(strcmp(argv[1],"sha256")==0){
            ENCRYP_TYPE = SHA256;
        }else if(strcmp(argv[1],"md5")==0){
            ENCRYP_TYPE = MD5;
        }else{
            cout<<"wrong arguments,please exec with ./serverM md5 or ./serverM sha256 "<<endl;
            exit(-1);
        }
    }else {
        cout<<"wrong arguments,please exec with ./serverM md5 or ./serverM sha256 or ./serverM "<<endl;
        exit(-1);
    }
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
        cout<<"username:"<<username<<endl;
        cout<<"passwd" << password <<endl;
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
                case SERVERU_UINT:
                    send_to_backen(SERVERU,code);
                    cout<<"The main server sent a request to Server U"<<endl;
                    break;
                case SERVERD_UINT:
                    send_to_backen(SERVERD, code);
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
                send_to_backen(SERVERU,code);
                cout<<"The main server sent a request to Server U"<<endl;
            }else if (code.at(1) == 'D'){
                send_to_backen(SERVERD,code);
                cout<<"The main server sent a request to Server D"<<endl;
            }else{
                //cout<<"Did not find "<<buffer+1<<" in the book code list."<<endl;
                memset(info, '\0', MAXDATASIZE);
                        info[0] = '2';
                        info[1] = buffer[0];
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
        strncpy(info, buf, MAXDATASIZE);
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)&their_addr;
        uint16_t port = ntohs(ipv4->sin_port);
        char backenCode;
        switch (port)
        {
        case SERVERU_UINT:
            backenCode = 'U';
            break;
        case SERVERD_UINT:
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
    }

    close(clientSocket);
}


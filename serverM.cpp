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


int main(){
	std::cout << "Main Server is up and running." << endl;
    createUDP();
    createTCP();
    listenTCP();
    map<string,string> maps;
    loadmember(maps);
    string password, username, code;// all the info server received
    // Do the authentication
    while (true) { 
        //connect tcp
        memset(clientMessage,'\0',sizeof(clientMessage));
        addr_len = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_len);
        if (new_fd == -1) {
            perror("accept");
        }
        inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr*)&their_addr),s, sizeof s);
        if (recv(new_fd, clientMessage,MAXDATASIZE, 0) == -1)
					perror("recv");
		username = clientMessage;
        memset(clientMessage,'\0',sizeof(clientMessage));
        if (recv(new_fd, clientMessage,MAXDATASIZE, 0) == -1)
					perror("recv");
		password = clientMessage;
        std::cout<<"Main Server received the username and password from the client using TCP over port "<<SERVERM_TCP<<"."<<endl;
        auto it = maps.find(username);
            if (it != maps.end()) {
                //std::cout << "Lengths: " << it->second.length() << ", " << password.length() << std::endl;
                if (it->second == password)
                {
                    std::cout<<"Password "<<password<<" matches the username. Send a reply to the client."<<endl;
                    memset(res_from_c, '\0', MAXDATASIZE);
                    res_from_c[0] = '2';
                    if ((numbytes = send(new_fd,res_from_c,  strlen(res_from_c)+1, 0)) == -1) {
                        perror("send");
                        exit(1);
                    }
                    close(new_fd);
                    break;
                }else{
                    std::cout<<"Password "<<password<<" does not matches the username. Send a reply to the client."<<endl;
                    memset(res_from_c, '\0', MAXDATASIZE);
                    res_from_c[0] = '1';
                    if ((numbytes = send(new_fd,res_from_c, strlen(res_from_c)+1, 0)) == -1) {
                        perror("send");
                        exit(1);
                    }
                    close(new_fd);
                }
            } else {
                std::cout<<username<<" is not registered. Send a reply to the client"<<endl;
                memset(res_from_c, '\0', MAXDATASIZE);
                    res_from_c[0] = '0';
                    if ((numbytes = send(new_fd,res_from_c, strlen(res_from_c)+1, 0)) == -1) {
                        perror("send");
                        exit(1);
                    }
                    close(new_fd);
            }       
    }
    // Do the query
    while (true) { 
        //connect tcp
        memset(clientMessage,'\0',sizeof(clientMessage));
        addr_len = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_len);
        if (new_fd == -1) {
            perror("accept");
        }
        inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr*)&their_addr),s, sizeof s);
        if (recv(new_fd, clientMessage,MAXDATASIZE, 0) == -1)
					perror("recv");
		code = clientMessage;
        
        std::cout<<"Main Server received the book request from client using TCP over port "<<SERVERM_TCP<<"."<<endl;
        memset(info, '\0', sizeof(info));
        if (code.substr(0, 1) == "S"){
            send_to_backen(SERVERS,code);
            std::cout<<"Found "<<code<<" located at Server S. Send to Server S."<<endl;
        }else if (code.substr(0, 1) == "H"){
            send_to_backen(SERVERH,code);
            std::cout<<"Found "<<code<<" located at Server H. Send to Server H."<<endl;
        }else if (code.substr(0, 1) == "L"){
            send_to_backen(SERVERL,code);
            std::cout<<"Found "<<code<<" located at Server L. Send to Server L."<<endl;
        }else{
            cout<<"Did not find "<<code<<" in the book code list."<<endl;
            memset(info, '\0', MAXDATASIZE);
                    info[0] = '2';
                    if ((numbytes = send(new_fd,info, strlen(info)+1, 0)) == -1) {
                        perror("send");
                        exit(1);
                    }
            close(new_fd);
            continue;
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
        switch (port)
        {
        case SERVERH_UINT:
            cout<<"Main Server received from server H the book status result using UDP over port "<<SERVERM<<':'<<endl;
            break;
        case SERVERL_UINT:
            cout<<"Main Server received from server L the book status result using UDP over port "<<SERVERM<<':'<<endl;
            break;
        case SERVERS_UINT:
            cout<<"Main Server received from server S the book status result using UDP over port "<<SERVERM<<':'<<endl;
            break;
        default:
            perror("port error");
            break;
        }
        if(info[0]=='0'){
            cout<<"book "<<code<<" is available"<<endl;
        }else if(info[0]=='1'){
            cout<<"book "<<code<<" is not available"<<endl;
        }else if(info[0]=='2'){
            cout<<"Did not find "<<code<<" in the book code list."<<endl;
        }else{
            perror("error udp message");
        }
        if ((numbytes = send(new_fd,info, sizeof(info)+1, 0)) == -1) {
            perror("send");
            exit(1);
        }
        cout << "Main Server sent the book status to the client." << endl;
        close(new_fd);
    }      
}
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
#include <map>
#include <sstream>

#define LOCAL_HOST "127.0.0.1" //define local host name
#define MAXDATASIZE 500
#define SERVERS "41153"
#define SERVERM "44153"
#define file_path "science.txt"
using namespace std;

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
struct sockaddr_storage their_addr;
char buf[MAXDATASIZE];
socklen_t addr_len;
char s[INET6_ADDRSTRLEN];

// From Beej’s guide to network programming
void createUDP(){
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVERS, &hints, &servinfo)) != 0){
		exit(1);
	}	
    for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
			continue;
		}
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			continue;
		}
		break;
	}
    if(p==NULL){
		exit(1);
	}
	
}

void loaddata(map<string,int> &books){
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "can't open file" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string key;
    char comma;
    int value;
    if (std::getline(iss, key, ',') && iss >> value) {
        books[key] = value;
    } else {
        std::cerr << "line error " << line << std::endl;
    }
}

    file.close();
    return;
}

void sendBookStatus(map<string,int> &books){
    cout<<"begin send."<<endl;
    for(const auto & it : books){
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        memset(buf,'0',MAXDATASIZE);
        strncpy(buf, it.first.c_str(), MAXDATASIZE);
        if ((rv = getaddrinfo(LOCAL_HOST, SERVERM, &hints, &servinfo)) != 0){
            exit(1);
        }

        if ((numbytes = sendto(sockfd, buf, strlen(buf)+1, 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1){
            exit(1);
        }
    }
    cout<<"send end"<<endl;
    memset(buf,'0',MAXDATASIZE);
    buf[0] = '2'; //finished;
    if ((numbytes = sendto(sockfd, buf, strlen(buf)+1, 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1){
            exit(1);
    }
    cout<<"The Server <S/D/U> has sent the room status to the main server."<<endl;
}
int main(){
    createUDP();
    cout << "Server S is up and running using UDP on port " << SERVERS <<"." << endl;
    map<string,int> books;
    string bookcode;
    loaddata(books);
    addr_len = sizeof their_addr;
    sendBookStatus(books);
    while(true){
        if((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
            exit(1);
        }
        bool isReserve = false;
        if (buf[0] == 'r'){
            isReserve = true;
        }
        if (buf[0] == 'a'){
            isReserve = false;
        }
        bookcode = string(buf+1);
        
        cout << "Server S received "<<bookcode<<" code from the Main Server." << endl;
        string check;
        auto it = books.find(bookcode);
        if (it != books.end()) {
            if(it->second>0){
                if(isReserve)
                    it->second--;
                check = "0";
            }else{
                check = "1";
            }
        } else {
            check = "2";
        } 
        if ((numbytes = sendto(sockfd, check.c_str(), strlen(check.c_str())+1, 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
            exit(1);
        }
        cout<<"Server S finished sending the availability status of code "<<bookcode<<" to the Main Server using UDP on port "<<SERVERS<<'.'<<endl;
    }
	freeaddrinfo(servinfo);
	close(sockfd);
}
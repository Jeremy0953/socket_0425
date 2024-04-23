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
#include <algorithm>

#define LOCAL_HOST "127.0.0.1" //define local host name
#define MAXDATASIZE 500
#define SERVERU "43326"
#define SERVERM "44326"
#define file_path "suite.txt"
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
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVERU, &hints, &servinfo)) != 0){
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

void loaddata(std::map<std::string, int>& books) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "can't open file" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string key;
        int value;
        std::replace(line.begin(), line.end(), ',', ' ');  // Replace comma with space for easier parsing
        std::istringstream iss(line);
        
        if (iss >> key >> value) { // Parse the key and value directly
            books[key] = value;
        } else {
            std::cerr << "line error " << line << std::endl;
        }
    }

    file.close();
}
void sendBookStatus(map<string,int> &books){
    //cout<<"begin send."<<endl;
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
    //cout<<"send end"<<endl;
    memset(buf,'0',MAXDATASIZE);
    buf[0] = '2'; //finished;
    if ((numbytes = sendto(sockfd, buf, strlen(buf)+1, 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1){
            exit(1);
    }
    cout<<"The Server U has sent the room status to the main server."<<endl;
}
int main(){
    createUDP();
    cout << "Server U is up and running using UDP on port " << SERVERU <<"." << endl;
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
        if(isReserve){
            cout<<"The Server U received an availability request from the main server."<<endl;
        }else {
            cout<<"The Server U received a reservation request from the main server."<<endl;
        }
        memset(buf,'0',MAXDATASIZE);
        auto it = books.find(bookcode);
        bool refresh = false;
        if (it != books.end()) {
            if(it->second>0){
                if(isReserve){
                    it->second--;
                    refresh = true;
                    cout<<"Successful reservation. The count of Room "<<bookcode<<" is now "<<it->second<<"."<<endl;
                }else{
                    cout<<"Room "<<bookcode<<" is available."<<endl;
                }
                buf[0] = '0';
            }else{
                if(isReserve){
                    cout<<"Cannot make a reservation. Room "<<bookcode<<" is not available."<<endl;
                }else{
                    cout<<"Room "<<bookcode<<" is not available."<<endl;
                }
                buf[0] = '1';
            }
        } else {
            if(isReserve){
                cout<<"Cannot make a reservation. Not able to find the room layout."<<endl;
            }else{
                cout<<"Not able to find the room layout."<<endl;
            }
            buf[0] = '2';
        } 
        if (isReserve)
            buf[1] = 'r';
        else
            buf[1] = 'a';
        if ((numbytes = sendto(sockfd, buf, 3, 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
            exit(1);
        }
        if(isReserve){
            if(refresh)
                cout<<"The Server U finished sending the response and the updated room status to the main server."<<endl;
            else
                cout<<"The Server U finished sending the response to the main server."<<endl;
        }else{
            cout<<"The Server U finished sending the response to the main server."<<endl;
        }
    }
	freeaddrinfo(servinfo);
	close(sockfd);
}
// C Standard Library headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <cctype>

// C++ Standard Library headers
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

// System/Network headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

constexpr int MAXDATASIZE = 500;

#define LOCAL_HOST  "127.0.0.1" // Define local host name
#define SERVERD  "42326"
#define SERVERM  "44326"
#define file_path  "double.txt" // Use uppercase for constants by convention

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
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVERD, &hints, &servinfo)) != 0){
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

void loaddata(std::map<std::string, int>& rooms) {
    std::ifstream file(file_path);
    if (!file) {
        std::cerr << "Cannot open file " << file_path << std::endl;
        return; // Early return if the file cannot be opened
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string key;
        int value;

        // Remove leading and trailing spaces
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Replace commas with spaces for easier parsing
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream iss(line);
        if (iss >> key >> value) { // Parse the key and value directly
            rooms[key] = value;
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }

    //std::cout << "Data loaded successfully." << std::endl;
    file.close(); // Ensure file is closed
}

void sendroomstatus(map<string,int> &rooms){
    //cout<<"begin send."<<endl;
    for(const auto & it : rooms){
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
    cout<<"The Server D has sent the room status to the main server."<<endl;
}
int main(){
    createUDP();
    cout << "Server D is up and running using UDP on port " << SERVERD <<"." << endl;
    map<string,int> rooms;
    string roomcode;
    loaddata(rooms);
    addr_len = sizeof their_addr;
    sendroomstatus(rooms);
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
        roomcode = string(buf+1);
        if(!isReserve){
            cout<<"The Server D received an availability request from the main server."<<endl;
        }else {
            cout<<"The Server D received a reservation request from the main server."<<endl;
        }
        memset(buf,'0',MAXDATASIZE);
        auto it = rooms.find(roomcode);
        bool refresh = false;
        if (it != rooms.end()) {
            if(it->second>0){
                if(isReserve){
                    it->second--;
                    refresh = true;
                    cout<<"Successful reservation. The count of Room "<<roomcode<<" is now "<<it->second<<"."<<endl;
                }else{
                    cout<<"Room "<<roomcode<<" is available."<<endl;
                }
                buf[0] = '0';
            }else{
                if(isReserve){
                    cout<<"Cannot make a reservation. Room "<<roomcode<<" is not available."<<endl;
                }else{
                    cout<<"Room "<<roomcode<<" is not available."<<endl;
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
        if(isReserve){
            buf[1] = 'r';
        }else{
            buf[1] = 'a';
        }
        if ((numbytes = sendto(sockfd, buf, 3, 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
            exit(1);
        }
        if(isReserve){
            if(refresh)
                cout<<"The Server D finished sending the response and the updated room status to the main server."<<endl;
            else
                cout<<"The Server D finished sending the response to the main server."<<endl;
        }else{
            cout<<"The Server D finished sending the response to the main server."<<endl;
        }
    }
	freeaddrinfo(servinfo);
	close(sockfd);
}
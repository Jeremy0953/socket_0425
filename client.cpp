#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <typeinfo>
#define LOCAL_HOST "127.0.0.1"
#define PORT "45153" 

#define MAXDATASIZE 500 // max data size of bytes we can hold one time

unsigned int clientPort;
#include <iostream>
using namespace std;
// From Beej’s guide to network programming
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string encrypt(string unencrypted){
    for (int i = 0; i<unencrypted.length(); i++){
        if ('0' <= unencrypted[i] &&  unencrypted[i] <= '9'){
            unencrypted[i] = (unencrypted[i] - '0' + 5) % 10 + '0';
        }
        if ('a' <= unencrypted[i] &&  unencrypted[i] <= 'z'){
            unencrypted[i] = (unencrypted[i] - 'a' + 5) % 26 + 'a';
        }
        if ('A' <= unencrypted[i] &&  unencrypted[i] <= 'Z'){
            unencrypted[i] = (unencrypted[i] - 'A' + 5) % 26 + 'A';
        }
    }
    return unencrypted;
}

int main(int argc, char *argv[])
{
    // Get info
    cout << "Client is up and running." << endl;
    string username;
    string password;
    bool isGuest = false;
    int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    char clientInput[MAXDATASIZE];
    
    int cnt = 0;//for counting how many times entering the wrong username/password
    string res;
    // start for the user to input username and password
    while (true){
        // From Beej’s guide to network programming
        //set the TCP connect:
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
            return 1;
        }

        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                continue;
            }
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                continue;
            }

            break;
        }

        if (p == NULL) {
            return 2;
        }
        // From Experiment instructions, to get a dynamic port
        struct sockaddr_in clinetAddress;
        bzero(&clinetAddress, sizeof(clinetAddress));
        socklen_t len = sizeof(clinetAddress);
        if(getsockname(sockfd, (struct sockaddr *) &clinetAddress, &len)==-1){
            perror("getsockname");
            exit(1);
        }
        clientPort = ntohs(clinetAddress.sin_port);

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)&clinetAddress),
                s, sizeof s);
        freeaddrinfo(servinfo);


        cout << "Please enter the username: ";
        getline(cin, username);
        memset(clientInput, '\0', sizeof(clientInput));
        strncpy(clientInput, encrypt(username).c_str(), MAXDATASIZE);
        if ((numbytes = send(sockfd,clientInput, sizeof(clientInput), 0)) == -1) {
            exit(1);
        }
        cout<<"Please enter the password: (Press “Enter” to skip)"<<endl;
        getline(cin, password);
        if (password.length() == 0){
            isGuest = true;
        }
        memset(clientInput, '\0', sizeof(clientInput));
        strncpy(clientInput, encrypt(password).c_str(), MAXDATASIZE);
        if ((numbytes = send(sockfd, clientInput, sizeof(clientInput), 0)) == -1) {
            exit(1);
        }
        cout << username << " sent an authentication request to the main server." <<endl;

        if (recv(sockfd, clientInput,MAXDATASIZE, 0) == -1)
            perror("recv");
        res = clientInput;
        
        // checkout the result
        if (clientInput[0] == '0'){
            cout<<username<<" received the result of authentication from Main Server using TCP over port"<<clientPort<<". Authentication failed: Username not found."<<endl;
        }
        else if (clientInput[0] == '1'){
            cout<<username<<" received the result of authentication from Main Server using TCP over port"<<clientPort<<". Authentication failed: Password does not match."<<endl;
        }
        else if (clientInput[0] == '2'){
            cout<<username<<" received the result of authentication from Main Server using TCP over port "<<clientPort<<". Authentication is successful."<<endl;
            break;
        }else if (clientInput[0] == '3'){
            break;
        }
    }

    string query_reply;
    // start the query
    while (true){
        string bookcode, category;
        bool isReserve = false;
        string command;
        cout<<"Would you like to search for the availability or make a reservation? (Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation ): <Availability or Reservation>"<<endl;
        getline(cin, command);
        isReserve = command == "Reservation";
        
        cout << "Please enter book code to query: ";
        getline(cin, bookcode);
        
        // send bookcode to serverM
        memset(clientInput, '\0', sizeof(clientInput));
        if (isReserve){
            clientInput[0] = 'r';
        }
        else{
            clientInput[0] = 'a';
        }
        
        strncpy(clientInput+1, bookcode.c_str(), MAXDATASIZE);
        if ((numbytes = send(sockfd,clientInput, sizeof(clientInput), 0)) == -1) {
            exit(1);
        }
        cout<<username<<" sent the request to the Main Server."<<endl;
        memset(buf, '\0', MAXDATASIZE);
        if (recv(sockfd, buf,MAXDATASIZE, 0) == -1)
            perror("recv");
        cout<<"Response received from the Main Server on TCP port: "<<clientPort<<"."<<endl;
        
        if(buf[0] == '0'){
            cout<<"The requested book "<<bookcode<<" is available in the library."<<endl;
        }else if(buf[0] == '1'){
            cout<<"The requested book "<<bookcode<<" is NOT available in the library."<<endl;
        }else if(buf[0] == '2'){
            cout<<"Not able to find the book-code "<<bookcode<<" in the system."<<endl;
        }else{
            cout<<"buf"<<buf<<endl;
            perror("response Mserver");
        }
        cout << endl;
        cout << "-----Start a new request-----" <<endl;
    }
    close(sockfd);
}
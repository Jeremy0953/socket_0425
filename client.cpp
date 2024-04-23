#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <typeinfo>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

constexpr int MAXDATASIZE = 500;

#define LOCAL_HOST  "127.0.0.1" // Define local host name
#define PORT  "45326"

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
            unencrypted[i] = (unencrypted[i] - '0' + 3) % 10 + '0';
        }
        if ('a' <= unencrypted[i] &&  unencrypted[i] <= 'z'){
            unencrypted[i] = (unencrypted[i] - 'a' + 3) % 26 + 'a';
        }
        if ('A' <= unencrypted[i] &&  unencrypted[i] <= 'Z'){
            unencrypted[i] = (unencrypted[i] - 'A' + 3) % 26 + 'A';
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
        if (isGuest)
            cout << username << " sent an guest request to the main server." <<endl;
        else
            cout << username << " sent an authentication request to the main server." <<endl;

        if (recv(sockfd, clientInput,MAXDATASIZE, 0) == -1)
            perror("recv");
        res = clientInput;
        
        // checkout the result
        if (clientInput[0] == '0'){
            cout<<"Failed login: Username does not exist."<<endl;
        }
        else if (clientInput[0] == '1'){
            cout<<"Failed login: Password does not match."<<endl;
        }
        else if (clientInput[0] == '2'){
            break;
        }else if (clientInput[0] == '3'){
            break;
        }
    }
    if(isGuest)
        cout<<"Welcome guest "<<username<<"!"<<endl;
    else
        cout<<"Welcome member "<<username<<"!"<<endl;
    string query_reply;
    // start the query
    while (true){
        string bookcode, category;
        bool isReserve = false;
        cout << "Please enter the room code: ";
        getline(cin, bookcode);
        string command;
        while(1){
            cout<<"Would you like to search for the availability or make a reservation? (Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation ): ";
            getline(cin, command);
            if(command=="Reservation" || command == "Availability")
                break;
        }
        isReserve = command == "Reservation";
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
        if(isGuest){
            if(isReserve){
                cout<<username<<" sent a reservation request to the main server."<<endl;
            }else{
                cout<<username<<" sent an availability request to the main server."<<endl;
            }
        }else{
            if(isReserve) {
                cout<<username<<" sent a reservation request to the main server."<<endl;
            }else {
                cout<<username<<" sent an availability request to the main server."<<endl;
            }
        }
        memset(buf, '\0', MAXDATASIZE);
        if (recv(sockfd, buf,MAXDATASIZE, 0) == -1)
            perror("recv");
        if(isGuest){
            switch(buf[0]){
                case '0':
                    cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                    cout<<"The requested room is available."<<endl;
                    break;
                case '1':
                    cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                    cout<<"The requested room is not available."<<endl;
                    break;
                case '2':
                    cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                    cout<<"Not able to find the room layout."<<endl;
                    break;
                case '3':
                    cout<<"Permission denied: Guest cannot make a reservation."<<endl;
                    break;
                default:
                    cout<<buf<<endl;
                    cout<<"not defined."<<endl;
                    perror("response Mserver");
                    break;
            }
    
        }else{
            switch (buf[0]){
                case '0':
                    if(buf[1]=='a'){
                        cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                        cout<<"The requested room is available."<<endl;
                    }else {
                        cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                        cout<<"Congratulation! The reservation for Room "<<bookcode<<" has been made."<<endl;
                    }
                    break;
                case '1':
                    if(buf[1]=='a'){
                        cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                        cout<<"The requested room is not available."<<endl;
                    }else {
                        cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                        cout<<"Sorry! The requested room is not available."<<endl;
                    }
                    break;
                case '2':
                    if(buf[1]=='a'){
                        cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                        cout<<"Not able to find the room layout."<<endl;
                    }else {
                        cout<<"The client received the response from the main server using TCP over port "<<PORT<<"."<<endl;
                        cout<<"Oops! Not able to find the room."<<endl;
                    }
                    break;
                default:
                    cout<<buf<<endl;
                    cout<<"not defined."<<endl;
                    perror("response Mserver");
                    break;
            }
        }

        cout << endl;
        cout << "-----Start a new request-----" <<endl;
    }
    close(sockfd);
}
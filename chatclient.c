/*
Author: Joshua Kluthe
Description:
This program implements a simple TCP client that connects to a server and allows the
client and server to chat.
*/

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
#include <sys/ioctl.h>

#define MAX_DATA 1024

/*
set_info_structs() takes addrinfo structs for hints and the server and sets their attributes
appropriately. It also requires the host and port as parameters in order to identify the server
address to get information from.
It returns 1 on failure, 0 on success.
*/
int set_info_structs(struct addrinfo *hints, struct addrinfo **serverinfo, char* host, char* port) {
    
    int rv;
    memset(hints, 0, sizeof(*hints));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    
    if((rv = getaddrinfo(host, port, hints, serverinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    return 0;
}

/*
connect_to_server() loops through the possible addresses in paramater *serverinfo until it
finds a null or succeeds in making a connection. On successful connection it returns the 
socket file descriptor, otherwise it calls exit(1).
*/
int connect_to_server(struct addrinfo *serverinfo) {
    
    struct addrinfo *p;
    int sockfd;
    
    //loop through possible addresses in serverinfo
    for(p = serverinfo; p != NULL; p = p->ai_next) {
        //attempt to create socket
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        //socket created, attempt to connect
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        //here the socket and connection are good, so break loop
        break;
    }
    //loop exits with either a connection made, or p == NULL. Check p
    if(p == NULL) {
        fprintf(stderr, "client: failed to connect to server\n");
        exit(1);
    }
    printf("Connection successful.\n");
    freeaddrinfo(serverinfo);
    return sockfd;
}

/*
write_from_conn() takes a socket file descriptor as a parameter.
It the receives a message from the socket, and writes it to the terminal in red, right-aligned.
If the message recieved is "\quit" it returns 0, else it returns 1.
*/
int write_from_conn(int sockfd) {
    //example for getting terminal width came from http://stackoverflow.com/questions/1022957/getting-terminal-width-in-c
    char buff[MAX_DATA];
    struct winsize w;
    int end;
    int i;
    
    ioctl(0, TIOCGWINSZ, &w);
    
    end = recv(sockfd, buff, MAX_DATA - 1, 0);
    buff[end] = '\0';
    
    printf("\e[0;31m%*s", w.ws_col, buff);
    
    //find the end of the username in the string, and see if they sent "\quit"
    i = 0;
    while(*(buff + i) != '>') {
        i++;
    }
    if(strcmp(buff + i + 2, "\\quit\n") == 0)
        return 0;
    else
        return 1;
}

/*
write_to_conn() takes a socket file descriptor and string chat_name as parameters.
It gets an input from the user and constructs and sends a message to the server.
If the input message is "\quit" it returns 0, else it returns 1.
*/
int write_to_conn(int sockfd, char *chat_name) {
    
    char in_message[MAX_DATA - 11];
    char out_message[MAX_DATA];
    int retval = 1;
    
    printf("\e[0;32m%s> ", chat_name);
    //get input message from user
    fgets(in_message, sizeof(in_message) - 1, stdin);
    //add a null if necessary
    if(in_message[strlen(in_message) - 1] == '\n')
        in_message[strlen(in_message) - 1] = '\0';
    //check if the user input "\quit" and set retval to false if so
    if(strcmp(in_message, "\\quit") == 0)
        retval = 0;
    
    //construct and send the message
    strcpy(out_message, chat_name);
    strcat(out_message, "> ");
    strcat(out_message, in_message);
    send(sockfd, out_message, strlen(out_message), 0);
    
    return retval;
}

/*
chat() takes a socket file descriptor as a parameter. It gets the desired chat username
from the user, and then chats with the server until one of the users enters "\quit"
*/
void chat(int sockfd) {
    char chat_name[11];
    
    printf("Initiating chat.\nInput your name: ");
    
    //get the name the user wants to use in chat
    fgets(chat_name, sizeof(chat_name) - 1, stdin);
    if(chat_name[strlen(chat_name) - 1] == '\n')
        chat_name[strlen(chat_name) - 1] = '\0';
    
    while(1) {
        
        if(!write_to_conn(sockfd, chat_name)) {
            printf("\e[0;37mClosing connection.\n");
            close(sockfd);
			break;
        }
        
        if(!write_from_conn(sockfd)) {
            printf("\e[0;37mServer closed connection.\n");
            close(sockfd);
			break;
        }
    }
}

/*
main() takes a server hostname and a port as args, and then creates a connection with the
server. It then initiates a chat with the server.
*/
int main(int argc, char *argv[]) {
    
    struct addrinfo hints, *serverinfo;
    int sockfd;
    
    if(argc < 2) {
        printf("ERROR: Must provide host and port, format 'chatclient HOST PORT'");
        exit(1);
    }
    char *host = argv[1];
    char *port = argv[2];
    
    if(set_info_structs(&hints, &serverinfo, host, port) != 0) {
        fprintf(stderr, "client: failed to get address info");
        exit(1);
    }
    
    sockfd = connect_to_server(serverinfo);
    
    chat(sockfd);
    
    return 0;
}

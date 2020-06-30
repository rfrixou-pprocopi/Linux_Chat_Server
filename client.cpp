// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>

#define PORT 6970
#define PORT2 6868
#define MAXLINE 1024
#define MAXBYTES 256

pthread_mutex_t mutex;
//pthread_mutex_t sock_mutex;
int sockt;
int sockf;
int port2 = PORT2;

int *sock;

std::string user = "Server";

void *write_handler(void *sock) {
    char input[250];
    std::string command,temp,temp2;
    int sockfd, struct_size, new_socket,port;
    bool bye_true = false;
    bool port_loop = false;
    //while(true) {

    scanf("%[^\n]%*c", input);

    if (strcmp(input, "bye") == 0) {
        if(strcmp(user.c_str(),"Server") == 0){
            write(sockt, "bye", 250);//(const void*)
            close(sockt);
            exit(1);
        }
        pthread_mutex_lock(&mutex);
        write(sockt, "bye", 250);//(const void*)
        bye_true = true;
        std::cout << "Conversation Terminated..." << std::endl;
        user = "Server";
        close(sockt);
        sockt = sockf;
        pthread_mutex_unlock(&mutex);
    }

    int pos1, pos2;
    command = input;
    pos1 = command.find_first_of(" ");
    temp = command.substr(0, pos1);
   // std::cout << "Debag:ifenter: " << command << std::endl;
    //std::cout << "Debag:ifenter: " << temp << std::endl;


    if (strcmp(temp.c_str(), "initiate") == 0) {

        pos1 = command.find_first_of("<");
        pos2 = command.find_first_of(">");

        user = command.substr(pos1+1,pos2-pos1-1);
        //std::cout << "Debag:user: " << command.substr(pos1+1,pos2-pos1-1) << std::endl;

        struct sockaddr_in servaddr, cliaddr;


        // Creating socket file descriptor
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        //std::cout << "Debag:socket: " << sockfd << std::endl;

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr.sin_family = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(port2);

        // Bind the socket with the server address
        while (bind(sockfd, (const struct sockaddr *) &servaddr,
                 sizeof(servaddr)) < 0) {
            perror("bind failed");
            servaddr.sin_port = htons(++port2);
            //exit(EXIT_FAILURE);
        }

        //==================================================
        temp2 = input;
        temp2 = temp2 + "|" + std::to_string(port2) + "|";
        write(sockt, temp2.c_str(), 250);//(const void*)

        //Listen
        listen(sockfd, 3);

        struct readThreadParams *readParams;
        //Accept any incoming connection
        std::cout << "Accepting all pending connections" << std::endl;
        struct_size = sizeof(struct sockaddr_in);

        while (new_socket = accept(sockfd, (struct sockaddr *) &cliaddr, (socklen_t *) &struct_size)) {
            sockt = new_socket;
            //std::cout << "Debug:print_input:" << input << std::endl;
        }

        if (new_socket < 0) {
            perror("Connection Failed");
            //return 1;
        }
    }

   // std::cout << "Debug:print_input:" << input << std::endl;
    pthread_mutex_lock(&mutex);
    if(!bye_true){
        write(sockt, input, 250);//(const void*)
    }
    sleep(2);
    pthread_mutex_unlock(&mutex);
    fflush(stdin);
    //}
}


void *read_handler(void *sock){
    char buffer[250];
    int n; int sockfd;
    bool init = false;
    bool change_user = true;
    int pos1,pos2,pos3,pos4,pos5,pos6;
    std::string command,req_user,init_str,mes_port;
    struct sockaddr_in servaddr;

    //std::cout << "Socket:" << sockt << std::endl;
    n = read(sockt, buffer, 250);//(void*)
    buffer[n] = '\0';

    if(!init){
        command = "";
        command.append(buffer);

        pos1 = command.find_first_of("<");
        pos2 = command.find_first_of(">");
        pos3 = command.find_first_of("(");
        pos4 = command.find_first_of(")");
        pos5 = command.find_first_of("|");
        pos6 = command.find_last_of("|");

        req_user = command.substr(pos1+1,pos2-pos1-1);
        init_str = command.substr(pos3+1,pos4-pos3-1);
        mes_port = command.substr(pos5+1,pos6-pos5-1);

        std::cout << "Debug:port" << mes_port << std::endl;

        //std::cout << "Debag:Command: " << command << std::endl;

        if(strcmp(init_str.c_str(),"init") == 0){
            //write(sockt, ("initiate <" + req_user + ">").c_str(), 250);//(const void*)
            //sockt = stoi(req_user);

            //std::cout << "Debag:user: " << command.substr(pos1+1,pos2-pos1-1) << std::endl;
            user = req_user;

            //std::cout << "Debag:ifenterclient: " << std::endl;
            sleep(2);

            if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }

           // std::cout << "Debag:socket: " << sockfd << std::endl;

            /*int optval = 1;
            setsockopt(sockfd, SO_KEEPALIVE, &optval, sizeof(optval));
            //SO_KEEPALIVE*/

            memset(&servaddr, 0, sizeof(servaddr));

            // Filling server information
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(stoi(mes_port));
            servaddr.sin_addr.s_addr = INADDR_ANY;


            if (connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0) {
                perror("connect failed. Error");
                //return 1;
            }else{
                sockt = sockfd;
                //std::cout << "Debag:connected: " << std::endl;
            }
        }

        init = true;
    }

    if(strcmp(buffer,"bye") == 0){
        pthread_mutex_lock(&mutex);
        //write(sockt, "bye", 250);//(const void*)
        std::cout << "Conversation Terminated..." << std::endl;
        std::cout << "From: " << user << ": " << buffer << std::endl;
        change_user = false;
        user = "Server";
        close(sockt);
        sockt = sockf;
        pthread_mutex_unlock(&mutex);
    }

    if(change_user){
        std::cout << "From: " << user << ": " << buffer << std::endl;
    }

}

// Driver code
int main() {
    int sockfd;
    char buffer[250];
    char *bye = "bye";
    char input[250];
    char *hello = "Hello from client";

    int num_readable;
    int    num_bytes;
    char   buf[MAXBYTES];
    fd_set readfds;

    struct sockaddr_in servaddr;
    struct timeval tv;

    bool init;

    int fd_stdin;
    fd_stdin = fileno(stdin);

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    //while(!init)

    std::cout << "Connection Successful!" << std::endl;

    int n, len;

    //do{
    //scanf("%s", input);
    scanf("%[^\n]%*c", input);
    //std::cout << "Debug:print_input_size:" << sizeof(input) << std::endl;
    //std::cout << "Debug:print_input:" << input << std::endl;

    write(sockfd,input,250);//(const void*)

    fflush(stdin);
   // printf("Hello message sent.\n");
    std::cout << "From: " << user << ": " << buffer << std::endl;
    //printf("Buffer : %s\n", buffer);

    n = read(sockfd, buffer, 250);//(void*)
    std::cout << "From: " << user << ": " << buffer << std::endl;
    //printf("Server : %s\n", buffer);

    pthread_t writer_thread;
    pthread_t reader_thread;

    sockt = sockfd;

    sockf = sockfd;

    while(true){
        if (pthread_create(&reader_thread, NULL, read_handler, (void *) sockt) < 0) {
            perror("Could not create thread");
            return 1;
        }

        sleep(2);

        if (pthread_create(&writer_thread, NULL, write_handler, (void *) sockt) < 0) {
            perror("Could not create thread");
            return 1;
        }
        sleep(2);
    }

    close(sockfd);
    return 0;
}

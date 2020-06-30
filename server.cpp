// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <pthread.h>

#include <iostream>
#include <vector>

#include <iostream>
#include <vector>
#include <cstring>

#include <set>
#include <map>
#include <sstream>
#include <iterator>

#define PORT 6970
#define PORT2 6868
#define MAXLINE 1024

struct readThreadParams {
    struct sockaddr_in cliaddr;
    int *sock_p;
};

std::set<std::string> users;
std::map<std::string,int> portmap;

pthread_mutex_t mutex;

void *connection_handler(void *argParams)
{
    //Get the socket descriptor
    //std::cout << "Debug:connection_handler_1" << std::endl;

    struct readThreadParams *readParams = (readThreadParams*)(argParams);
    //int read_size,new_socket;
    char message[250];
    /*
    char *hello = "Hello from server";
    char *login = "login";
    char comp[5];*/
    std::string command,function,user,temp,curr_user,req_user,mes_port;
    int req_port;
    bool loop;
    unsigned int len, n;

    //std::cout << "Debug:connection_handler_2" << std::endl;

    len = sizeof(readParams->cliaddr);  //len is value/resuslt
    int pos1,pos2,pos3,pos4,pos5;

    do {
        //std::cout << "ceiling" << std::endl;
        n = read(*(readParams->sock_p), (void *) message, 250);
        //std::cout << "floor" << std::endl;
//-----------------------------------------------------------------
        //char message[250] = "login <kwstas>";
        loop = true;
        command = "";
        command.append(message);

        pos1 = command.find_first_of("<");
        pos2 = command.find_first_of(">");
        pos3 = command.find_first_of(" ");

        user = command.substr(pos1+1,pos2-pos1-1);
        temp = command.substr(0,pos3);

        std::cout << "Command:" << command << std::endl;

        if ((strcmp(temp.c_str(),"login") == 0) && (strcmp(user.c_str(), "") != 0) && (pos3 + 1 == pos1)
            && (pos2 == command.size() - 1) && (users.find(user) == users.end())) {

            write(*(readParams->sock_p), "Login Successful!", 32);//(const void*)
            curr_user.append(user);
            std::cout << user << std::endl;

            pthread_mutex_lock(&mutex);
            users.insert(user);
            pthread_mutex_unlock(&mutex);
            loop = false;

            portmap.insert(std::pair<std::string, int> (user, *(readParams->sock_p)));

        } else {
            std::cout << "Login attempt failed" << std::endl;
            write(*(readParams->sock_p), "Login failed! Please try to login...", 32);//(const void*)
        }
//---------------------------------------------------------------
    }while(loop);

    //std::cout << "Debug:do_while" << std::endl;

    while(strcmp(message,"bye") != 0) {
        /*std::cout << "Debug:print_len" << len << std::endl;
        std::cout << "Debug:message_now" << message << std::endl;
        std::cout << "Debug:connection_handler_3" << std::endl;*/
//----------------------------------------------------------------------------
        pthread_mutex_lock(&mutex);
        std::set<std::string> diff;
        diff = users;
        diff.erase(user);
        pthread_mutex_unlock(&mutex);

        n = 0;
        n = read(*(readParams->sock_p), (void *) message, 250);

        if (strcmp(message, "list") == 0) {
            if(diff.empty()){
                write(*(readParams->sock_p), "You are the only remaining user!", 32);//(const void*)
            }
            else{
                //std::cout << "Debug:list" << len << std::endl;
                std::ostringstream stream;
                std::copy(diff.begin(), diff.end(), std::ostream_iterator<std::string>(stream, ","));
                std::string result = stream.str();

                //std::cout << "Debug:string" << result.c_str() << " size: " << result.size() << std::endl;

                write(*(readParams->sock_p), result.c_str(), result.size());//(const void*)
            }
        }else{
            command = "";
            command.append(message);

            pos1 = command.find_first_of("<");
            pos2 = command.find_first_of(">");
            pos3 = command.find_first_of(" ");
            pos4 = command.find_first_of("|");
            pos5 = command.find_last_of("|");

            req_user = command.substr(pos1+1,pos2-pos1-1);
            temp = command.substr(0,pos3);
            mes_port = command.substr(pos4+1,pos5-pos4-1);

            std::cout << "Debug:port" << mes_port << std::endl;

            if ((strcmp(temp.c_str(),"initiate") == 0) && (strcmp(req_user.c_str(), "") != 0) && (pos3 + 1 == pos1)
                && (pos5 == command.size() - 1) && (users.find(req_user) != users.end()) && (strcmp(req_user.c_str(),curr_user.c_str()) != 0)) {

                req_port = portmap.at(req_user);
                //std::cout << "Debug:portmap" << req_port << std::endl;

                std::string init_mess;
                init_mess = "(init) <" + curr_user + ">" + " |" + mes_port + "|";
                sleep(2);
                write(req_port, init_mess.c_str(), init_mess.size());//(const void*)
                //std::cout << "Debug:exit_kill" << std::endl;

            }else{
                if(strcmp(message,"bye") != 0) {
                    write(*(readParams->sock_p), "Unknown Command!", 32);//(const void*)
                }
            }
        }
        //---------------------------------------------------------------------------------------------------------------
        //std::cout << "Debug:list" << len << std::endl;
        //std::cout << "Debug:message" << message << std::endl;
    }
//------------------------------------------------------------------
    write(*(readParams->sock_p), "Shutting down connection...", 32);//(const void*)

    if(shutdown(*(readParams->sock_p),2) == 0){
        std::cout << "Connection with user " << user << " terminated successfully!"<< std::endl;
    }
    else{
        std::cout << "Connection with user " << user << " terminated abruptly!"<< std::endl;
    }

    //std::cout << "Debug:exit1 " << std::endl;
    pthread_mutex_lock(&mutex);
    users.erase(user);
    pthread_mutex_unlock(&mutex);
    pthread_exit;
    //std::cout << "Debug:exit2 " << std::endl;
    //pthread_cancel(pthread_self());
    //return 0;
}


// Driver code
int main() {
    int sockfd, struct_size, new_socket, *sock_p;
    char buffer[MAXLINE];
    char *hello = "Hello from server";
    char *mess = "Connection established! Handler assignment...";
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //Listen
    listen(sockfd , 3);

    struct readThreadParams *readParams;

    //Accept any incoming connection
    std::cout << "Accepting all pending connections" << std::endl;
    struct_size = sizeof(struct sockaddr_in);

    int i = 0;
    pthread_t sniffer_thread[16];


    while ((new_socket = accept(sockfd, (struct sockaddr *) &cliaddr, (socklen_t *) &struct_size)) && (i <= 16)) {
        std::cout << "Connection Accepted" << std::endl;
        write(new_socket, mess, strlen(mess));


        sock_p = (int *) malloc(1);
        *sock_p = new_socket;

        readParams = static_cast<readThreadParams *> (malloc(sizeof(*readParams)));
        readParams->cliaddr = cliaddr;
        readParams->sock_p = sock_p;

        if (pthread_create(&sniffer_thread[i++], NULL, connection_handler, readParams) < 0) {
            perror("Could not create thread");
            return 1;
        }

        //pthread_join( sniffer_thread , NULL);
        std::cout << "Handler Assigned." << std::endl;
    }


    if (new_socket < 0) {
        perror("Connection Failed");
        return 1;
    }

    //std::cout << "GOODBYE!" << std::endl;

    return 0;
}
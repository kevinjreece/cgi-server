//
//  main.cpp
//  BasicWebServer
//
//  Created by Kevin J Reece on 1/28/15.
//  Copyright (c) 2015 Kevin J Reece. All rights reserved.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include "ConcurrentQueue.h"
#include "SocketServingSocket.h"

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define QUEUE_SIZE          5

ConcurrentQueue con_queue;
std::mutex mut;

void serve(int id, string path) {
    while (true) {

        mut.lock();

        ServerSocket* socket = con_queue.deQ();
        cout << "Socket number " << id << " received a connection.\n";
        
        socket->processRequest();
        
        printf("\nClosing the socket");
        /* close socket */
        if(!socket->closeSocket()) {
            free(socket);
            printf("\nCould not close socket\n");
        }
    }
}

int main(int argc, char* argv[])
{
    mut.lock();
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    int nHostPort;
    int num_threads;
    string path;
    
    if(argc < 4) {
        printf("\nUsage: server host-port thread-num home-dir\n");
        return 0;
    }
    else {
        nHostPort=atoi(argv[1]);
        num_threads = atoi(argv[2]);
        path = argv[3];
    }
    
    printf("\nStarting server");
    
    SocketServingSocket server = SocketServingSocket(nHostPort, path);

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; i++) {
        threads.push_back(std::thread(serve, i, path));
        usleep(30);
    }

    server.listen(QUEUE_SIZE);
    
    while (true) {
        
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        ServerSocket* socket = server.acceptConnection(
                                                    (struct sockaddr*)&Address,
                                                    (socklen_t *)&nAddressSize);
        
        printf("\nGot a connection from %X (%d)\n",
        Address.sin_addr.s_addr,
        ntohs(Address.sin_port));
        
        con_queue.enQ(socket);
        mut.unlock();
        
    }
    

}

/*  execve() to pass environment variables to .cgi
        char *argToChild[5];
        char *envToChild[5];
        argvToChild[0] = (char *)"foo";
        envToChild[0] = (char *)"bar";
 
 */

/*
    pid_t pID = fork();
    if (pID == 0) {
        // this is the child
        // make argc and exec arrays
        execve;
    }
    else {
        // this is the parent

        int stat;
        int err;
        err = wait(&stat); // This waits for the child to die so that the parent can harvest it
    }
 */
/*
    chroot() -> to change root directory of program
 */
/*
    int pipe (int fildes[2]) to go from parent to child and vice versa
 */
/*
    dupe to go from child to program
 */

















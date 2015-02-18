//
//  SocketServingSocket.h
//  BasicWebServer
//
//  Created by Kevin J Reece on 1/28/15.
//  Copyright (c) 2015 Kevin J Reece. All rights reserved.
//

#ifndef BasicWebServer_SocketServingSocket_h
#define BasicWebServer_SocketServingSocket_h

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>

#include "ServerSocket.h"

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255

class SocketServingSocket {
private:
    int _socket;
    int _host_port;
    string _home_dir;
    bool _is_connected;
    
public:
    SocketServingSocket(int host_port, string home_dir) {
        _host_port = host_port;
        _home_dir = home_dir;
        
        struct hostent* host_info;   /* holds info about a machine */
        struct sockaddr_in address; /* Internet socket address stuct */
        int address_size = sizeof(struct sockaddr_in);
        
        printf("\nMaking socket");
        /* make a socket */
        _socket=socket(AF_INET,SOCK_STREAM,0);
        
        if(_socket == SOCKET_ERROR)
        {
            printf("\nCould not make a socket\n");
            _is_connected = false;
        }
        
        int optval = 1;
        setsockopt (_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        
        /* fill address struct */
        address.sin_addr.s_addr=INADDR_ANY;
        address.sin_port=htons(_host_port);
        address.sin_family=AF_INET;
        
        printf("\nBinding to port %d",_host_port);
        
        /* bind to a port */
        if(::bind(_socket,(struct sockaddr*) &address,sizeof(address)) < 0) {
            printf("\nCould not connect to host\n");
            _is_connected = false;
        }
        /*  get port number */
        getsockname( _socket, (struct sockaddr *) &address,(socklen_t *)&address_size);
        printf("opened socket as fd (%d) on port (%d) for stream i/o\n",_socket, ntohs(address.sin_port) );
        
        printf("Server\n\
               sin_family        = %d\n\
               sin_addr.s_addr   = %d\n\
               sin_port          = %d\n"
               , address.sin_family
               , address.sin_addr.s_addr
               , ntohs(address.sin_port)
               );
    }
    
    bool listen(int queue_size) {
        if(::listen(_socket,queue_size) == SOCKET_ERROR) {
            printf("\nCould not listen\n");
            _is_connected = false;
            return false;
        }
        else {
            _is_connected = true;
            return true;
        }
    }
    
    ServerSocket* acceptConnection(struct sockaddr* address, socklen_t* address_size) {
        int new_socket = accept(_socket, address, address_size);
        
        ServerSocket* new_server_socket = new ServerSocket(new_socket, _home_dir);
        
        return new_server_socket;
    }
    
    bool closeSocket() {
        if (close(_socket) == -1) {
            return false;
        }
        else {
            _is_connected = false;
            return true;
        }
    }
    
};



#endif

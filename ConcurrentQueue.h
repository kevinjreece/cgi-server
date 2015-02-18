//
//  MutQueue.h
//  BasicWebServer
//
//  Created by Kevin J Reece on 1/29/15.
//  Copyright (c) 2015 Kevin J Reece. All rights reserved.
//

#ifndef BasicWebServer_MutQueue_h
#define BasicWebServer_MutQueue_h

#include <queue>
#include <mutex>
#include "ServerSocket.h"

class ConcurrentQueue {
private:
    std::queue<ServerSocket*> _queue;
    std::mutex _mutex;
    
    
public:
    ConcurrentQueue() {
    }
    
    void enQ(ServerSocket* socket) {
        _mutex.lock();
        _queue.push(socket);
        _mutex.unlock();
    }
    
    ServerSocket* deQ() {
        _mutex.lock();
        ServerSocket* socket = _queue.front();
        _queue.pop();
        _mutex.unlock();
        return socket;
    }
    
};

#endif

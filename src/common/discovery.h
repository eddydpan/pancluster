#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

namespace discovery {

const int DISCOVERY_PORT = 8888;
const char* DISCOVERY_MESSAGE = "PANCLUSTER_MASTER";
const int MAX_RETRIES = 5;
const int TIMEOUT_SECONDS = 2;

// Broadcast master node IP address
class Broadcaster {
public:
    Broadcaster(int port = DISCOVERY_PORT) : port_(port), running_(false) {}
    
    bool start();
    void stop();
    void broadcast_once();
    
private:
    int port_;
    bool running_;
    int sockfd_;
};

// Discover master node IP address
class Discoverer {
public:
    Discoverer(int port = DISCOVERY_PORT) : port_(port) {}
    
    std::string discover(int timeout_sec = TIMEOUT_SECONDS, int max_retries = MAX_RETRIES);
    
private:
    int port_;
};

} // namespace discovery

#endif // DISCOVERY_H

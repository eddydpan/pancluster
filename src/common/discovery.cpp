#include "discovery.h"
#include <thread>
#include <chrono>

namespace discovery {

bool Broadcaster::start() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        std::cerr << "Failed to create broadcast socket" << std::endl;
        return false;
    }

    int broadcast_enable = 1;
    if (setsockopt(sockfd_, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        std::cerr << "Failed to enable broadcast" << std::endl;
        close(sockfd_);
        return false;
    }

    running_ = true;
    return true;
}

void Broadcaster::broadcast_once() {
    if (!running_) return;

    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port_);
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(sockfd_, DISCOVERY_MESSAGE, strlen(DISCOVERY_MESSAGE), 0,
           (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
}

void Broadcaster::stop() {
    running_ = false;
    if (sockfd_ >= 0) {
        close(sockfd_);
    }
}

std::string Discoverer::discover(int timeout_sec, int max_retries) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create discovery socket" << std::endl;
        return "";
    }

    // Set socket timeout
    struct timeval tv;
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Bind to discovery port
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port_);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "Failed to bind discovery socket" << std::endl;
        close(sockfd);
        return "";
    }

    char buffer[256];
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    std::cout << "Listening for master node broadcasts on port " << port_ << "..." << std::endl;

    for (int retry = 0; retry < max_retries; retry++) {
        int bytes_received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                      (struct sockaddr*)&sender_addr, &sender_len);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            if (strcmp(buffer, DISCOVERY_MESSAGE) == 0) {
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &sender_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
                std::cout << "Discovered master node at: " << ip_str << std::endl;
                close(sockfd);
                return std::string(ip_str);
            }
        } else {
            std::cout << "No broadcast received, retrying... (" << (retry + 1) << "/" << max_retries << ")" << std::endl;
        }
    }

    std::cerr << "Failed to discover master node after " << max_retries << " attempts" << std::endl;
    close(sockfd);
    return "";
}

} // namespace discovery

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include "../common/discovery.h"

volatile sig_atomic_t stop_flag = 0;

void signal_handler(int) {
    stop_flag = 1;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    discovery::Broadcaster broadcaster;
    
    if (!broadcaster.start()) {
        std::cerr << "Failed to start broadcaster" << std::endl;
        return 1;
    }

    std::cout << "Master node starting..." << std::endl;
    std::cout << "Broadcasting discovery messages on port " << discovery::DISCOVERY_PORT << std::endl;
    std::cout << "Press Ctrl+C to exit." << std::endl;

    while (!stop_flag) {
        broadcaster.broadcast_once();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    broadcaster.stop();
    std::cout << "\nMaster node shutting down." << std::endl;
    
    return 0;
}
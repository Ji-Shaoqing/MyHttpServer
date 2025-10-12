#include<iostream>
#include<csignal>
#include<memory>
#include<stdexcept>
#include"SimpleSocket.hpp"
#include"SimpleServer.hpp"

std::unique_ptr<SimpleServer> serverPtr = nullptr; // Global server instance pointer.

void signalHandler(int signal) {
    std::cout << "\nInterrupt signal (" << signal << ") received.\n";
    serverPtr.reset(); // Release the server instance.
    std::cout << "Server resources released." << std::endl;
    exit(signal);
}

int main() {
    std::cout << "Starting server..." << std::endl;
    signal(SIGINT, signalHandler); // Register signal handler for Ctrl+C.
    signal(SIGTERM, signalHandler); // Register signal handler for termination signal.
    try {
        serverPtr = std::make_unique<SimpleServer>(8080);
        serverPtr->start();
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
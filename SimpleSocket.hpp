#ifndef SIMPLESOCKET_HPP
#define SIMPLESOCKET_HPP

#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<iostream>

class SimpleSocket {
public:
    SimpleSocket() {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(socket_fd < 0) {
            std::cout << "Failed to create socket!" << std::endl;
        }
    }

    ~SimpleSocket() {
        if(socket_fd >= 0) {
            close(socket_fd);
            std::cout << "Close socket automatically." << std::endl;
        }
    }

    int getFd() const {
        return socket_fd;
    }

    bool isValid() const {
        return socket_fd >= 0;
    }

private:
    int socket_fd = -1;
};

#endif // SIMPLESOCKET_HPP
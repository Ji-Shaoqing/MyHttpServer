#ifndef SIMPLESERVER_HPP
#define SIMPLESERVER_HPP

#include"SimpleSocket.hpp"
#include<iostream>
#include<string>
#include<sstream>
#include<memory>

class SimpleServer {
public:
    SimpleServer(int pt): port(pt) {
        setupServer();
    }

    void setupServer() { // Setup server socket.
        if(!server_socket.isValid()) {
            throw std::runtime_error("Failed to create socket!");
        }
        int opt = 1; // For reusing address.
        setsockopt(server_socket.getFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in server_addr; // Server address structure.
        server_addr.sin_family = AF_INET; // IPv4
        server_addr.sin_port = htons(port); // Port
        server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address.
        if(bind(server_socket.getFd(), (sockaddr*)&server_addr, sizeof(server_addr)) < 0) { // Bind socket to the address and port.
            throw std::runtime_error("Failed to bind socket!" + std::to_string(port));
        }
        if(listen(server_socket.getFd(), 10) < 0) { // Start listening for incoming connections.
            throw std::runtime_error("Failed to listen on socket!");
        }
        std::cout << "Server listening on port " << port << "..." << std::endl;
    }

    std::string createHTMLResponse(const std::string title, const std::string content) {
        return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="UTF-8">
            <title>)" + title + R"(</title>
            <style>
                body { font-family: Arial, sans-serif; margin: 40px; }
                .container { max-width: 800px; margin: 0 auto; }
                .header { background-color: #4caf50; color: white; padding: 20px; border-radius: 5px; }
            </style>
        </head>
        <body>
            <div class="container">
                <div class="header">
                    <h1>)" + title + R"(</h1>
                </div>
                <div class="content">
                    <p>)" + content + R"(</p>
                </div>
            </div>
        </body>
        </html>
        )";
    }

    void handleClient(int client_fd) {
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Client connected." << std::endl;
        std::cout << "Handling client request..." << std::endl;

        // 读取请求（循环直到读到空行分隔请求头或读取结束）
        std::string req;
        const int chunk = 1024;
        char buf[chunk];
        ssize_t n;
        while ((n = read(client_fd, buf, chunk)) > 0) {
            req.append(buf, buf + n);
            if (req.find("\r\n\r\n") != std::string::npos) break;
            // 如果数据量超过一定大小，也停止读取以避免无限增长
            if (req.size() > 16 * 1024) break;
            // 若还有数据可读则继续循环（阻塞式）
            // 在简单服务器中不做更复杂的非阻塞处理
        }
        if (n < 0) {
            std::cerr << "Read error on client socket." << std::endl;
            close(client_fd);
            return;
        }

        std::cout << "Received request:\n" << req << std::endl;

        // 解析请求首行
        size_t pos = req.find("\r\n");
        std::string firstLine = (pos == std::string::npos) ? req : req.substr(0, pos);
        std::cout << "Request line: " << firstLine << std::endl;

        std::istringstream iss(firstLine);
        std::string method, path, httpver;
        iss >> method >> path >> httpver;

        // 如果浏览器请求 favicon，则返回 404 并关闭连接
        if (path == "/favicon.ico") {
            std::string notfound = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Length: 0\r\n"
                                   "Connection: close\r\n\r\n";
            ssize_t sent = 0;
            size_t total = notfound.size();
            while (sent < (ssize_t)total) {
                ssize_t s = send(client_fd, notfound.c_str() + sent, total - sent, 0);
                if (s <= 0) break;
                sent += s;
            }
            close(client_fd);
            std::cout << "Favicon request handled (404) and connection closed." << std::endl;
            return;
        }

        // 构造响应（包含 Content-Length 和 Connection: close）
        std::string body = createHTMLResponse("Simple Server", "Hello from SimpleServer!");
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html; charset=UTF-8\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
        std::string http_response = resp.str();

        // 发送响应（处理部分发送）
        ssize_t sent = 0;
        size_t total = http_response.size();
        while (sent < (ssize_t)total) {
            ssize_t s = send(client_fd, http_response.c_str() + sent, total - sent, 0);
            if (s <= 0) break;
            sent += s;
        }

        close(client_fd);
        std::cout << "Response sent and connection closed." << std::endl;
    }

    void start() {
        std::cout << "Waiting for client connection...Press Ctrl+C to stop the server." << std::endl;
        while(true) {
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_socket.getFd(), (sockaddr*)&client_addr, &client_len);
            if(client_fd < 0) {
                std::cout << "Failed to accept connection!" << std::endl;
                continue;
            }
            handleClient(client_fd);
        }
    }

private:
    SimpleSocket server_socket;
    int port;
};

#endif // SIMPLESERVER_HPP
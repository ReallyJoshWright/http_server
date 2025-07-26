#pragma once

#include <expected>
#include <string>

#include <netinet/in.h>

#include "logger.h"

enum class HttpProtocol {
    Http,
    Https,
};

struct HttpOptions {
    HttpProtocol protocol;
    std::string address;
    int port;
    int max_number_of_requests;
    bool to_log;
};

class HttpServer {
    public:
        std::string protocol;
        std::string ip_address;
        int port;
        int max_number_of_requests;
        int server_fd;
        struct sockaddr_in address;
        socklen_t address_len;
        Logger log;

        HttpServer(HttpOptions options);
        ~HttpServer();
        std::expected<int, std::string> listener();
        std::expected<bool, std::string> openBrowser();
        std::expected<int, std::string> accepter(int client_socket);
        std::expected<bool, std::string> handleClient(int client, bool use_ws);
        void closeSocket(int socket);

    private:
};

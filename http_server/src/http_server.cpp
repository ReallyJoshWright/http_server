#include <expected>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <format>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "http_server.h"
#include "logger.h"

using std::string;
using std::format;
using std::expected;
using std::unexpected;

///////////////////////////////////////////////////////////////////////////////
// constructors
///////////////////////////////////////////////////////////////////////////////
HttpServer::HttpServer(HttpOptions options) {
    if (options.protocol == HttpProtocol::Http) {
        protocol = "http";
    } else if (options.protocol == HttpProtocol::Https) {
        protocol = "https";
    }

    if (options.to_log == true) {
        log.setLogger(true);
    } else {
        log.setLogger(false);
    }

    ip_address = options.address;
    port = options.port;
    max_number_of_requests = options.max_number_of_requests;
    server_fd = -1;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    address_len = sizeof(address);
}

///////////////////////////////////////////////////////////////////////////////
// destructors
///////////////////////////////////////////////////////////////////////////////
HttpServer::~HttpServer() {
    if (server_fd != -1) {
        closeSocket(server_fd);
    }
}

///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////

expected<int, string> HttpServer::listener() {
    // socket(IPv4, TCP, TCP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return unexpected("Socket creation failed");
    }

    int enable = 1;
    // setsockopt(socket file descriptor, socket API level, reuse port, on)
    if (setsockopt(
            server_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (const char *)&enable,
            sizeof(enable)
        ) < 0) {

        closeSocket(server_fd);
        return unexpected("setsockopt failed");
    }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        closeSocket(server_fd);
        return unexpected("Bind failed. Check if port is already in use.");
    }

    // listen(socket file descriptor, max num of pending connections)
    if (listen(server_fd, max_number_of_requests) < 0) {
        closeSocket(server_fd);
        return unexpected("Listen failed");
    }

    log.info(format("Server listening on port {}", port));

    int client_socket = -1;

    return client_socket;
}

expected<bool, string> HttpServer::openBrowser() {
    string cmd_str = format(
        "xdg-open {}://{}:{} > /dev/null 2>&1",
        protocol,
        ip_address,
        port
    );

    const char *cmd = cmd_str.c_str();
    if (std::system(cmd) != 0) {
        string cmd_err = format("Error: {}: command not found", cmd_str);
        return unexpected(cmd_err);
    }

    return true;
}

expected<int, string> HttpServer::accepter(int client_socket) {
    if ((client_socket = accept(
        server_fd,
        (struct sockaddr *)&address,
        &address_len
    )) < 0) {

        return unexpected("Accept failed");
    }

    char client_ip[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);

    log.info(
        format(
            "Connection accepted from {}:{}",
            client_ip,
            ntohs(address.sin_port)
        )
    );

    return client_socket;
}

expected<bool, string> HttpServer::handleClient(int client, bool use_ws) {
    char buffer[4096] = {0};
    int bytes_received;

    bytes_received = read(client, buffer, sizeof(buffer) - 1);

    if (bytes_received <= 0) {
        string error = format(
            "Client {} disconnected during initial HTTP request "
            "(recv returned {}).",
            client,
            bytes_received
        );

        closeSocket(client);
        return unexpected(error);
    }

    buffer[bytes_received] = '\0';
    string request(buffer);
    log.info(
        format(
            "Received request from client {}:\n{}",
            client,
            request.substr(0, request.find('\n'))
        )
    );

    if (use_ws == false) {
        size_t first_space = request.find(' ');
        size_t second_space = request.find(' ', first_space + 1);

        if (first_space == string::npos || second_space == string::npos) {
            string bad_request_response =
                "HTTP/1.1 400 Bad Request\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 15\r\n"
                "Connection: close\r\n"
                "\r\n"
                "400 Bad Request";

            write(
                client,
                bad_request_response.c_str(),
                bad_request_response.length()
            );

            closeSocket(client);
            return unexpected("Invalid HTTP request line.");
        }

        string path = request.substr(
            first_space + 1,
            second_space - (first_space + 1)
        );

        string file_path = "public";
        if (path == "/") {
            file_path += "/index.html";
        } else {
            file_path += path;
        }

        string content_type = "text/plain";
        if (file_path.ends_with(".html")) {
            content_type = "text/html";
        } else if (file_path.ends_with(".css")) {
            content_type = "text/css";
        } else if (file_path.ends_with(".js")) {
            content_type = "application/javascript";
        } else if (file_path.ends_with(".ico")) {
            content_type = "image/x-icon";
        }

        std::ifstream file(file_path, std::ios::binary);
        if (file.is_open()) {
            std::stringstream ss;
            ss << file.rdbuf();
            string file_content = ss.str();
            file.close();

            string http_response = format(
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: {}\r\n"
                "Content-Length: {}\r\n"
                "Connection: close\r\n"
                "\r\n"
                "{}",
                content_type,
                file_content.length(),
                file_content
            );

            write(client, http_response.c_str(), http_response.length());
        } else {
            string not_found_response =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 13\r\n"
                "Connection: close\r\n"
                "\r\n"
                "404 Not Found";

            write(
                client,
                not_found_response.c_str(),
                not_found_response.length()
            );
        }

        closeSocket(client);
    } else {
        string bad_request_response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 15\r\n"
            "Connection: close\r\n"
            "\r\n"
            "400 Bad Request";

        write(
            client,
            bad_request_response.c_str(),
            bad_request_response.length()
        );

        closeSocket(client);
    }

    return true;
}

void HttpServer::closeSocket(int socket) {
    if (socket != -1) {
        close(socket);
    }
}

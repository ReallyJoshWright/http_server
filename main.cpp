#include <iostream>
#include <print>

#include "http_server.h"

using std::println;
using std::cerr;

int main() {
    HttpOptions options;
    options.port = 3000;
    options.address = "127.0.0.1";
    options.protocol = HttpProtocol::Http;
    options.max_number_of_requests = 10;
    options.to_log = true;

    HttpServer server(options);
    int client_socket = -1;

    auto result1 = server.listener();
    if (!result1.has_value()) {
        println(cerr, "{}", result1.error());
        return 1;
    } else {
        client_socket = result1.value();
    }

    auto result2 = server.openBrowser();
    if (!result2.has_value()) {
        println(cerr, "{}", result2.error());
        return 1;
    }

    while (true) {
        auto result3 = server.accepter(client_socket);
        if (!result3.has_value()) {
            println(cerr, "{}", result3.error());
        } else {
            client_socket = result3.value();
        }

        auto result4 = server.handleClient(client_socket, false);
    }

    return 0;
}

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>

int main(int argc, char **argv) {
    // You can use print statements as follows for debugging, they'll be visible
    // when running tests.
    std::cout << "Logs from your program will appear here!\n";

    // Uncomment this block to pass the first stage

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    // Since the tester restarts your program quite often, setting REUSE_PORT
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
        0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
        0) {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
           (socklen_t *)&client_addr_len);
    std::cout << "Client connected\n";


    char buffer[4000];
    recv(client_fd, &buffer, 4000, 0);
    std::string response(buffer, 4000);
    int i = response.find('/');
    char s[1000];
    memset(s, 0, 1000);
    response = response.substr(i, response.find(' ', i)-i);
    if (response == "/") {
        strcpy(s, "HTTP/1.1 200 OK\r\n\r\n");
        printf("%s", s);
        send(client_fd, &s, 1000, 0);
        return 0;
    }
    
    int x = response.find("/echo");
    if (x == -1) {
        strcpy(s, "HTTP/1.1 404 NOT FOUND\r\n\r\n");
        send(client_fd, &s, 1000, 0);
        return 0;
    }
    response=response.substr(x+6);
    std::string message = response;
    response="HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContest-Length: ";
    std::stringstream t;
    t << message.size();
    response += t.str();
    response += "\n\r\n\r";
    response += message+"\r\n\r\n";
    std::cout << response;
    memset(s, 0, 1000);
    strcpy(s, response.c_str());
    std::cout << s;
    send(client_fd, &s, 1000, 0);
    
    close(server_fd);
    return 0;
}

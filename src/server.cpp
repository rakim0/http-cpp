#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void client(int client_fd, std::string directory)
{
    char messageBuffer[1024];
    int bytes_read = 0;

    if ((bytes_read = read(client_fd, &messageBuffer, 1024)) < 0) {
        std::cerr << "Failed to read from client\n";
        close(client_fd);
        return;
    }
    std::string httpOk = "HTTP/1.1 200 OK\r\n\r\n";
    std::string httpNo = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    int server_send = 0;

    messageBuffer[bytes_read] = '\0';
    std::string path;
    std::string request(messageBuffer);
    int methodEnd = request.find(' ');
    if (methodEnd != std::string::npos) {
        int pathStart = methodEnd + 1;
        int pathEnd = request.find(' ', pathStart);
        if (pathEnd != std::string::npos) {
            path = request.substr(pathStart, pathEnd - pathStart);
        }
    }
    // std::cout << path << '\n';
    int findEcho = request.find("echo");
    int findFile = request.find("/files/");
    int findUserAgent = request.find("User-Agent: ");

    std::string userAgent;
    std::string arg;
    std::string filename;

    if (path == "/") {
        server_send = send(client_fd, httpOk.c_str(), httpOk.size(), 0);
        if (server_send == -1) {
            std::cerr << "Server failed to send buffer!!\n";
            close(client_fd);
            return;
        }
    }
    else if (findEcho != std::string::npos) {
        if (findEcho != std::string::npos) {
            int argStart = findEcho;
            int argEnd = request.find(' ', findEcho);
            if (argEnd != std::string::npos) {
                arg = request.substr(argStart + 5, argEnd - argStart - 5);
            }
        }
        std::string response = "";
        response = response + "HTTP/1.1 200 OK\r\n" +
                   "Content-Type: text/plain\r\n" +
                   "Content-Length: " + std::to_string(arg.length()) +
                   "\r\n\r\n" + arg;
        // std::cout << response << '\n';
        server_send = send(client_fd, response.c_str(), response.size(), 0);
    }
    else if (request.find("GET /user-agent") != std::string::npos) {
        if (findUserAgent != std::string::npos) {
            int userAgentEnd = request.find("\r\n", findUserAgent + 12);
            userAgent = request.substr(findUserAgent + 12,
                                       userAgentEnd - (findUserAgent + 12));

            std::string response = "";
            response = response + "HTTP/1.1 200 OK\r\n" +
                       "Content-Type: text/plain\r\n" + "Content-Length: " +
                       std::to_string(userAgent.length()) + "\r\n\r\n" +
                       userAgent;
            // std::cout << response << '\n';
            server_send =
                send(client_fd, response.c_str(), response.size(), 0);
        }
    }

    else if (findFile != std::string::npos) {
        int fileStart = findFile + 7;
        int fileEnd = request.find(' ', findFile);
        filename = request.substr(fileStart, fileEnd - fileStart);
        std::cout << filename << '\n';
        std::string fullPath = directory + '/' + filename;

        std::ifstream file;
        file.open(fullPath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            server_send = send(client_fd, httpNo.c_str(), httpNo.size(), 0);
            if (server_send == -1) {
                std::cerr << "Server failed to send buffer!!\n";
                close(client_fd);
                return;
            }
        }

        const auto sz = std::filesystem::file_size(fullPath);
        std::string fileContent(sz, '\0');
        file.read(fileContent.data(), sz);
        std::string response = "";
        response = response + "HTTP/1.1 200 OK\r\n" +
                   "Content-Type: application/octet-stream\r\n" +
                   "Content-Length: " + std::to_string(fileContent.length()) +
                   "\r\n\r\n" + fileContent;
        // std::cout << response << '\n';
        server_send = send(client_fd, response.c_str(), response.size(), 0);
    }

    else {
        server_send = send(client_fd, httpNo.c_str(), httpNo.size(), 0);
        if (server_send == -1) {
            std::cerr << "Server failed to send buffer!!\n";
            close(client_fd);
            return;
        }
    }
    close(client_fd);
    return;
}

int main(int argc, char **argv)
{
    // You can use print statements as follows for debugging, they'll be
    // visible when running tests.
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
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse,
                   sizeof(reuse)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) != 0) {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }

    std::string directory = "";
    if (argc == 3) {
        directory = argv[2];
    }
    std::cout << directory << '\n';

    std::vector<std::thread> thread_list;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                               (socklen_t *)&client_addr_len);
        if (client_fd < 0) {
            // std::cerr << "client failed to connect\n";
            return 1;
        }
        // std::cout << "Client connected\n";
        thread_list.emplace_back(client, client_fd, directory);
    }
    for (auto &thread : thread_list) {
        thread.join();
    }
    close(server_fd);
}

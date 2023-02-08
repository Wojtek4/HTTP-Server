#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdexcept>
#include "config.hpp"

void setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

int get_listen_socket() {

    int listen_socket;
    sockaddr_in serv_addr;

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0) {
        throw std::runtime_error("ERROR opening socket");
    }

    setnonblocking(listen_socket);

    bzero((char*)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(listen_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("ERROR on binding");
    }
    
    if (listen(listen_socket, MAX_QUEUE_LENGTH) == -1) {
        throw std::runtime_error("ERROR on listening");
    }

    return listen_socket;
}

#endif /* SOCKET_HPP */

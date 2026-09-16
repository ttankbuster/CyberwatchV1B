#include "network.h"


typedef struct {
    const char* host;
    int port;
    int socket_fd;
} Network;

int network_init(Network* network, const char* host, int port) {
    if (network == NULL || host == NULL) {
        return -1;
    }
    network->host = host;
    network->port = port;
    network->socket_fd = -1;
    return 0;
}

int network_test(){

}
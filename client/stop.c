/*
 * stop.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "stop.h"
#include "config.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int stopServer()
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char response[HANDSHAKE_SIZE];

    memset(response, '\0', sizeof(response));

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0)
    {
        printf("Unable to create socket\n");
        return -1;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Unable to connect\n");
        close(socket_desc);
        return -1;
    }

    // Send STOP command
    if (send(socket_desc, "STOP", 4, 0) < 0)
    {
        printf("Unable to send request\n");
        close(socket_desc);
        return -1;
    }

    // Receive response
    recv(socket_desc, response, sizeof(response), 0);

    printf("Server stopped\n");
    close(socket_desc);
    return 0;
}
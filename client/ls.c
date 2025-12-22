/*
 * ls.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "ls.h"
#include "config.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int listVersions(char *remoteFile)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char client_handshake[HANDSHAKE_SIZE], server_response[BUFFER_SIZE];

    memset(server_response, '\0', sizeof(server_response));
    memset(client_handshake, '\0', sizeof(client_handshake));

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

    // Send LS request
    snprintf(client_handshake, sizeof(client_handshake), "LS %s", remoteFile);

    if (send(socket_desc, client_handshake, strlen(client_handshake), 0) < 0)
    {
        printf("Unable to send request\n");
        close(socket_desc);
        return -1;
    }

    // Receive response
    if (recv(socket_desc, server_response, sizeof(server_response), 0) < 0)
    {
        printf("Error receiving server response\n");
        close(socket_desc);
        return -1;
    }

    close(socket_desc);

    // Check for errors
    if (strncmp(server_response, "OK ", 3) != 0)
    {
        printf("Server error: %s", server_response);
        return -1;
    }

    // Parse and display versions
    printf("Versions of %s:\n", remoteFile);

    char *line = strtok(server_response, "\n");
    line = strtok(NULL, "\n"); // skip "OK <count>"

    while (line != NULL)
    {
        char version[16];
        long timestamp;
        if (sscanf(line, "%s %ld", version, &timestamp) == 2)
        {
            time_t t = (time_t)timestamp;
            printf("  %s  %s", version, ctime(&t));
        }
        line = strtok(NULL, "\n");
    }

    return 0;
}
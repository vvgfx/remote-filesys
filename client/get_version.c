/*
 * get_version.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "get_version.h"
#include "config.h"
#include "utility.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int getFileVersion(char *remoteFile, char *version, char *localFile)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char client_handshake[HANDSHAKE_SIZE], server_handshake[HANDSHAKE_SIZE];
    char buffer[BUFFER_SIZE];

    memset(server_handshake, '\0', sizeof(server_handshake));
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
    printf("Connected with server successfully\n");

    // Send GET_VERSION request
    snprintf(client_handshake, sizeof(client_handshake), "GET_VERSION %s %s", remoteFile, version);

    if (send(socket_desc, client_handshake, strlen(client_handshake), 0) < 0)
    {
        printf("Unable to send request\n");
        close(socket_desc);
        return -1;
    }

    // Receive handshake response
    if (recv(socket_desc, server_handshake, sizeof(server_handshake), 0) < 0)
    {
        printf("Error receiving server response\n");
        close(socket_desc);
        return -1;
    }
    printf("Server's response: %s", server_handshake);

    // Check for errors
    if (strncmp(server_handshake, "OK ", 3) != 0)
    {
        printf("Server error: %s\n", server_handshake);
        close(socket_desc);
        return -1;
    }

    // Parse file size from response
    long fileSize = atol(server_handshake + 3);
    printf("Expecting %ld bytes\n", fileSize);

    // Build full local path
    char fullLocalPath[512];
    snprintf(fullLocalPath, sizeof(fullLocalPath), "files/%s", localFile);

    // Create directories if needed
    createDirectories(fullLocalPath);

    // Open local file for writing
    FILE *fp = fopen(fullLocalPath, "wb");
    if (fp == NULL)
    {
        printf("Cannot open local file: %s\n", fullLocalPath);
        close(socket_desc);
        return -1;
    }

    // Receive file data in chunks
    long total_received = 0;
    int bytes_received;

    while (total_received < fileSize)
    {
        bytes_received = recv(socket_desc, buffer, sizeof(buffer), 0);

        if (bytes_received < 0)
        {
            printf("Error receiving file data\n");
            fclose(fp);
            close(socket_desc);
            return -1;
        }
        if (bytes_received == 0)
        {
            printf("Connection closed unexpectedly\n");
            fclose(fp);
            close(socket_desc);
            return -1;
        }

        fwrite(buffer, 1, bytes_received, fp);
        total_received += bytes_received;
    }

    fclose(fp);
    close(socket_desc);

    printf("Received %ld bytes, wrote to %s\n", total_received, fullLocalPath);
    return 0;
}
/*
 * write.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "write.h"
#include "config.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int writeFile(char *localFile, char *remoteFile)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char client_handshake[HANDSHAKE_SIZE], server_handshake[HANDSHAKE_SIZE], server_response[HANDSHAKE_SIZE];
    char buffer[BUFFER_SIZE];

    memset(server_handshake, '\0', sizeof(server_handshake));
    memset(server_response, '\0', sizeof(server_response));
    memset(client_handshake, '\0', sizeof(client_handshake));

    // Build full local path
    char fullLocalPath[512];
    snprintf(fullLocalPath, sizeof(fullLocalPath), "files/%s", localFile);

    // Open local file for reading
    FILE *fp = fopen(fullLocalPath, "rb");
    if (fp == NULL)
    {
        printf("Cannot open local file: %s\n", fullLocalPath);
        return -1;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0)
    {
        printf("Unable to create socket\n");
        fclose(fp);
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
        fclose(fp);
        return -1;
    }
    printf("Connected with server successfully\n");

    // Send WRITE request with file size
    snprintf(client_handshake, sizeof(client_handshake), "WRITE %s %ld", remoteFile, fileSize);

    if (send(socket_desc, client_handshake, strlen(client_handshake), 0) < 0)
    {
        printf("Unable to send handshake\n");
        close(socket_desc);
        fclose(fp);
        return -1;
    }

    // Wait for READY response
    if (recv(socket_desc, server_handshake, sizeof(server_handshake), 0) < 0)
    {
        printf("Error while receiving server's response\n");
        close(socket_desc);
        fclose(fp);
        return -1;
    }
    printf("Server's response: %s\n", server_handshake);

    if (strcmp(server_handshake, "READY") != 0)
    {
        printf("Server not ready\n");
        close(socket_desc);
        fclose(fp);
        return -1;
    }

    // Send file data in chunks
    int bytes_read;
    long total_sent = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        if (send(socket_desc, buffer, bytes_read, 0) < 0)
        {
            printf("Unable to send data\n");
            close(socket_desc);
            fclose(fp);
            return -1;
        }
        total_sent += bytes_read;
    }

    fclose(fp);
    printf("Sent %ld bytes\n", total_sent);

    // Wait for SUCCESS response
    if (recv(socket_desc, server_response, sizeof(server_response), 0) < 0)
    {
        printf("Unable to verify server response\n");
        close(socket_desc);
        return -1;
    }
    printf("Server's response: %s\n", server_response);

    close(socket_desc);
    return 0;
}
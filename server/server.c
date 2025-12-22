/*
 * server.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 *
 * Multi-threaded TCP Socket Server for Remote File System.
 * Supports: WRITE, GET, GET_VERSION, RM, LS, STOP
 */

#include "server.h"
#include "config.h"
#include "get.h"
#include "get_version.h"
#include "ls.h"
#include "rm.h"
#include "utility.h"
#include "write.h"

#include <arpa/inet.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int socket_desc;

sem_t thread_sem;

typedef struct
{
    int client_sock;
    struct sockaddr_in client_addr;
} ClientArgs;

void handle_sigint(int sig)
{
    (void)sig;
    printf("\nShutting down server...\n");
    close(socket_desc);
    sem_destroy(&thread_sem);
    exit(0);
}

int main(void)
{
    socklen_t client_size;
    struct sockaddr_in server_addr, client_addr;

    // Create files directory
    mkdir("files", 0755);

    // Initialize thread pool semaphore
    if (sem_init(&thread_sem, 0, MAX_THREADS) < 0)
    {
        perror("sem_init");
        return -1;
    }

    // Initialize lock table
    for (int i = 0; i < MAX_LOCKS; i++)
    {
        lock_table[i].path[0] = '\0';
        lock_table[i].refcount = 0;
        pthread_rwlock_init(&lock_table[i].rwlock, NULL);
    }

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0)
    {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Allow port reuse
    int opt = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Bind to port
    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for connections
    if (listen(socket_desc, 16) < 0)
    {
        printf("Error while listening\n");
        close(socket_desc);
        return -1;
    }
    printf("\nListening for incoming connections (max %d concurrent)...\n", MAX_THREADS);

    // Register signal handler
    signal(SIGINT, handle_sigint);

    client_size = sizeof(client_addr);

    // Main accept loop
    while (1)
    {
        int client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

        if (client_sock < 0)
        {
            printf("Can't accept\n");
            continue;
        }

        printf("\n\nClient connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Prepare thread arguments
        ClientArgs *args = malloc(sizeof(ClientArgs));
        if (args == NULL)
        {
            printf("Failed to allocate client args\n");
            close(client_sock);
            continue;
        }
        args->client_sock = client_sock;
        args->client_addr = client_addr;

        // Spawn handler thread
        pthread_t tid;
        if (pthread_create(&tid, NULL, clientHandler, args) != 0)
        {
            printf("Failed to create thread\n");
            close(client_sock);
            free(args);
            continue;
        }

        pthread_detach(tid);
    }

    close(socket_desc);
    sem_destroy(&thread_sem);
    return 0;
}

void *clientHandler(void *arg)
{
    ClientArgs *args = (ClientArgs *)arg;
    int client_sock = args->client_sock;
    char client_handshake[HANDSHAKE_SIZE];

    // Wait for thread pool slot
    sem_wait(&thread_sem);
    printf("Thread acquired slot for client %s:%d\n", inet_ntoa(args->client_addr.sin_addr),
           ntohs(args->client_addr.sin_port));

    memset(client_handshake, '\0', sizeof(client_handshake));

    // Receive command from client
    if (recv(client_sock, client_handshake, sizeof(client_handshake), 0) < 0)
    {
        printf("Couldn't receive handshake\n");
        goto cleanup;
    }
    printf("Handshake from client: %s\n", client_handshake);

    // Parse and dispatch command
    char *token = strtok(client_handshake, " ");

    if (strcmp(token, "WRITE") == 0)
    {
        token = strtok(NULL, " ");
        char *filename = token;
        token = strtok(NULL, " ");
        int fileSize = atoi(token);

        writeFile(filename, fileSize, client_sock);
    }
    else if (strcmp(token, "GET") == 0)
    {
        token = strtok(NULL, " ");
        char *filename = token;

        getFile(filename, client_sock);
    }
    else if (strcmp(token, "GET_VERSION") == 0)
    {
        token = strtok(NULL, " ");
        char *filename = token;
        token = strtok(NULL, " ");
        int version = atoi(token + 1); // skip 'v' prefix

        getFileVersion(filename, version, client_sock);
    }
    else if (strcmp(token, "RM") == 0)
    {
        token = strtok(NULL, " ");
        char *filename = token;

        removeFile(filename, client_sock);
    }
    else if (strcmp(token, "LS") == 0)
    {
        token = strtok(NULL, " ");
        char *filename = token;

        listVersions(filename, client_sock);
    }
    else if (strcmp(token, "STOP") == 0)
    {
        send(client_sock, "OK\n", 3, 0);
        close(client_sock);
        free(args);
        sem_post(&thread_sem);
        printf("Received STOP command, shutting down...\n");
        close(socket_desc);
        exit(0);
    }

cleanup:
    close(client_sock);
    free(args);
    sem_post(&thread_sem);
    printf("Thread released slot\n");
    return NULL;
}
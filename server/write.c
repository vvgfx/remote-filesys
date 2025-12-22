/*
 * write.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "write.h"
#include "config.h"
#include "utility.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

int writeFile(char *fileName, int fileSize, int client_sock)
{
    char server_message[HANDSHAKE_SIZE];
    char buffer[BUFFER_SIZE];
    int bytes_received = 0;
    int total_received = 0;

    char basePath[512];
    snprintf(basePath, sizeof(basePath), "files/%s", fileName);

    // Acquire write lock on base path
    FileLock *lock = acquireFileLock(basePath);
    if (lock == NULL)
    {
        strcpy(server_message, "ERROR server busy");
        send(client_sock, server_message, strlen(server_message), 0);
        return -1;
    }
    pthread_rwlock_wrlock(&lock->rwlock);

    // Create parent directories if needed
    createDirectories(basePath);

    // Find next version number
    int nextVersion = findLatestVersion(basePath) + 1;

    char versionedPath[600];
    snprintf(versionedPath, sizeof(versionedPath), "%s.v%d", basePath, nextVersion);

    // Open file for writing
    FILE *fp = fopen(versionedPath, "wb");
    if (fp == NULL)
    {
        printf("Failed to open file: %s\n", versionedPath);
        strcpy(server_message, "ERROR cannot create file");
        send(client_sock, server_message, strlen(server_message), 0);
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }

    // Check available disk space
    struct statvfs stat;
    if (statvfs("files", &stat) == 0)
    {
        unsigned long available = stat.f_bavail * stat.f_frsize;
        if (available < (unsigned long)fileSize)
        {
            printf("Insufficient storage: need %d, have %lu\n", fileSize, available);
            strcpy(server_message, "ERROR insufficient storage");
            send(client_sock, server_message, strlen(server_message), 0);
            fclose(fp);
            remove(versionedPath);
            pthread_rwlock_unlock(&lock->rwlock);
            releaseFileLock(lock);
            return -1;
        }
    }

    // Send READY handshake
    strcpy(server_message, "READY");
    if (send(client_sock, server_message, strlen(server_message), 0) < 0)
    {
        printf("Can't send\n");
        fclose(fp);
        remove(versionedPath);
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }

    // Receive file data in chunks
    while (total_received < fileSize)
    {
        bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);

        if (bytes_received < 0)
        {
            printf("Error receiving data\n");
            fclose(fp);
            remove(versionedPath);
            pthread_rwlock_unlock(&lock->rwlock);
            releaseFileLock(lock);
            return -1;
        }
        if (bytes_received == 0)
        {
            printf("Connection closed unexpectedly\n");
            fclose(fp);
            remove(versionedPath);
            pthread_rwlock_unlock(&lock->rwlock);
            releaseFileLock(lock);
            return -1;
        }

        fwrite(buffer, 1, bytes_received, fp);
        total_received += bytes_received;
    }

    fclose(fp);

    printf("Received %d bytes, wrote to %s\n", total_received, versionedPath);

    // Send SUCCESS response
    strcpy(server_message, "SUCCESS");
    if (send(client_sock, server_message, strlen(server_message), 0) < 0)
    {
        printf("Can't send success response\n");
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }

    pthread_rwlock_unlock(&lock->rwlock);
    releaseFileLock(lock);
    return 0;
}

int createDirectories(char *path)
{
    char temp[512];
    char *p = NULL;

    snprintf(temp, sizeof(temp), "%s", path);

    size_t len = strlen(temp);
    if (temp[len - 1] == '/')
        temp[len - 1] = '\0';

    // Create each directory component
    for (p = temp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            mkdir(temp, 0755);
            *p = '/';
        }
    }

    return 0;
}
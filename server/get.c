/*
 * get.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "get.h"
#include "config.h"
#include "utility.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

int getFile(char *fileName, int client_sock)
{
    char response[HANDSHAKE_SIZE];
    char buffer[BUFFER_SIZE];

    char basePath[512];
    snprintf(basePath, sizeof(basePath), "files/%s", fileName);

    // Acquire read lock
    FileLock *lock = acquireFileLock(basePath);
    if (lock == NULL)
    {
        snprintf(response, sizeof(response), "ERROR server busy\n");
        send(client_sock, response, strlen(response), 0);
        return -1;
    }
    pthread_rwlock_rdlock(&lock->rwlock);

    // Find latest version
    int latestVersion = findLatestVersion(basePath);
    if (latestVersion == 0)
    {
        printf("No versions found for: %s\n", basePath);
        snprintf(response, sizeof(response), "ERROR file not found\n");
        send(client_sock, response, strlen(response), 0);
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }

    char versionedPath[600];
    snprintf(versionedPath, sizeof(versionedPath), "%s.v%d", basePath, latestVersion);

    // Open file for reading
    int fd = open(versionedPath, O_RDONLY);
    if (fd < 0)
    {
        printf("Failed to open file: %s\n", versionedPath);
        snprintf(response, sizeof(response), "ERROR file not found\n");
        send(client_sock, response, strlen(response), 0);
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }

    // Get file size
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        printf("Failed to stat file: %s\n", versionedPath);
        snprintf(response, sizeof(response), "ERROR cannot stat file\n");
        send(client_sock, response, strlen(response), 0);
        close(fd);
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }
    off_t fileSize = st.st_size;

    // Send OK handshake with file size
    snprintf(response, sizeof(response), "OK %ld\n", (long)fileSize);
    if (send(client_sock, response, strlen(response), 0) < 0)
    {
        printf("Failed to send handshake\n");
        close(fd);
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }

    // Stream file to client
    ssize_t bytes_read;
    ssize_t total_sent = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        ssize_t bytes_sent = 0;
        while (bytes_sent < bytes_read)
        {
            ssize_t n = send(client_sock, buffer + bytes_sent, bytes_read - bytes_sent, 0);
            if (n < 0)
            {
                printf("Error sending file data\n");
                close(fd);
                pthread_rwlock_unlock(&lock->rwlock);
                releaseFileLock(lock);
                return -1;
            }
            bytes_sent += n;
        }
        total_sent += bytes_sent;
    }

    close(fd);
    printf("Sent %ld bytes from %s (v%d)\n", (long)total_sent, basePath, latestVersion);

    pthread_rwlock_unlock(&lock->rwlock);
    releaseFileLock(lock);
    return 0;
}
/*
 * ls.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "ls.h"
#include "config.h"
#include "utility.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

int listVersions(char *fileName, int client_sock)
{
    char response[BUFFER_SIZE];
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

    // Extract directory and filename
    char dirPath[512];
    char baseName[256];

    strncpy(dirPath, basePath, sizeof(dirPath));
    char *lastSlash = strrchr(dirPath, '/');
    if (lastSlash)
    {
        strncpy(baseName, lastSlash + 1, sizeof(baseName));
        *lastSlash = '\0';
    }
    else
    {
        strncpy(baseName, basePath, sizeof(baseName));
        strcpy(dirPath, ".");
    }

    DIR *dir = opendir(dirPath);
    if (dir == NULL)
    {
        snprintf(response, sizeof(response), "ERROR path not found\n");
        send(client_sock, response, strlen(response), 0);
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }

    // Version info struct for collecting results
    typedef struct
    {
        int version;
        time_t mtime;
    } VersionInfo;

    VersionInfo versions[256];
    int count = 0;
    size_t baseLen = strlen(baseName);
    struct dirent *entry;

    // Scan directory for matching versions
    while ((entry = readdir(dir)) != NULL && count < 256)
    {
        if (strncmp(entry->d_name, baseName, baseLen) == 0 && strncmp(entry->d_name + baseLen, ".v", 2) == 0)
        {
            int version = atoi(entry->d_name + baseLen + 2);

            char fullPath[768];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name);

            struct stat st;
            if (stat(fullPath, &st) == 0)
            {
                versions[count].version = version;
                versions[count].mtime = st.st_mtime;
                count++;
            }
        }
    }
    closedir(dir);

    if (count == 0)
    {
        snprintf(response, sizeof(response), "ERROR file not found\n");
        send(client_sock, response, strlen(response), 0);
        pthread_rwlock_unlock(&lock->rwlock);
        releaseFileLock(lock);
        return -1;
    }

    // Sort by version number (bubble sort)
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (versions[i].version > versions[j].version)
            {
                VersionInfo tmp = versions[i];
                versions[i] = versions[j];
                versions[j] = tmp;
            }
        }
    }

    // Build response
    int offset = snprintf(response, sizeof(response), "OK %d\n", count);
    for (int i = 0; i < count; i++)
    {
        offset += snprintf(response + offset, sizeof(response) - offset, "v%d %ld\n", versions[i].version,
                           (long)versions[i].mtime);
    }

    send(client_sock, response, strlen(response), 0);

    printf("Listed %d versions of %s\n", count, basePath);

    pthread_rwlock_unlock(&lock->rwlock);
    releaseFileLock(lock);
    return 0;
}
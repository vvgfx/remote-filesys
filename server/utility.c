/*
 * utility.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "utility.h"
#include "config.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FileLock lock_table[MAX_LOCKS];
pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER;

// Note: claude helped me with this design, but all code is my own.

FileLock *acquireFileLock(const char *path)
{
    pthread_mutex_lock(&table_mutex);

    FileLock *found = NULL;
    FileLock *empty = NULL;

    // Search for existing entry or find empty slot
    for (int i = 0; i < MAX_LOCKS; i++)
    {
        if (lock_table[i].refcount > 0 && strcmp(lock_table[i].path, path) == 0)
        {
            found = &lock_table[i];
            break;
        }
        if (empty == NULL && lock_table[i].refcount == 0)
            empty = &lock_table[i];
    }

    // Use existing entry
    if (found)
    {
        found->refcount++;
        pthread_mutex_unlock(&table_mutex);
        return found;
    }

    // No empty slots available
    if (empty == NULL)
    {
        pthread_mutex_unlock(&table_mutex);
        printf("Lock table full!\n");
        return NULL;
    }

    // Claim empty slot
    strncpy(empty->path, path, sizeof(empty->path) - 1);
    empty->path[sizeof(empty->path) - 1] = '\0';
    empty->refcount = 1;

    pthread_mutex_unlock(&table_mutex);
    return empty;
}

void releaseFileLock(FileLock *lock)
{
    pthread_mutex_lock(&table_mutex);
    lock->refcount--;
    pthread_mutex_unlock(&table_mutex);
}

int findLatestVersion(const char *basePath)
{
    char dirPath[512];
    char baseName[256];

    // Extract directory and filename
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
        return 0;

    int maxVersion = 0;
    size_t baseLen = strlen(baseName);
    struct dirent *entry;

    // Scan for versioned files
    while ((entry = readdir(dir)) != NULL)
    {
        // Check if filename matches "baseName.vN" pattern
        if (strncmp(entry->d_name, baseName, baseLen) == 0 && strncmp(entry->d_name + baseLen, ".v", 2) == 0)
        {
            int version = atoi(entry->d_name + baseLen + 2);
            if (version > maxVersion)
                maxVersion = version;
        }
    }

    closedir(dir);
    return maxVersion;
}
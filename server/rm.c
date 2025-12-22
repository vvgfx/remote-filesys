/*
 * rm.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "config.h"
#define _XOPEN_SOURCE 500

#include "rm.h"
#include "utility.h"

#include <dirent.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Callback for nftw() to delete files and directories.
 *
 * @param path Path to the current entry.
 * @param sb Stat buffer (unused).
 * @param typeflag Type of entry (file, directory, etc.).
 * @param ftwbuf FTW buffer (unused).
 * @return 0 on success, non-zero on failure.
 */
static int rmCallback(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    (void)sb;
    (void)ftwbuf;

    int result;
    if (typeflag == FTW_D || typeflag == FTW_DP)
        result = rmdir(path);
    else
        result = unlink(path);

    if (result < 0)
        perror(path);

    return result;
}

int removeFile(char *fileName, int client_sock)
{
    char response[HANDSHAKE_SIZE];

    char basePath[512];
    snprintf(basePath, sizeof(basePath), "files/%s", fileName);

    // Acquire write lock
    FileLock *lock = acquireFileLock(basePath);
    if (lock == NULL)
    {
        snprintf(response, sizeof(response), "ERROR server busy\n");
        send(client_sock, response, strlen(response), 0);
        return -1;
    }
    pthread_rwlock_wrlock(&lock->rwlock);

    // Check if it's a directory
    struct stat st;
    if (stat(basePath, &st) == 0 && S_ISDIR(st.st_mode))
    {
        // Directory - recursive delete using nftw
        // FTW_DEPTH: process children before parent
        // FTW_PHYS: don't follow symlinks
        int result = nftw(basePath, rmCallback, 64, FTW_DEPTH | FTW_PHYS);
        if (result < 0)
        {
            printf("Failed to remove directory: %s\n", basePath);
            snprintf(response, sizeof(response), "ERROR failed to remove\n");
            send(client_sock, response, strlen(response), 0);
            pthread_rwlock_unlock(&lock->rwlock);
            releaseFileLock(lock);
            return -1;
        }
    }
    else
    {
        // File - delete all versions
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
            printf("Path does not exist: %s\n", basePath);
            snprintf(response, sizeof(response), "ERROR path not found\n");
            send(client_sock, response, strlen(response), 0);
            pthread_rwlock_unlock(&lock->rwlock);
            releaseFileLock(lock);
            return -1;
        }

        // Scan and delete all versions
        int deleted = 0;
        size_t baseLen = strlen(baseName);
        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL)
        {
            if (strncmp(entry->d_name, baseName, baseLen) == 0 && strncmp(entry->d_name + baseLen, ".v", 2) == 0)
            {
                char fullPath[768];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name);
                if (unlink(fullPath) == 0)
                {
                    printf("Deleted: %s\n", fullPath);
                    deleted++;
                }
            }
        }
        closedir(dir);

        if (deleted == 0)
        {
            printf("No versions found for: %s\n", basePath);
            snprintf(response, sizeof(response), "ERROR path not found\n");
            send(client_sock, response, strlen(response), 0);
            pthread_rwlock_unlock(&lock->rwlock);
            releaseFileLock(lock);
            return -1;
        }

        printf("Removed %d versions of %s\n", deleted, basePath);
    }

    snprintf(response, sizeof(response), "OK\n");
    send(client_sock, response, strlen(response), 0);

    pthread_rwlock_unlock(&lock->rwlock);
    releaseFileLock(lock);
    return 0;
}
/*
 * utility.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#ifndef UTILITY_H
#define UTILITY_H

#include "config.h"
#include <pthread.h>

/**
 * @brief Per-file lock entry for reader-writer synchronization.
 */
typedef struct
{
    char path[512];
    pthread_rwlock_t rwlock;
    int refcount;
} FileLock;

extern FileLock lock_table[MAX_LOCKS];
extern pthread_mutex_t table_mutex;

/**
 * @brief Acquires a lock entry for the given path.
 *
 * Searches the lock table for an existing entry or claims an empty slot.
 * Increments the refcount to track active users.
 *
 * @param path File path to lock.
 * @return Pointer to FileLock on success, NULL if table is full.
 */
FileLock *acquireFileLock(const char *path);

/**
 * @brief Releases a previously acquired lock entry.
 *
 * Decrements the refcount. The slot can be reused when refcount reaches 0.
 *
 * @param lock Pointer to the FileLock to release.
 */
void releaseFileLock(FileLock *lock);

/**
 * @brief Finds the highest version number for a file.
 *
 * Scans the directory for files matching "basename.vN" pattern
 * and returns the maximum N found.
 *
 * @param basePath Base file path (without version suffix).
 * @return Highest version number, or 0 if no versions exist.
 */
int findLatestVersion(const char *basePath);

#endif
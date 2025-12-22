/*
 * write.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Handles WRITE request from a client.
 *
 * Acquires a write lock, checks disk space, creates a new version
 * of the file, and receives the data from the client.
 *
 * @param fileName Path to the file (relative to files/).
 * @param fileSize Expected number of bytes to receive.
 * @param client_sock Client socket descriptor.
 * @return 0 on success, -1 on failure.
 */
int writeFile(char *fileName, int fileSize, int client_sock);

/**
 * @brief Creates all directories in a given path.
 *
 * Iterates through the path and creates each directory component
 * if it does not already exist. Similar to `mkdir -p`.
 *
 * @param path The full file path (directories will be created for all parent components).
 * @return 0 on success.
 */
int createDirectories(char *path);
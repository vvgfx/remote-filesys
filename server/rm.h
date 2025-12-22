/*
 * rm.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Handles RM request from a client.
 *
 * For files, deletes all versions matching the base path.
 * For directories, performs recursive deletion using nftw().
 * Acquires a write lock before modifying.
 *
 * @param filename Path to the file or directory (relative to files/).
 * @param client_sock Client socket descriptor.
 * @return 0 on success, -1 on failure.
 */
int removeFile(char *filename, int client_sock);
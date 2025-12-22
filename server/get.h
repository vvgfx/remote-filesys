/*
 * get.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Handles GET request from a client.
 *
 * Finds the latest version of the requested file, acquires a read lock,
 * sends the file size in the handshake, then streams the file data
 * to the client.
 *
 * @param filename Path to the file (relative to files/).
 * @param client_sock Client socket descriptor.
 * @return 0 on success, -1 on failure.
 */
int getFile(char *filename, int client_sock);
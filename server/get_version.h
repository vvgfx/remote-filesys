/*
 * get_version.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Handles GET_VERSION request from a client.
 *
 * Retrieves a specific version of the requested file, acquires a read lock,
 * sends the file size in the handshake, then streams the file data
 * to the client.
 *
 * @param filename Path to the file (relative to files/).
 * @param version Version number to retrieve.
 * @param client_sock Client socket descriptor.
 * @return 0 on success, -1 on failure.
 */
int getFileVersion(char *filename, int version, int client_sock);
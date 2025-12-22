/*
 * ls.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Handles LS request from a client.
 *
 * Scans the directory for all versions of the requested file,
 * collects version numbers and timestamps, sorts them, and
 * sends the list back to the client.
 *
 * @param filename Path to the file (relative to files/).
 * @param client_sock Client socket descriptor.
 * @return 0 on success, -1 on failure.
 */
int listVersions(char *filename, int client_sock);
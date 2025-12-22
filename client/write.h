/*
 * write.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Uploads a local file to the remote server.
 *
 * Connects to the server, sends a WRITE request with the file size,
 * waits for READY, then streams the file data. The server stores
 * it as a new version.
 *
 * @param localFile Path to the local file (relative to files/).
 * @param remoteFile Path where the file will be stored on the server.
 * @return 0 on success, -1 on failure.
 */
int writeFile(char *localFile, char *remoteFile);
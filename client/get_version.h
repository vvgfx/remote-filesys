/*
 * get_version.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Retrieves a specific version of a file from the remote server.
 *
 * Connects to the server, sends a GET_VERSION request for the specified
 * remote file and version, receives the file data, and writes it to the
 * local filesystem under files/.
 *
 * @param remoteFile Path to the file on the remote server.
 * @param version Version string (e.g., "v1", "v2").
 * @param localFile Path where the file will be saved locally.
 * @return 0 on success, -1 on failure.
 */
int getFileVersion(char *remoteFile, char *version, char *localFile);

/*
 * get.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Retrieves a file from the remote server and saves it locally.
 *
 * Connects to the server, sends a GET request for the specified remote file,
 * receives the file data, and writes it to the local filesystem under files/.
 *
 * @param remoteFile Path to the file on the remote server.
 * @param localFile Path where the file will be saved locally.
 * @return 0 on success, -1 on failure.
 */
int getFile(char *remoteFile, char *localFile);
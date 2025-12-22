/*
 * rm.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Removes a file or directory from the remote server.
 *
 * Connects to the server and sends an RM request. For files, all
 * versions are deleted. For directories, recursive deletion is performed.
 *
 * @param remoteFile Path to the file or directory on the remote server.
 * @return 0 on success, -1 on failure.
 */
int removeFile(char *remoteFile);
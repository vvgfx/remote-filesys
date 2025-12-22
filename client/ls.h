/*
 * ls.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Lists all versions of a file on the remote server.
 *
 * Connects to the server, sends an LS request, and displays
 * all available versions with their timestamps.
 *
 * @param remoteFile Path to the file on the remote server.
 * @return 0 on success, -1 on failure.
 */
int listVersions(char *remoteFile);
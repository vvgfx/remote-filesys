/*
 * stop.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Sends a STOP command to the remote server.
 *
 * Connects to the server and sends a STOP request, causing
 * the server to shut down cleanly.
 *
 * @return 0 on success, -1 on failure.
 */
int stopServer();
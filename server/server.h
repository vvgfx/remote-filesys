/*
 * server.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Entry point for the RFS server.
 *
 * Initializes the socket, semaphore, and lock table. Listens for
 * incoming connections and spawns threads to handle each client.
 *
 * @return 0 on success, -1 on failure.
 */
int main(void);

/**
 * @brief Signal handler for SIGINT.
 *
 * Closes the socket, destroys the semaphore, and exits cleanly.
 *
 * @param sig Signal number (unused).
 */
void handle_sigint(int sig);

/**
 * @brief Thread handler for a connected client.
 *
 * Waits for a slot in the thread pool, receives the client's command,
 * dispatches to the appropriate handler, and cleans up on completion.
 * Supports WRITE, GET, GET_VERSION, RM, LS, and STOP commands.
 *
 * @param arg Pointer to ClientArgs containing socket and address info.
 * @return NULL
 */
void *clientHandler(void *arg);
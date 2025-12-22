/*
 * client.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Entry point for the RFS client.
 *
 * Parses command line arguments and dispatches to the appropriate
 * handler function (WRITE, GET, RM, LS, GET_VERSION).
 *
 * @param argc Argument count.
 * @param args Argument vector.
 * @return 0 on success, -1 on failure.
 */
int main(int argc, char *args[]);
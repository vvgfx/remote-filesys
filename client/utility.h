/*
 * utility.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#pragma once

/**
 * @brief Creates all directories in a given path.
 *
 * Iterates through the path and creates each directory component
 * if it does not already exist. Similar to `mkdir -p`.
 *
 * @param path The full file path (directories will be created for all parent components).
 * @return 0 on success.
 */
int createDirectories(char *path);
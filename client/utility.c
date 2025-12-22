/*
 * utility.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 */

#include "utility.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// Note: claude helped me with this function, but what it basically does every loop is it creates the directory, then
// concats the remaining part of the string, and goes on.
int createDirectories(char *path)
{
    char temp[512];
    char *p = NULL;

    snprintf(temp, sizeof(temp), "%s", path);

    size_t len = strlen(temp);
    if (temp[len - 1] == '/')
        temp[len - 1] = '\0';

    // Create each directory component
    for (p = temp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            mkdir(temp, 0755);
            *p = '/';
        }
    }

    return 0;
}
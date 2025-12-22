/*
 * client.c / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 *
 * TCP Socket Client for Remote File System.
 * Supports: WRITE, GET, RM, LS, GET_VERSION, STOP
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "client.h"
#include "get.h"
#include "get_version.h"
#include "ls.h"
#include "rm.h"
#include "stop.h"
#include "write.h"

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        printf("Invalid number of arguments.\n");
        return -1;
    }

    if (strcmp(args[1], "WRITE") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: %s WRITE localfile [remotefile]\n", args[0]);
            return -1;
        }
        char *localFile = args[2];
        char *remoteFile = (argc >= 4) ? args[3] : args[2];
        return writeFile(localFile, remoteFile);
    }
    else if (strcmp(args[1], "GET") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: %s GET remotefile [localfile]\n", args[0]);
            return -1;
        }
        char *remoteFile = args[2];
        char *localFile = (argc >= 4) ? args[3] : args[2];
        return getFile(remoteFile, localFile);
    }
    else if (strcmp(args[1], "RM") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: %s RM remotepath\n", args[0]);
            return -1;
        }
        char *remoteFile = args[2];
        return removeFile(remoteFile);
    }
    else if (strcmp(args[1], "LS") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: %s LS remotefile\n", args[0]);
            return -1;
        }
        char *remoteFile = args[2];
        return listVersions(remoteFile);
    }
    else if (strcmp(args[1], "GET_VERSION") == 0)
    {
        if (argc < 4)
        {
            printf("Usage: %s GET_VERSION remotefile version [localfile]\n", args[0]);
            return -1;
        }
        char *remoteFile = args[2];
        char *version = args[3];
        char *localFile = (argc >= 5) ? args[4] : args[2];
        return getFileVersion(remoteFile, version, localFile);
    }
    else if (strcmp(args[1], "STOP") == 0)
    {
        return stopServer();
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", args[1]);
        return -1;
    }
}
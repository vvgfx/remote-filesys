/*
 * config.h / Practicum - 2
 *
 * Vishnu Vardan S / CS5600 / Northeastern University
 * Fall 2025 / Dec 4, 2025
 *
 * Shared configuration for client and server.
 * Values must match for client-server communication.
 */

#ifndef CONFIG_H
#define CONFIG_H

#define SERVER_IP "127.0.0.1" // this must sync with client. Ugly solution, but works for now.
#define SERVER_PORT 2000

#define HANDSHAKE_SIZE 2000
#define BUFFER_SIZE 4096

#endif
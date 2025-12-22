## Overview

A multi-threaded remote file system implementation over TCP sockets. The client (`rfs`) connects to the server, executes a single command, and disconnects. The server runs continuously, handling multiple concurrent clients with per-file reader-writer locks and automatic file versioning.

## Features

- **WRITE**: Upload files to the server with automatic versioning
- **GET**: Download the latest version of a file
- **GET_VERSION**: Download a specific version of a file
- **LS**: List all versions of a file with timestamps
- **RM**: Delete a file (all versions) or directory (recursive)
- **STOP**: Cleanly shut down the server

## Project Structure

```
root/
├── client/
│   ├── config.h          # IP/port configuration
│   ├── client.c/h        # Entry point, command dispatch
│   ├── write.c/h         # WRITE command
│   ├── get.c/h           # GET command
│   ├── get_version.c/h   # GET_VERSION command
│   ├── ls.c/h            # LS command
│   ├── rm.c/h            # RM command
│   ├── utility.c/h       # Directory creation
│   ├── files/            # Local file storage
│   └── Makefile
├── server/
│   ├── config.h          # IP/port configuration (must match client)
│   ├── server.c/h        # Entry point, thread management
│   ├── write.c/h         # WRITE handler with disk space check
│   ├── get.c/h           # GET handler
│   ├── get_version.c/h   # GET_VERSION handler
│   ├── ls.c/h            # LS handler
│   ├── rm.c/h            # RM handler with nftw recursive delete
│   ├── utility.c/h       # Lock table, version lookup
│   ├── files/            # Remote file storage
│   └── Makefile
└── test.sh               # Automated test suite
```

## Building

```bash
cd client && make
cd ../server && make
```

Both Makefiles compile with `-Wall -Wextra`. The server links with `-pthread`.

## Configuration

IP address and port are defined in `config.h` in both client and server directories. These must be kept in sync. For your convenience, I have kept them at localhost. If you wish to move to a normal setup, you have to run `ifconfig` on the server pc, retrieve the IP, then save it in both config files in client and server.

```c
#define SERVER_IP "10.0.0.1"
#define SERVER_PORT 2000
```

## Usage

Start the server:
```bash
cd server
./server
```

Run client commands:
```bash
cd client
./rfs WRITE local/path.txt remote/path.txt    # Upload file
./rfs WRITE local/path.txt                     # Remote path defaults to local path
./rfs GET remote/path.txt local/path.txt       # Download latest version
./rfs GET remote/path.txt                      # Local path defaults to remote path
./rfs GET_VERSION remote/path.txt v2 local.txt # Download specific version
./rfs LS remote/path.txt                       # List all versions
./rfs RM remote/path.txt                       # Delete all versions
./rfs RM remote/directory                      # Delete directory recursively
./rfs STOP                                     # Shut down server
```

All local paths are relative to `files/` directory.

## Protocol

Communication uses a simple text-based protocol over TCP.

**WRITE**:
1. Client sends: `WRITE <remote_path> <file_size>`
2. Server responds: `READY` or `ERROR <reason>`
3. Client streams file bytes
4. Server responds: `SUCCESS`

**GET / GET_VERSION**:
1. Client sends: `GET <remote_path>` or `GET_VERSION <remote_path> v<N>`
2. Server responds: `OK <file_size>\n` or `ERROR <reason>\n`
3. Server streams file bytes

**LS**:
1. Client sends: `LS <remote_path>`
2. Server responds:
   ```
   OK <count>
   v1 <timestamp>
   v2 <timestamp>
   ...
   ```

**RM**:
1. Client sends: `RM <remote_path>`
2. Server responds: `OK\n` or `ERROR <reason>\n`

**STOP**:
1. Client sends: `STOP`
2. Server closes socket and exits

## Design

### Threading Model

The server uses a thread-per-client model with a counting semaphore limiting concurrent connections to 8 (configurable via `MAX_THREADS`). When a client connects:

1. Main thread calls `accept()` and spawns a handler thread
2. Handler thread calls `sem_wait()` to acquire a slot
3. Handler processes the command
4. Handler calls `sem_post()` and exits

Clients are accepted immediately but block on the semaphore if all slots are occupied. This provides backpressure without dropping connections.

### File Locking

Per-file reader-writer locks prevent data corruption from concurrent access. The lock table maps file paths to `pthread_rwlock_t` with reference counting.

- **GET, GET_VERSION, LS**: Acquire read lock (multiple readers allowed)
- **WRITE, RM**: Acquire write lock (exclusive access)

The lock granularity is per-file, not global, allowing parallel operations on different files.

### Versioning

Files are stored with version suffixes: `filename.v1`, `filename.v2`, etc. On WRITE, the server scans for the highest existing version and creates `v(N+1)`. GET returns the latest version. RM deletes all versions matching the base path.

Version lookup uses `readdir()` to scan the directory and extract version numbers from filenames matching the pattern `<basename>.v<N>`.

### Storage Checks

Before accepting file data, the server checks available disk space using `statvfs()`. If insufficient space exists, it responds with `ERROR insufficient storage` before the client begins transmission.

## Testing

Run the automated test suite from the root directory:

```bash
chmod +x test.sh
./test.sh
```

The test suite covers:
- Server not running (connection refused)
- Invalid parameters and unknown commands
- WRITE: small files, large files, empty files, versioning, nonexistent source
- GET: existing files, nonexistent files, content verification
- LS: version listing, timestamps
- GET_VERSION: specific version retrieval, invalid versions
- RM: files, directories, nonexistent paths
- Concurrent writes to different files
- Concurrent writes to same file (lock contention)

Tests are performed sequentially with the server running in the background. The server is automatically started and stopped by the script.

## References
please refer to the references.md file for all references.

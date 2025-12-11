/*
 * Copyright (c) University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef CREATOR_UTILS_H
#define CREATOR_UTILS_H

/**
 * @file
 * @ingroup fd-net-device
 * Logging macros, and socket connection helper, for use in non-ns-3 code.
 */

#include <cstdint>
#include <cstring>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

/**
 * Control logging functions in external programs.
 */
extern bool gVerbose;

/**
 * Write a message to `std::cout`
 * @param msg The message to log
 */
#define LOG(msg)                                                                                   \
    if (gVerbose)                                                                                  \
    {                                                                                              \
        std::cout << __FUNCTION__ << "(): " << msg << std::endl;                                   \
    }

/**
 * Abort on fatal errors.
 * @param msg The message explaining the fatal error
 * @param printErrno Whether to also print the `errno` and corresponding string
 */
#define ABORT(msg, printErrno)                                                                     \
    std::cout << __FILE__ << ": fatal error at line " << __LINE__ << ": " << __FUNCTION__          \
              << "(): " << msg << std::endl;                                                       \
    if (printErrno)                                                                                \
    {                                                                                              \
        std::cout << "    errno = " << errno << " (" << strerror(errno) << ")" << std::endl;       \
    }                                                                                              \
    exit(-1);

/**
 * Abort on a condition
 * @param cond The condition, abort if @c true
 * @param msg The message explaining the fatal error
 * @param printErrno Whether to also print the `errno` and corresponding string
 */
#define ABORT_IF(cond, msg, printErrno)                                                            \
    if (cond)                                                                                      \
    {                                                                                              \
        ABORT(msg, printErrno);                                                                    \
    }

/**
 * SendSocket magic number
 */
constexpr uint32_t EMU_MAGIC{65867};

/**
 * TapFdNetDevice magic number
 */
constexpr uint32_t TAP_MAGIC{95549};

/**
 * @brief Send the file descriptor back to the code that invoked the creation.
 *
 * @param path The socket address information from the Unix socket we use
 * to send the created socket back to.
 * @param fd The file descriptor we're going to send.
 * @param magic_number A verification number to verify the caller is talking to the
 * right process.
 */
void SendSocket(const char* path, int fd, const int magic_number);

#endif /* CREATOR_UTILS_DEVICE_H */

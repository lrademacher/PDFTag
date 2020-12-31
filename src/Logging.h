#ifndef __LOGGING_H
#define __LOGGING_H

/* Includes */
#include <stdio.h>

/* Defines/Macros */
#define LOG_INFO (0)
#define LOG_WARN (1)
#define LOG_ERR  (2)

#define LOG_LEVEL LOG_INFO

// TODO: Add filtering
#define LOG(level, ...) printf((level==LOG_INFO) ? "INFO: " : (level==LOG_WARN) ? "WARNING: " : (level==LOG_ERR) ? "ERROR: " : ""); printf(__VA_ARGS__)

#endif
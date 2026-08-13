#pragma once

#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>

#include <wpeframework/helpers/UtilsLogging.h>

#define DSLOG_INFO(fmt, ...) do { fprintf(stderr, "[DSMGR] [%d] INFO [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define DSLOG_WARN(fmt, ...) do { fprintf(stderr, "[DSMGR] [%d] WARN [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)
#define DSLOG_ERR(fmt, ...) do { fprintf(stderr, "[DSMGR] [%d] ERROR [%s:%d] %s: " fmt "\n", (int)syscall(SYS_gettid), WPEFramework::Core::FileNameOnly(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__); fflush(stderr); } while (0)


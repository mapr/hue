/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef WINDOWS_NETWORK_H
#define WINDOWS_NETWORK_H

#ifdef __WINDOWS__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <process.h>
#include <fcntl.h>
#include <pthread.h>

struct iovec 
{
  void *iov_base;
  size_t iov_len;
};

struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

struct timespec
{
  time_t tv_sec; // Seconds since 00:00:00 GMT, 1 Jan 1970
  time_t tv_nsec; // Additional nanoseconds since tv_sec
};

struct ifaddrs
{
  struct ifaddrs  *ifa_next;
  char            *ifa_name; // not filled
  unsigned int    ifa_flags; 
  struct sockaddr *ifa_addr; // address of interface
  struct sockaddr *ifa_netmask; // not filled
  union {
    struct sockaddr *ifu_broadaddr;  // not filled
    struct sockaddr *ifu_dstaddr;    // not filled
  } ifa_ifu;
#define           ifa_broadaddr ifa_ifu.ifu_broadaddr
#define           ifa_dstaddr   ifa_ifu.ifu_dstaddr
  void            *ifa_data;         // not filled

  // should not be used outside
  struct sockaddr _addr;
};

// Definition of a gettimeofday function
int gettimeofday(struct timeval *tv, struct timezone *tz);

int getifaddrs(struct ifaddrs **ifap);

void freeifaddrs(struct ifaddrs *ifa);

unsigned int sleep(unsigned int secs);
unsigned int usleep(unsigned int usecs);

#endif // __WINDOWS__
#endif

/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef FS_NONLINUXSUPPORT_H
#define FS_NONLINUXSUPPORT_H

#if defined(_WIN32) || defined(_WIN64)
#define __WINDOWS__
#endif

#include <pthread.h>
#include <errno.h>
#include <sys/types.h>

#ifdef __WINDOWS__
# include <string.h>
const char *win_strerror(int errnum);
# define strerror win_strerror
# include "common/windows/usergroup.h"
# include "common/windows/network.h"
# include <WinBase.h>

# ifndef USE_STL
# define USE_STL
# endif

// All mapping macros
# define strtok_r strtok_s
# define snprintf _snprintf
# define strcasecmp _stricmp
# define strncasecmp _strnicmp
# define PRId64 "lld"
# define srand48 srand
# define lrand48 rand

// sysconf related stuff
# define _SC_GETPW_R_SIZE_MAX 1
# define _SC_GETGR_R_SIZE_MAX 2

struct tm *localtime_r(const time_t *timep, struct tm *result);
#ifdef __cplusplus
extern "C" {
#endif
int init_spoofeduser(const char *username, int uid, const char *groupname, int gid);
void cleanup_spoofeduser();
#ifdef __cplusplus
}
#endif

// No shmem support in windows
#define USE_SHM 0

// No signal support in Windows
# define HAS_SIGNAL_SUPPORT 0
# define SIGPIPE  13

#else
# include <sys/socket.h>
# include <sys/uio.h>
# include <unistd.h>
# include <grp.h>
# include <signal.h>

// dont compile with shmem for Mac. compiling with shmem, requires turning it
// off in core-site.xml for Mac clients.
#ifdef __APPLE__
#define USE_SHM 0
#else
#define USE_SHM 1
#endif

#define HAS_SIGNAL_SUPPORT 1
#endif // __WINDOWS__

#ifndef EREMOTEIO
#define EREMOTEIO 121
#endif

#ifndef ENAVAIL
#define ENAVAIL   119
#endif

#ifndef EBADE
#define EBADE     52
#endif

#ifndef EBADFD
#define EBADFD    77
#endif

#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE 131
#endif

#ifndef HOST_NAME_MAX
#ifndef __WINDOWS__
  // from linux: /usr/include/bits/local_lim.h
# define HOST_NAME_MAX _SC_HOST_NAME_MAX 
# else
  // from linux: /usr/include/bits/local_lim.h
# define HOST_NAME_MAX 64
#endif  // __WINDOWS__
#endif

#ifndef NAME_MAX
  // from linux: /usr/include/linux/limits.h
#define NAME_MAX         255
#endif

#ifndef PATH_MAX
  // from linux: /usr/include/linux/limits.h
#define PATH_MAX         4096
#endif

#ifndef IOV_MAX
  // from linux: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
#define IOV_MAX 1024
#endif

#ifdef USE_STL
# define DECLARE_ARRAY(type, nelems, var) \
  std::vector<type> s_vector_##var(nelems); \
  std::vector<type>::pointer var = &(s_vector_##var[0])
#else
# define DECLARE_ARRAY(type, nelems, var) type var[nelems]
#endif

#ifdef __APPLE__
  // Mac has no pthread_yield
#define pthread_yield pthread_yield_np

#ifdef __cplusplus
extern "C" {
#endif

// Mac has no pthread_spinlock
typedef pthread_mutex_t pthread_spinlock_t;

inline int
pthread_spin_init (pthread_spinlock_t *lock, int pshared)
{
  return pthread_mutex_init (lock, NULL);
}

inline int
pthread_spin_lock (pthread_spinlock_t *lock)
{
  return pthread_mutex_lock (lock);
}

inline int
pthread_spin_unlock (pthread_spinlock_t *lock)
{
  return pthread_mutex_unlock (lock);
}

#ifdef __cplusplus
}
#endif
#endif // __APPLE__

// Mac: No TCP_CORK (TCP_NOPUSH is buggy)

inline int
Cork (int sockfd, int level)
{
#ifdef TCP_CORK
  int set=1;
  return setsockopt (sockfd, level, TCP_CORK, &set, sizeof(set));
#else
  return 0;
#endif
}

inline int
UnCork (int sockfd, int level)
{
#ifdef TCP_CORK
  int set=0;
  return setsockopt (sockfd, level, TCP_CORK, &set, sizeof(set));
#else
  return 0;
#endif
}

inline int
EnableQuickAck(int sockfd, int level)
{
#ifdef TCP_QUICKACK
  int i = 1;
  return setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, (void *)&i, sizeof(i));
#else
  return 0;
#endif
}

inline int
GetGroupList (const char *username, gid_t gid, gid_t *grps, int *ngroups)
{
#ifdef __APPLE__
  return getgrouplist(username, gid, (int *)grps, ngroups);
#else
  return getgrouplist(username, gid, grps, ngroups);
#endif
}

inline int
IgnoreSignal(int signum)
{
#if HAS_SIGNAL_SUPPORT
  struct sigaction act;
  act.sa_handler = SIG_IGN;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  return sigaction(signum, &act, NULL);
#else
  return 0;
#endif
}

#ifdef __GNUC__
#define atomic_add32(ptr, val) __sync_fetch_and_add ((ptr), (val))
#define atomic_sub32(ptr, val) __sync_fetch_and_sub ((ptr), (val))
#define atomic_add64(ptr, val) __sync_fetch_and_add ((ptr), (val))
#define atomic_sub64(ptr, val) __sync_fetch_and_sub ((ptr), (val))
#elif defined (__WINDOWS__)
#define atomic_add32(ptr, val) InterlockedExchangeAdd ((ptr), (val))
#define atomic_sub32(ptr, val) InterlockedExchangeAdd ((ptr), -(val)) 
#define atomic_add64(ptr, val) InterlockedExchangeAdd64 ((long long *)(ptr), (val))
#define atomic_sub64(ptr, val) InterlockedExchangeAdd64 ((long long *)(ptr), -(val))
#else
#error "Need to port atomic_add32 on this platform"
#endif

#endif // FS_NONLINUXSUPPORT_H

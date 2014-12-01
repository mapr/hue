/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef WINDOWS_USERGROUP_H
#define WINDOWS_USERGROUP_H

#ifdef __WINDOWS__

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h> // for assert
#include <string.h>

typedef int uid_t;
typedef int gid_t;

struct passwd {
	char *pw_name;
	uid_t pw_uid;
	gid_t pw_gid;
};

struct group {
	char *gr_name;
	gid_t gr_gid;
};

int getgrouplist(const char *username, gid_t gid, gid_t *grps, int *ngroups);

struct passwd * getpwnam(const char *username);

struct group * getgrnam(const char *name);

struct passwd * getpwuid(uid_t uid);

int
getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);

int
getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);

int
getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen, struct group **result);

int
getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen, struct group **result);

struct group * getgrgid(gid_t gid);

int getgroups(int size, gid_t list[]);

uid_t getuid();

uid_t geteuid();

gid_t getgid();

gid_t getegid();

int setuid(uid_t uid);

long sysconf(int name);

#ifdef __cplusplus
}
#endif

#endif // __WINDOWS__
#endif

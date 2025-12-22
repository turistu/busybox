/*
 * Copyright 2012, Denys Vlasenko
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
//kbuild:lib-y += missing_syscalls.o

#include "libbb.h"

#if defined(ANDROID) || defined(__ANDROID__)
# include <sys/syscall.h>
pid_t getsid(pid_t pid)
{
	return syscall(__NR_getsid, pid);
}

int sethostname(const char *name, size_t len)
{
	return syscall(__NR_sethostname, name, len);
}

int pivot_root(const char *new_root, const char *put_old)
{
	return syscall(__NR_pivot_root, new_root, put_old);
}

# if __ANDROID_API__ < 21
int tcdrain(int fd)
{
	return ioctl(fd, TCSBRK, 1);
}
# endif
#endif

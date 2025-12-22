//kbuild:ifneq (,$(findstring android, $(CROSS_COMPILE)))
//kbuild:lib-y += android.o
//kbuild:endif
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <mntent.h>
#include <termios.h>
#include "platform.h"

#ifdef OLD_ANDROID
#include <net/if_ether.h>
#else
#include <netinet/if_ether.h>
#endif

static int usershell_done;
void setusershell(void) { usershell_done = 0; }
void endusershell(void) { usershell_done = 2; }
char *getusershell(void){
	switch(usershell_done++){
	case 0:	return (char*)"/system/bin/sh";
	case 1: return (char*)"/bin/sh";
	default: return NULL;
	}
}
int sigtimedwait(const sigset_t *set, siginfo_t *info,
	const struct timespec *timeout)
{
	return syscall(__NR_rt_sigtimedwait, set, info, timeout);
}
int sigisemptyset(sigset_t *set)
{
	int i, *p;
	for(i = 0, p = (int*)set; i < sizeof*set/sizeof*p; i++)
		if(*p) return 0;
	return 1;
}
FILE *setmntent(const char *fn, const char *type)
{
	return fopen(fn, type);
}
int endmntent(FILE *fp)
{
	return fclose(fp);
}
static int is8(char c) { return c >= '0' && c <= '7'; }
static int lsplit(char *b, char **f, int n)
{
	int i; char *d;
	for(d = b, i = 0; i < n; ){
		while(isspace(*b)) b++;
		if(*b == '#') break;
		f[i++] = d;
		for(;;)
			if(*b == '\\' && is8(b[1]) && is8(b[2]) && is8(b[3])){
				*d++ = strtol(b + 1, 0, 8); b += 4;
			}else if(isspace(*b)){
				*d++ = '\0'; b++; break;
			}else if(*b)
				*d++ = *b++;
			else
				goto out;
	}
out:
	return i;
}
struct mntent *getmntent_r(FILE* fp, struct mntent* m, char *b, int z)
{
	char *f[6] = { 
		/* fuck the worthless -Wwrite-strings */
		(char*)"", (char*)"", (char*)"auto",
		(char*)"defaults", (char*)"0", (char*)"0"
	};
	for(;;){
		if(!fgets(b, z, fp)) return 0;
		if(lsplit(b, f, 6) > 1) break;
	}
	m->mnt_fsname = f[0]; m->mnt_dir = f[1];
	m->mnt_type = f[2]; m->mnt_opts = f[3];
	m->mnt_freq = atoi(f[4]); m->mnt_passno = atoi(f[5]);
	return m;
}
struct mntent *getmntent(FILE* fp) {
	static char *b;
	struct mntent m;
	if(!b && !(b = malloc(4096))) return 0;
	return getmntent_r(fp, &m, b, 4096);
}
#ifdef OLD_ANDROID
int cfsetspeed(struct termios *ts, speed_t sp);
int cfsetspeed(struct termios *ts, speed_t sp) {
	cfsetispeed(ts, sp);
	cfsetospeed(ts, sp);
	return 0;
}
#endif
int sysinfo(void *v) {
	return syscall(__NR_sysinfo, v);
}

/*
 * Copyright (C) 2010 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/if_ether.h>
#include <ctype.h>

static inline int
xdigit (char c) {
    unsigned d;
    d = (unsigned)(c-'0');
    if (d < 10) return (int)d;
    d = (unsigned)(c-'a');
    if (d < 6) return (int)(10+d);
    d = (unsigned)(c-'A');
    if (d < 6) return (int)(10+d);
    return -1;
}

/*
 * Convert Ethernet address in the standard hex-digits-and-colons to binary
 * representation.
 * Re-entrant version (GNU extensions)
 */
struct ether_addr *
ether_aton_r (const char *asc, struct ether_addr * addr)
{
    int i, val0, val1;
    for (i = 0; i < ETHER_ADDR_LEN; ++i) {
        val0 = xdigit(*asc);
        asc++;
        if (val0 < 0)
            return NULL;

        val1 = xdigit(*asc);
        asc++;
        if (val1 < 0)
            return NULL;

        addr->ether_addr_octet[i] = (u_int8_t)((val0 << 4) + val1);

        if (i < ETHER_ADDR_LEN - 1) {
            if (*asc != ':')
                return NULL;
            asc++;
        }
    }
    if (*asc != '\0')
        return NULL;
    return addr;
}
/*
 * Copyright (C) 2010 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <sys/types.h>
#include <netinet/if_ether.h>

/*
 * Convert Ethernet address to standard hex-digits-and-colons printable form.
 * Re-entrant version (GNU extensions).
 */
char *
ether_ntoa_r (const struct ether_addr *addr, char * buf)
{
    snprintf(buf, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
            addr->ether_addr_octet[0], addr->ether_addr_octet[1],
            addr->ether_addr_octet[2], addr->ether_addr_octet[3],
            addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
    return buf;
}

/*
 * Convert Ethernet address to standard hex-digits-and-colons printable form.
 */
char *
ether_ntoa (const struct ether_addr *addr)
{
    static char buf[18];
    return ether_ntoa_r(addr, buf);
}

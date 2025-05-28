#ifndef _DOOM_NET_BASE_H_
#define _DOOM_NET_BASE_H_

// it is used for IP validation (so it knows which node it is)
// easiest is to just use the reference implementation
struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct sockaddr {
    short     sa_family;      /* Address family */
    char            sa_data[];      /* Socket address */
};

#define AF_INET		2	/* Internet IP Protocol 	*/

#endif

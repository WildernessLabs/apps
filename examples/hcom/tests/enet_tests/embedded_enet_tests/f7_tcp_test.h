#ifndef __APPS_EXAMPLES_TCP_F7_TEST_H
#define __APPS_EXAMPLES_TCP_F7_TEST_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <arpa/inet.h>

#ifdef TCP_F7_TEST_HOST
#else
# include <debug.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef TCP_F7_TEST_HOST
   /* HTONS/L macros are unique to uIP */

#  define HTONS(a)       htons(a)
#  define HTONL(a)       htonl(a)

   /* Have SO_LINGER */

#  define TCP_F7_TEST_HAVE_SOLINGER 1

#else
#  ifdef CONFIG_NET_SOLINGER
#    define TCP_F7_TEST_HAVE_SOLINGER 1
#  else
#    undef TCP_F7_TEST_HAVE_SOLINGER
#  endif
#endif /* TCP_F7_TEST_HOST */

#ifdef CONFIG_EXAMPLES_TCP_F7_TEST_IPv6
#  define AF_INETX AF_INET6
#  define PF_INETX PF_INET6
#else
#  define AF_INETX AF_INET
#  define PF_INETX PF_INET
#endif

#ifndef CONFIG_EXAMPLES_TCP_F7_TEST_SERVER_PORTNO
#  define CONFIG_EXAMPLES_TCP_F7_TEST_SERVER_PORTNO 5471
#endif

#ifdef CONFIG_EXAMPLES_TCP_F7_TEST_SENDSIZE
#  define SENDSIZE CONFIG_EXAMPLES_TCP_F7_TEST_SENDSIZE
#else
#  define SENDSIZE 4096
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef CONFIG_EXAMPLES_TCP_F7_TEST_IPv6
uint16_t g_tcpblasterserver_ipv6[8];
#else
uint32_t g_f7_tcp_server_ipv4;
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef CONFIG_EXAMPLES_TCP_F7_TEST_INIT
void tcpblaster_initialize(void);
#endif

extern void f7_tcp_client_entry(FAR void *arg);
extern void f7_tcp_server_entry(FAR void *arg);

#endif /* __APPS_EXAMPLES_TCP_F7_TEST_H */

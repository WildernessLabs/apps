
/****************************************************************************
 * examples\hcom\tests\enet_tests\embedded_enet_tests\first_enet_test.c
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>

#include "f7_tcp_test.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/
  uint32_t g_f7_tcp_server_ipv4 = HTONL(CONFIG_EXAMPLES_TCP_F7_TEST_SERVER_PORTNO);

// Called by developer 
void f7_tcp_test_app(void)
{
  int ret;

      /* Convert the <server-addr> argument into a binary address */

// #ifdef CONFIG_EXAMPLES_TCPBLASTER_IPv6
//       ret = inet_pton(AF_INET6, argv[1], g_f7_tcp_server_ipv6);
// #else
// Get IP Address from meadow.cfg
  ret = inet_pton(AF_INET, "192.168.2.123", &g_f7_tcp_server_ipv4);
// #endif
  if (ret < 0)
  {
    fprintf(stderr, "ERROR: <server-addr> is invalid\n");
  }

  // Create 2 pthreads to do the work
  pthread_t thread;
  pthread_attr_t attr;
  struct sched_param param;

  param.sched_priority = 130;
  (void)pthread_attr_init(&attr);
  (void)pthread_attr_setschedparam(&attr, &param);
  (void)pthread_attr_setstacksize(&attr, 1204);

  ret = pthread_create(&thread, &attr, f7_tcp_server_entry, NULL);
  if (ret < 0)
  {
    syslog(1, "%s@%d-Thread create 'f7_tcp_server' error:%d\n",
            __FILE__, __LINE__, ret);
    return;
  }

  param.sched_priority = 130;
  (void)pthread_attr_init(&attr);
  (void)pthread_attr_setschedparam(&attr, &param);
  (void)pthread_attr_setstacksize(&attr, 1024);

  ret = pthread_create(&thread, &attr, f7_tcp_client_entry, NULL);
  if (ret < 0)
  {
    syslog(1, "%s@%d-Thread create 'f7_tcp_client' error:%d\n",
            __FILE__, __LINE__, ret);
    return;
  }

  return;
}

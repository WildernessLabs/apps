/****************************************************************************
 * examples/espcptest/espcptest_main.c
 *
 *   Copyright (C) 2020 Wilderness Labs
 *   Author: Mark Stevens
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <debug.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "defines.h"
#include "secrets.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/
#define usrsocktest_dbg(...) ((void)0)

/****************************************************************************
 * Private Data
 ****************************************************************************/
static struct
{
  unsigned int ok;
  unsigned int failed;

  unsigned int nchecks;
} overall;
/****************************************************************************
 * Public Data
 ****************************************************************************/
int usrsocktest_endp_malloc_cnt = 0;
int usrsocktest_dcmd_malloc_cnt = 0;
bool usrsocktest_test_failed = false;
bool espcptest_test_failed;

/****************************************************************************
 * Private Functions
 ****************************************************************************/



static void get_mallinfo(struct mallinfo *mem)
{
#ifdef CONFIG_CAN_PASS_STRUCTS
  *mem = mallinfo();
#else
  (void)mallinfo(mem);
#endif
}

static void print_mallinfo(const struct mallinfo *mem, const char *title)
{
  if (title)
    printf("%s:\n", title);
  printf("       %11s%11s%11s%11s\n", "total", "used", "free", "largest");
  printf("Mem:   %11d%11d%11d%11d\n",
         mem->arena, mem->uordblks, mem->fordblks, mem->mxordblk);
}

static void utest_assert_print_head(FAR const char *func, const int line,
                                    FAR const char *check_str)
{
  printf("\t[TEST ASSERT FAILED!]\n"
         "\t\tIn function \"%s\":\n"
         "\t\tline %d: Assertion `%s' failed.\n", func, line, check_str);
}


static void run_tests(FAR const char *name, void (CODE *test_fn)(void))
{
  printf("Testing group \"%s\" =>\n", name);
  fflush(stdout);
  fflush(stderr);

  usrsocktest_test_failed = false;
  test_fn();
  if (!usrsocktest_test_failed)
    {
      printf("\tGroup \"%s\": [OK]\n", name);
      overall.ok++;
    }
  else
    {
      printf("\tGroup \"%s\": [FAILED]\n", name);
      overall.failed++;
    }

  fflush(stdout);
  fflush(stderr);
}

/****************************************************************************
 * Name: runAllTests
 *
 * Description:
 *   Sequentially runs all included test groups
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   None
 *
 * Assumptions/Limitations:
 *   None
 *
 ****************************************************************************/
static void runAllTests(void)
{
  RUN_TEST_GROUP(GenericList);
  RUN_TEST_GROUP(EspcpCoprocessor);
  RUN_TEST_GROUP(EspcpQueue);
  RUN_TEST_GROUP(EspcpMessageDispatcher);
  RUN_TEST_GROUP(EspcpThread);
}


void getGoogleHomePage(void)
{
  int result = 0;
  espcp_init();
  espcp_ip_information_t *ip = espcp_start_wifi(NETWORK_NAME, NETWORK_PASSWORD);

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = espcp_address_family_type_af_inet;
  hints.ai_socktype = espcp_socket_protocol_type_sock_stream;
  hints.ai_addrlen = 0;
  hints.ai_addr = NULL;
  hints.ai_canonname = "";
  struct addrinfo *res;

  result = espcp_getaddrinfo("www.google.com", "80", &hints, &res);
  if (result != 0)
  {
    return;
  }

  int socket_handle = espcp_socket(res->ai_family, res->ai_socktype, 0);
  if (socket_handle < 0)
  {
    return;
  }

  if (espcp_connect(socket_handle, res->ai_addr, res->ai_addrlen) != 0)
  {
    return;
  }

  espcp_freeaddrinfo(res);

  #define WEB_SERVER "www.google.com"
  #define WEB_PATH "/"
  #define WEB_PORT "80"
  #define REQUEST "GET " WEB_PATH " HTTP/1.0\r\nHost: " WEB_SERVER ":" WEB_PORT  "\r\n" \
                  "User-Agent: Meadow\r\n\r\n"

  int amount_written = espcp_write(socket_handle, REQUEST, strlen(REQUEST));
  if (amount_written < 0)
  {
    return;
  }

  /*
    //
    //    setsockopt
    //
    DebugHelpers.DebugHelpers.LogMessage(true, "Setting socket options (setsockopt)");
    var tv = new TimeVal();
    tv.TvSec = 5;
    tv.TvUsec = 0;
    if (_f7MicroCoprocessor.SetSockOpt(socketHandle, SocketLevelType.SolSocket, SocketOptionType.SoRcvTimeO, tv, 0) != 0)
    {
        _f7MicroCoprocessor.Close(socketHandle);
        DebugHelpers.DebugHelpers.LogMessage(true, $"Exiting test");
        return;
    }
    DebugHelpers.DebugHelpers.LogMessage(true, "    Timeout set for socket.");
  
    //
    //    write
    //
    string uri = "/";
    string host = "www.google.com";
    string request = $"GET {uri} HTTP/1.0\r\nHost: {host}\r\nUser-Agent: esp-idf/3.3 esp32\r\n\r\n";
    byte[] writeBuffer = System.Text.Encoding.UTF8.GetBytes(request);
    DebugHelpers.DebugHelpers.LogMessage(true, "Sending request to server (write)");
    var amountWritten = _f7MicroCoprocessor.Write(socketHandle, writeBuffer, writeBuffer.Length);
    DebugHelpers.DebugHelpers.LogMessage(true, $"    {amountWritten} bytes sent to server");
    //
    //    TODO: need to work out how to deal with errno.
    //
    if (amountWritten < 0)
    {
        _f7MicroCoprocessor.Close(socketHandle);
        DebugHelpers.DebugHelpers.LogMessage(true, $"Exiting test");
        return;
    }
    DebugHelpers.DebugHelpers.LogMessage(true, "    Request written to server.");
    //
    //    read
    //
    const int BUFFER_SIZE = 1024;
    var readBuffer = new byte[BUFFER_SIZE];
    int totalNumberOfBytes = 0;
    int amountRead = 0;
    string result = "";
    bool firstBlock = true;
    DebugHelpers.DebugHelpers.LogMessage(true, "Reading data from server (read)");
    do
    {
        Array.Clear(readBuffer, 0, BUFFER_SIZE);
        amountRead = _f7MicroCoprocessor.Read(socketHandle, readBuffer, BUFFER_SIZE);
        DebugHelpers.DebugHelpers.LogMessage(true, $"    {amountRead} bytes read from server");
        if (amountRead > 0)
        {
            result += System.Text.Encoding.UTF8.GetString(readBuffer);
            totalNumberOfBytes += amountRead;
            if (firstBlock)
            {
                var lines = result.Split(new[] {'\r', '\n'}, StringSplitOptions.RemoveEmptyEntries);
                var numberLinesToDisplay = lines.Length > 5 ? 5 : lines.Length;
                DebugHelpers.DebugHelpers.LogMessage(true, $"    First {numberLinesToDisplay} lines:");
                for (int index = 0; index < numberLinesToDisplay; index++)
                {
                    DebugHelpers.DebugHelpers.LogMessage(true, $"    Line {index + 1}: {lines[index]}");
                }
            }
        }
        else
        {
            if (amountRead < 0)
            {
                _f7MicroCoprocessor.Close(socketHandle);
                DebugHelpers.DebugHelpers.LogMessage(true, $"Exiting test");
                return;
            }
        }

        firstBlock = false;
    } while (amountRead > 0);

    DebugHelpers.DebugHelpers.LogMessage(true, $"    Total {totalNumberOfBytes} bytes read from server.");
    //
    //    close
    //
    DebugHelpers.DebugHelpers.LogMessage(true, "Closing socket (close)");
    if (_f7MicroCoprocessor.Close(socketHandle) < 0)
    {
        DebugHelpers.DebugHelpers.LogMessage(true, $"Exiting test");
        return;
    }

    DebugHelpers.DebugHelpers.LogMessage(true, "HTTP GET Test Complete.");
  */
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
bool espcptest_assert_print_value(FAR const char *func,
                                    const int line,
                                    FAR const char *check_str,
                                    long int test_value,
                                    long int should_be)
{
  ++overall.nchecks;

  if (test_value == should_be)
    {
      int keep_errno = errno;

      usrsocktest_dbg("%d => OK.\n", line);

      errno = keep_errno;
      return true;
    }

  utest_assert_print_head(func, line, check_str);

  printf("\t\t\tgot value: %ld\n", test_value);
  printf("\t\t\tshould be: %ld\n", should_be);

  fflush(stdout);
  fflush(stderr);

  return false;
}

bool espcptest_assert_print_buf(FAR const char *func,
                                  const int line,
                                  FAR const char *check_str,
                                  FAR const void *test_buf,
                                  FAR const void *expect_buf,
                                  size_t buflen)
{
  ++overall.nchecks;

  if (memcmp(test_buf, expect_buf, buflen) == 0)
    {
      int keep_errno = errno;

      usrsocktest_dbg("%d => OK.\n", line);

      errno = keep_errno;
      return true;
    }

  utest_assert_print_head(func, line, check_str);

  fflush(stdout);
  fflush(stderr);

  return false;
}

/****************************************************************************
 * espcptest_main
 ****************************************************************************/

#ifdef BUILD_MODULE
int main(int argc, FAR char *argv[])
#else
int esp32spicommstest_main(int argc, char *argv[])
#endif
{

  struct mallinfo mem_before, mem_after;

  memset(&overall, 0, sizeof(overall));

  printf("Starting unit-tests...\n");
  fflush(stdout);
  fflush(stderr);

  get_mallinfo(&mem_before);

  // runBatteryLevelTest();
  getGoogleHomePage();

  printf("Unit-test groups done... OK:%d, FAILED:%d, TOTAL:%d\n",
         overall.ok, overall.failed, overall.ok + overall.failed);
  printf(" -- number of checks made: %d\n", overall.nchecks);
  fflush(stdout);
  fflush(stderr);

  get_mallinfo(&mem_after);

  print_mallinfo(&mem_before, "HEAP BEFORE TESTS");
  print_mallinfo(&mem_after, "HEAP AFTER TESTS");

  fflush(stdout);
  fflush(stderr);
  exit(0);

  return 0;
}


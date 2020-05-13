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

/****************************************************************************
 * Name: runSpiHardwareTests
 *
 * Description:
 *  Test the SPI hardware interface.  This need to be run on the Meadow
 *  board, it cannot be run on the emulator.
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
static void runSpiHardwareTests(void)
{
  espcp_configuration_t *configuration = espcp_get_default_configuration();
  configuration->request_queue = espcp_create_message_queue(ESPCP_MESSAGE_QUEUE_NAME);

  espcp_spi_setup(espcp_queue_send_response_message);
  espcp_message_t *message = (espcp_message_t *) malloc(sizeof(espcp_message_t));
  message->message_type = espcp_message_types_header;
  message->interface = espcp_esp32_interfaces_system;
  message->function = espcp_system_function_get_battery_charge_level;
  message->status_code = 0;
  message->message_id = espcp_get_next_message_id();
  message->payload = NULL;
  message->payload_length = 0;
  int result;
  result = espcp_send_header(configuration, message);

  assert(result == espcp_status_codes_completed_ok);

  //
  //  Wait for the message waiting signal.
  //
  // espcp_wait_for_message_waiting_signal();

  // message->message_type = espcp_message_types_transport;
  // message->interface = espcp_esp32_interfaces_transport;
  // message->function = espcp_transport_function_send_response;
  // message->status_code = 0;
  // message->message_id = espcp_get_next_message_id();
  // message->payload = NULL;
  // message->payload_length = 0;
  // espcp_send_header(configuration, message);

  // espcp_message_t *acknowledgement;
  // result = espcp_get_message_header_acknowledgement(configuration, message, &acknowledgement);
  // assert(result == espcp_status_codes_completed_ok);

  // result = espcp_get_response_from_esp32(configuration);
  // assert(result == espcp_status_codes_completed_ok);




  // espcp_wi_fi_credentials_t *credentials = (espcp_wi_fi_credentials_t *) malloc(sizeof(espcp_wi_fi_credentials_t));
  // credentials->network_name = "YOUR NETWORK NAME";
  // credentials->password = "YOUR PASSWORD";
  // message->interface = espcp_esp32_interfaces_wi_fi;
  // message->function = espcp_wi_fi_function_start;
  // message->status_code = 0;
  // message->message_id = espcp_get_next_message_id();
  // message->payload_length = espcp_wi_fi_credentials_buffer_size(credentials);
  // message->payload = (uint8_t *) malloc(message->payload_length);
  // espcp_encode_wi_fi_credentials(credentials, message->payload);

  // espcp_send_header(configuration, message);
  // result = espcp_get_message_header_acknowledgement(configuration, message, &acknowledgement);
  // assert(result == espcp_status_codes_completed_ok);

  // message->message_type = espcp_message_types_data;
  // result = espcp_send_message_body(configuration, message);
  // assert(result == espcp_status_codes_completed_ok);
}

//
//  Temporary hack.
//
espcp_configuration_t *espcp_get_configuration(void);

void testGetBatteryLevel(void)
{
  espcp_init();
  espcp_configuration_t *configuration = espcp_get_configuration();

  espcp_message_t *message = (espcp_message_t *) malloc(sizeof(espcp_message_t));
  message->message_type = espcp_message_types_header;
  message->interface = espcp_esp32_interfaces_system;
  message->function = espcp_system_function_get_battery_charge_level;
  message->status_code = 0;
  message->message_id = espcp_get_next_message_id();
  message->payload = NULL;
  message->payload_length = 0;

  espcp_message_and_semaphore_t *message_and_semaphore = (espcp_message_and_semaphore_t *) malloc(sizeof(espcp_message_and_semaphore_t));
  message_and_semaphore->message = message;
  message_and_semaphore->semaphore = NULL;

  int result = espcp_add_message_to_queue(configuration->request_queue, message_and_semaphore);
  assert(result == espcp_status_codes_completed_ok);

  usleep(1000000);
}

void testStartWiFi(void)
{
  espcp_init();
  espcp_configuration_t *configuration = espcp_get_configuration();

  espcp_wi_fi_credentials_t *credentials = (espcp_wi_fi_credentials_t *) malloc(sizeof(espcp_wi_fi_credentials_t));
  credentials->network_name = NETWORK_NAME;
  credentials->password = NETWORK_PASSWORD;

  espcp_message_t *message = (espcp_message_t *) malloc(sizeof(espcp_message_t));
  message->message_type = espcp_message_types_header;
  message->interface = espcp_esp32_interfaces_wi_fi;
  message->function = espcp_wi_fi_function_start;
  message->status_code = 0;
  message->message_id = espcp_get_next_message_id();
  message->payload_length = espcp_wi_fi_credentials_buffer_size(credentials);
  message->payload = (uint8_t *) malloc(message->payload_length);
  espcp_encode_wi_fi_credentials(credentials, message->payload);

  espcp_message_and_semaphore_t *message_and_semaphore = (espcp_message_and_semaphore_t *) malloc(sizeof(espcp_message_and_semaphore_t));
  message_and_semaphore->message = message;
  sem_t semaphore;
  message_and_semaphore->semaphore = &semaphore;
  sem_init(message_and_semaphore->semaphore, 0, 0);

  int result = espcp_add_message_to_queue(configuration->request_queue, message_and_semaphore);
  assert(result == espcp_status_codes_completed_ok);
  sem_wait(message_and_semaphore->semaphore);
  printf("Done\n");
}

void runBatteryLevelTest(void)
{
  espcp_init();
  int32_t level = espcp_get_battery_charge_level();
}

void getGoogleHomePage()
{
  espcp_init();
  espcp_ip_information_t *ip = espcp_start_wifi(NETWORK_NAME, NETWORK_PASSWORD);  

  /*
    //
    //    getaddrinfo
    //
    var hints = new AddrInfo();
    hints.Family = (int)AddressFamilyType.AfInet;
    hints.SocketType = (int)SocketType.Stream;
    hints.AddrLength = 0;
    hints.CanonName = "";

    DebugHelpers.DebugHelpers.LogMessage(true, "Requesting lookup for www.google.com:80");
    var errorCode = _f7MicroCoprocessor.GetAddrInfo("www.google.com", "80", hints, out var res);
    DebugHelpers.DebugHelpers.LogMessage(true, $"    Error code {errorCode}");
    if (errorCode != 0)
    {
        DebugHelpers.DebugHelpers.LogMessage(true, $"Exiting test");
        return;
    }
    var getAddrInfoResult = (AddrInfo)res;
    //
    //    socket
    //
    DebugHelpers.DebugHelpers.LogMessage(true, "Getting socket handle");
    int socketHandle = _f7MicroCoprocessor.Socket(getAddrInfoResult.Family, getAddrInfoResult.SocketType, 0);
    DebugHelpers.DebugHelpers.LogMessage(true, $"    Socket handle: {socketHandle}");
    if (socketHandle < 0)
    {
        _f7MicroCoprocessor.FreeAddrInfo(getAddrInfoResult.MyHeapAddress);
        DebugHelpers.DebugHelpers.LogMessage(true, $"Exiting test");
        return;
    }
    //
    //    connect
    //
    DebugHelpers.DebugHelpers.LogMessage(true, "Opening connection (connect)");
    SockAddr sockAddr = Encoders.ExtractSockAddr(getAddrInfoResult.Addr, 0);
    if (_f7MicroCoprocessor.Connect(socketHandle, sockAddr, (int)getAddrInfoResult.AddrLen) < 0)
    {
        _f7MicroCoprocessor.Close(socketHandle);
        _f7MicroCoprocessor.FreeAddrInfo(getAddrInfoResult.MyHeapAddress);
        DebugHelpers.DebugHelpers.LogMessage(true, $"Exiting test");
        return;
    }
    DebugHelpers.DebugHelpers.LogMessage(true, "    Connected.");
    //
    //    freeaddrinfo
    //
    DebugHelpers.DebugHelpers.LogMessage(true, "Freeing address information resources (freeaddrinfo)");
    _f7MicroCoprocessor.FreeAddrInfo(getAddrInfoResult.MyHeapAddress);
    DebugHelpers.DebugHelpers.LogMessage(true, "    Complete.");
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

  // runAllTests();
  // runSpiHardwareTests();
  // testMessageThread();
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


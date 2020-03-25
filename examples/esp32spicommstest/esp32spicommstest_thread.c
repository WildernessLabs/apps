/****************************************************************************
 * examples/esp32spicommstest/esp32spicommstest_thread.c
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <debug.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "defines.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/

#ifndef dbg
  #define dbg _warn
#endif

#define usrsocktest_dbg(...) ((void)0)

#define TEST_SOCKET_SOCKID_BASE 10000U
#define TEST_SOCKET_COUNT 8

#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define noinline

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

struct esp32_spi_comms_configuration_s _configuration;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ESP32 SPI Communications thread test group setup
 *
 * Description:
 *   Setup function executed before each testcase in this test group
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

TEST_SETUP(Esp32SpiCommsThread)
{
  memset(&_configuration, 0, sizeof(_configuration));
}

/****************************************************************************
 * Name: ESP32 SPI Communications thread test group teardown
 *
 * Description:
 *   Setup function executed after each testcase in this test group
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

TEST_TEAR_DOWN(Esp32SpiCommsThread)
{
  int result;

  result = esp32spicomms_thread_stop(&_configuration);
  assert(result == OK);
}

/****************************************************************************
 * 
 * Two instances of the thread should result in a success for the first instance
 * but a failure for the second instance.
 * 
 * Pre-requisite:
 *  _configuration is setup in TEST_SETUP
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsThread, CheckIfThreadIsRunning)
{
  TEST_ASSERT_EQUAL(false, is_esp32spicomms_thread_running(&_configuration));
  TEST_ASSERT_EQUAL(OK, esp32spicomms_thread_start(&_configuration));
  TEST_ASSERT_EQUAL(true, is_esp32spicomms_thread_running(&_configuration));
  TEST_ASSERT_EQUAL(OK, esp32spicomms_thread_stop(&_configuration));
  TEST_ASSERT_EQUAL(false, is_esp32spicomms_thread_running(&_configuration));
}

/****************************************************************************
 * 
 * Make sure that we can start a single instance of the ESP32 SPI thread
 * 
 * Pre-requisite:
 *  _configuration is setup in TEST_SETUP
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsThread, ThreadSetup)
{
  TEST_ASSERT_EQUAL(OK, esp32spicomms_thread_start(&_configuration));
}

/****************************************************************************
 * 
 * Stopping a non-existant thread should just return OK.
 * 
 * Pre-requisite:
 *  _configuration is setup in TEST_SETUP
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsThread, StopThreadWhenNoThreadsRunning)
{
  TEST_ASSERT_EQUAL(OK, esp32spicomms_thread_stop(&_configuration));
}

/****************************************************************************
 * 
 * Check that we can stop a running thread.
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsThread, StopRunningThread)
{
  TEST_ASSERT_EQUAL(OK, esp32spicomms_thread_start(&_configuration));
  TEST_ASSERT_EQUAL(OK, esp32spicomms_thread_stop(&_configuration));
}

/****************************************************************************
 * 
 * Two instances of the thread should result in a success for the first instance
 * but a failure for the second instance.
 * 
 * Pre-requisite:
 *  _configuration is setup in TEST_SETUP
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsThread, CreateMoreThanOneThread)
{
  TEST_ASSERT_EQUAL(OK, esp32spicomms_thread_start(&_configuration));
  TEST_ASSERT_EQUAL(EALREADY, esp32spicomms_thread_start(&_configuration));
}

/****************************************************************************
 *
 * Run the test cases.
 * 
 ****************************************************************************/

TEST_GROUP(Esp32SpiCommsThread)
{
  RUN_TEST_CASE(Esp32SpiCommsThread, CheckIfThreadIsRunning);
  RUN_TEST_CASE(Esp32SpiCommsThread, StopThreadWhenNoThreadsRunning);
  RUN_TEST_CASE(Esp32SpiCommsThread, ThreadSetup);
  RUN_TEST_CASE(Esp32SpiCommsThread, CreateMoreThanOneThread);
  RUN_TEST_CASE(Esp32SpiCommsThread, StopRunningThread);
}

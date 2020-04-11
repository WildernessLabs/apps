/****************************************************************************
 * examples/espcptest/espcptest_thread.c
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

espcp_configuration_t g_configuration;

mqd_t g_queue_id = 0;

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
TEST_SETUP(EspcpThread)
{
  g_queue_id = espcp_create_message_queue(ESPCP_MESSAGE_QUEUE_NAME);
  TEST_ASSERT_EQUAL(0, espcp_thread_start(&g_configuration));
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
TEST_TEAR_DOWN(EspcpThread)
{
  TEST_ASSERT_EQUAL(0, espcp_thread_stop(&g_configuration));
  TEST_ASSERT_EQUAL(0, espcp_delete_message_queue(g_queue_id));
  g_queue_id = 0;
}

/****************************************************************************
 * 
 * Check to see if a thread is runing prior to thread startup.
 * 
 * Pre-requisite:
 *  _configuration is setup.
 * 
 ****************************************************************************/
// TEST(EspcpThread, CheckIfThreadRunningWhenNoThreadIsRunning)
// {
//   TEST_ASSERT_EQUAL(false, espcp_is_thead_running(&g_configuration));
// }

/****************************************************************************
 * 
 * Check to see if a thread is runing after thread startup.
 * but a failure for the second instance.
 * 
 * Pre-requisite:
 *  _configuration is setup.
 * 
 ****************************************************************************/
// TEST(EspcpThread, CheckIfThreadRunningAfterThreadStartup)
// {
//   TEST_ASSERT_EQUAL(false, espcp_is_thead_running(&g_configuration));
// }

/****************************************************************************
 * 
 * Make sure that we can start a single instance of the ESP32 SPI thread
 * 
 * Pre-requisite:
 *  _configuration is setup in TEST_SETUP
 * 
 ****************************************************************************/
TEST(EspcpThread, ThreadSetup)
{
  TEST_ASSERT_EQUAL(true, espcp_is_thead_running(&g_configuration));
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
// TEST(EspcpThread, CreateMoreThanOneThread)
// {
//   TEST_ASSERT_EQUAL(EALREADY, espcp_thread_start(&g_configuration));
// }

/****************************************************************************
 * 
 * Added 5 messages to the queue and the thread should pick up messages
 * and display the IDs.
 * 
 * Pre-requisite:
 *  _configuration is setup in TEST_SETUP
 *  Message queue test should be run first to ensure the queueing works.
 * 
 ****************************************************************************/
// TEST(EspcpThread, SendMessagesToThread)
// {
//   mqd_t queue_id = espcp_create_message_queue(ESPCP_MESSAGE_QUEUE_NAME);
//   TEST_ASSERT_EQUAL(true, queue_id > 0);
//   for (int index = 0; index < 5; index++)
//   {
//     espcp_message_t *message = malloc(sizeof(espcp_message_t));
//     message->message_id = index + 1;
//     printf("Adding message %u to queue\n", message->message_id);
//     espcp_message_and_semaphore_t *message_and_semaphore = malloc(sizeof(espcp_message_and_semaphore_t));
//     message_and_semaphore->message = message;
//     espcp_add_message_to_queue(_configuration.outbound_message_queue, message_and_semaphore);
//   }
// }

/****************************************************************************
 *
 * Run the test cases.
 * 
 ****************************************************************************/
TEST_GROUP(EspcpThread)
{
  memset(&g_configuration, 0, sizeof(g_configuration));
  
  // TEST_ASSERT_EQUAL(OK, espcp_create_message_queue(&_configuration));
  // RUN_TEST_CASE(EspcpThread, CheckIfThreadRunningWhenNoThreadIsRunning);
  // RUN_TEST_CASE(EspcpThread, CheckIfThreadRunningAfterThreadStartup);
  RUN_TEST_CASE(EspcpThread, ThreadSetup);
  // RUN_TEST_CASE(EspcpThread, CreateMoreThanOneThread);
  // RUN_TEST_CASE(EspcpThread, SendMessagesToThread);
}

/****************************************************************************
 * examples/espcptest/espcptest_queue.c
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
 * Name: ESP32 SPI Communications queue test group setup
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
TEST_SETUP(espcpQueue)
{
  memset(&_configuration, 0, sizeof(_configuration));
}

/****************************************************************************
 * Name: ESP32 SPI Communications queue test group teardown
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
TEST_TEAR_DOWN(espcpQueue)
{
  int result = OK;

  result = mq_close(_configuration.outbound_message_queue);

  assert(result == OK);
}

/****************************************************************************
 * 
 * Setup the message queue.
 * 
 * Pre-requisite:
 *  _configuration is setup in TEST_SETUP
 * 
 ****************************************************************************/
TEST(espcpQueue, QueueSetup)
{
  TEST_ASSERT_EQUAL(OK, espcp_create_message_queue(&_configuration));
}

/****************************************************************************
 * 
 * Add a message to the outbound message queue.
 * 
 * Pre-requisite:
 *  _configuration is setup in TEST_SETUP
 * 
 ****************************************************************************/
TEST(espcpQueue, AddAndRetrieveMessages)
{
  TEST_ASSERT_EQUAL(OK, espcp_create_message_queue(&_configuration));

  /*
   *  There should be no messages in the queue when the queue has just been created.
   */
  struct mq_attr mqStatus;
  TEST_ASSERT_EQUAL(OK, mq_getattr(_configuration.outbound_message_queue, &mqStatus));
  TEST_ASSERT_EQUAL(0, mqStatus.mq_curmsgs);

  /*
   *  Add a message to the queue.  At this point in time we do not care about
   *  the message contents, just that the message gets added.
   */
  struct message_s esp32Message;
  struct message_and_semaphore_s message;
  sem_t semaphore;
  message.message = &esp32Message;
  message.semaphore = semaphore;
  TEST_ASSERT_EQUAL(OK, espcp_add_message_to_queue(_configuration.outbound_message_queue, &message));

  /*
   *  There should now be 1 message in the queue.
   */
  TEST_ASSERT_EQUAL(OK, mq_getattr(_configuration.outbound_message_queue, &mqStatus));
  TEST_ASSERT_EQUAL(1, mqStatus.mq_curmsgs);

  /*
   *  Now get the "message" from the queue and see if we get a pointer to 
   *  the original message.
   */
  struct message_and_semaphore_s *retrieved_message;
  int number_of_bytes = mq_receive(_configuration.outbound_message_queue, (void *) &retrieved_message, sizeof(retrieved_message), NULL);
  TEST_ASSERT_EQUAL(sizeof(struct message_and_semaphore_s *), number_of_bytes);
  TEST_ASSERT_EQUAL(&message, retrieved_message);

  /*
   *  There should now be no messages in the queue.
   */
  TEST_ASSERT_EQUAL(OK, mq_getattr(_configuration.outbound_message_queue, &mqStatus));
  TEST_ASSERT_EQUAL(0, mqStatus.mq_curmsgs);
}

/****************************************************************************
 *
 * Run the test cases.
 * 
 ****************************************************************************/
TEST_GROUP(espcpQueue)
{
  RUN_TEST_CASE(espcpQueue, QueueSetup);
  RUN_TEST_CASE(espcpQueue, AddAndRetrieveMessages);
}

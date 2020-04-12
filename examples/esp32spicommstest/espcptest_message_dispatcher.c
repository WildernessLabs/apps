/****************************************************************************
 * examples/espcptest/espcptest_message_dispatcher.c
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

/*
 *  ID of the queue being tested.
 */
static mqd_t g_queue_id = 0;

/*
 *  The g_expected_buffer variable holds a pointer to a byte array of data that
 *  is expected as the result of a test.
 * 
 *  The test methods can use this to send expected results through to helper
 *  methods.
 */
uint8_t *g_expected_buffer = NULL;

static espcp_configuration_t g_configuration;


/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: TEST_SETUP(EspcpMessageDispatcher)
 *
 * Description:
 *  Setup the named message queue and assign the queue ID to a global
 *  variable.
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
TEST_SETUP(EspcpMessageDispatcher)
{
  /*
   *    The message dispatcher needs an existing message queue.
   */
  g_queue_id = espcp_create_message_queue(ESPCP_MESSAGE_QUEUE_NAME);
  TEST_ASSERT_EQUAL(true, ((int) g_queue_id) != -1);

  TEST_ASSERT_EQUAL(0, espcp_setup_message_dispatcher());
}

/****************************************************************************
 * Name: TEST_TEAR_DOWN(EspcpMessageDispatcher)
 *
 * Description:
 *  Close the message queue and then unlink it.
 * 
 *  At the end of this the queue should no longer exist.
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
TEST_TEAR_DOWN(EspcpMessageDispatcher)
{
  TEST_ASSERT_EQUAL(0, espcp_teardown_message_dispatcher());
  TEST_ASSERT_EQUAL(0, espcp_delete_message_queue(g_queue_id));
  g_queue_id = 0;
}

/****************************************************************************
 * 
 * Verify that espcp_get_next_message_id can get an incrementing message ID.
 * queue.
 * 
 * Pre-requisite:
 *  g_queue_id holds the descriptor to a valid message queue.
 * 
 ****************************************************************************/
TEST(EspcpMessageDispatcher, GetNextMessageID)
{
  TEST_ASSERT_EQUAL(0x80000001, espcp_get_next_message_id());
  TEST_ASSERT_EQUAL(0x80000002, espcp_get_next_message_id());
}

/****************************************************************************
 * 
 * Verify that espcp_queue_send_response_message will queue the
 * "Send Response" message.
 * 
 * Pre-requisite:
 *  g_queue_id holds the descriptor to a valid message queue.
 * 
 ****************************************************************************/
TEST(EspcpMessageDispatcher, QueueSendResponseMessage)
{
  struct mq_attr mqStatus; 
  TEST_ASSERT_EQUAL(OK, mq_getattr(g_queue_id, &mqStatus));
  TEST_ASSERT_EQUAL(0, mqStatus.mq_curmsgs);

  espcp_queue_send_response_message();

  TEST_ASSERT_EQUAL(OK, mq_getattr(g_queue_id, &mqStatus));
  TEST_ASSERT_EQUAL(1, mqStatus.mq_curmsgs);

  espcp_message_and_semaphore_t *retrieved_message;
  int number_of_bytes = mq_receive(g_queue_id, (void *) &retrieved_message, sizeof(retrieved_message), NULL);
  TEST_ASSERT_EQUAL(sizeof(espcp_message_and_semaphore_t *), number_of_bytes);
  TEST_ASSERT_EQUAL(0x20, retrieved_message->message->message_type);
  TEST_ASSERT_EQUAL(5, retrieved_message->message->interface);
  TEST_ASSERT_EQUAL(1, retrieved_message->message->function);
}

void mdh_send_header_to_coprocessor(void *tx, void *rx, size_t length)
{
  TEST_ASSERT_EQUAL_UINT8_ARRAY(g_expected_buffer, tx, length);
}

/****************************************************************************
 * 
 * Verify that a header message can be encoded and presented to a method.
 * 
 * In real life the method will send the encoded message to the ESP32,
 * here we will insert our own method that will compare the encoded message
 * with what we expect to be produced.
 * 
 * Pre-requisite:
 *  g_queue_id holds the descriptor to a valid message queue.
 * 
 ****************************************************************************/
TEST(EspcpMessageDispatcher, SendHeaderToCoprocessor)
{
  espcp_message_t *message = malloc(sizeof(espcp_message_t));
  memset(message, 0, sizeof(espcp_message_t));
  message->message_type = 0x80;
  message->interface = 1;
  message->function = 2;
  message->message_id = 3;

  espcp_message_and_semaphore_t *message_and_semaphore = malloc(sizeof(espcp_message_and_semaphore_t));
  message_and_semaphore->semaphore = NULL;
  message_and_semaphore->message = message;

  uint8_t array[] = 
  { 
    0x80, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,	0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xda, 0x8b, 0x31, 0x34, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };
  g_expected_buffer = array;

  // g_configuration.send_data_to_esp32 = mdh_send_header_to_coprocessor;
  g_configuration.send_data_to_esp32 = NULL;
  espcp_send_header_to_coprocessor(&g_configuration, message_and_semaphore);
}

/****************************************************************************
 *
 * Run the test cases.
 * 
 ****************************************************************************/
TEST_GROUP(EspcpMessageDispatcher)
{
    RUN_TEST_CASE(EspcpMessageDispatcher, GetNextMessageID);
    RUN_TEST_CASE(EspcpMessageDispatcher, QueueSendResponseMessage);
    // RUN_TEST_CASE(EspcpMessageDispatcher, SendHeaderToCoprocessor);
}

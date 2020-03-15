/****************************************************************************
 * examples/usrsocktest/usrsocktest_daemon.c
 *
 *   Copyright (C) 2015, 2017 Haltian Ltd. All rights reserved.
 *    Author: Jussi Kivilinna <jussi.kivilinna@haltian.com>
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

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: BasicDaemon test group setup
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

TEST_SETUP(Esp32SpiCommsTask)
{
  // sd = -1;
  // sd2 = -1;
  // sd3 = -1;
  // started = false;
}

/****************************************************************************
 * Name: BasicDaemon test group teardown
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

TEST_TEAR_DOWN(Esp32SpiCommsTask)
{
  int result;

  result = esp32spicomms_task_stop();
  assert(result == OK);
}

/****************************************************************************
 * 
 * Two instances of the task should result in a success for the first instance
 * but a failure for the second instance.
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsTask, CheckIfTaskIsRunning)
{
  TEST_ASSERT_EQUAL(false, is_esp32spicomms_task_running());
  TEST_ASSERT_EQUAL(OK, esp32spicomms_task_start());
  TEST_ASSERT_EQUAL(true, is_esp32spicomms_task_running());
  TEST_ASSERT_EQUAL(OK, esp32spicomms_task_stop());
  TEST_ASSERT_EQUAL(false, is_esp32spicomms_task_running());
}

/****************************************************************************
 * 
 * Make sure that we can start a single instance of the ESP32 SPI task
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsTask, TaskSetup)
{
  TEST_ASSERT_EQUAL(OK, esp32spicomms_task_start());
}

/****************************************************************************
 * 
 * Stopping a non-existant task should just return OK.
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsTask, StopTaskWhenNoTasksRunning)
{
  TEST_ASSERT_EQUAL(OK, esp32spicomms_task_stop());
}

/****************************************************************************
 * 
 * Check that we can stop a running task.
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsTask, StopRunningTask)
{
  TEST_ASSERT_EQUAL(OK, esp32spicomms_task_start());
  TEST_ASSERT_EQUAL(OK, esp32spicomms_task_stop());
}

/****************************************************************************
 * 
 * Two instances of the task should result in a success for the first instance
 * but a failure for the second instance.
 * 
 ****************************************************************************/
TEST(Esp32SpiCommsTask, CreateMoreThanOneTask)
{
  TEST_ASSERT_EQUAL(OK, esp32spicomms_task_start());
  TEST_ASSERT_EQUAL(-EALREADY, esp32spicomms_task_start());
}

/****************************************************************************
 *
 * Run the test cases.
 * 
 ****************************************************************************/

TEST_GROUP(Esp32SpiCommsTask)
{
  RUN_TEST_CASE(Esp32SpiCommsTask, CheckIfTaskIsRunning);
  RUN_TEST_CASE(Esp32SpiCommsTask, StopTaskWhenNoTasksRunning);
  RUN_TEST_CASE(Esp32SpiCommsTask, TaskSetup);
  RUN_TEST_CASE(Esp32SpiCommsTask, CreateMoreThanOneTask);
  RUN_TEST_CASE(Esp32SpiCommsTask, StopRunningTask);
}

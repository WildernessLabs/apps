/****************************************************************************
 * examples/esp32spicommstest/defines.h
 * Common defines for all esp32spicomms testcases
 *
 *   Copyright (C) 2015, 2017 Haltian Ltd. All rights reserved.
 *   Author: Roman Saveljev <roman.saveljev@haltian.com>
 *           Jussi Kivilinna <jussi.kivilinna@haltian.com>
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

#ifndef __EXAMPLES_ESP32SPICOMMSTEST_DEFINES_H
#define __EXAMPLES_ESP32SPICOMMSTEST_DEFINES_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>
#include <mqueue.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ESP32SPICOMMS_NODE "/dev/esp32spicomms"

#define ESP32SPICOMMSTEST_DAEMON_CONF_DEFAULTS \
  { \
    .max_sockets = UINT_MAX, \
    .supported_domain = AF_INET, \
    .supported_type = SOCK_STREAM, \
    .supported_protocol = 0, \
    .delay_all_responses = false, \
    .endpoint_addr = "127.0.0.1", \
    .endpoint_port = 255, \
    .endpoint_block_connect = false, \
    .endpoint_block_send = false, \
    .endpoint_recv_avail_from_start = true, \
    .endpoint_recv_avail = 4, \
  }

/* Test case macros */

#define utest_match(type, should_be_value, test_value) do { \
    type tmp_utest_test_value = (type)(test_value); \
    type tmp_utest_should_value = (type)(should_be_value); \
    if (!esp32spicommstest_assert_print_value(__func__, __LINE__, \
                                        "(" #type ")(" #test_value ") == " \
                                        "(" #type ")(" #should_be_value ")", \
                                        tmp_utest_test_value, \
                                        tmp_utest_should_value)) \
      { \
        esp32spicommstest_test_failed = true; \
        return; \
      } \
  } while(false)

#define utest_no_match(type, should_be_value, test_value) do { \
    type tmp_utest_test_value = (type)(test_value); \
    type tmp_utest_should_value = (type)(should_be_value); \
    if (esp32spicommstest_assert_print_value(__func__, __LINE__, \
                                        "(" #type ")(" #test_value ") == " \
                                        "(" #type ")(" #should_be_value ")", \
                                        tmp_utest_test_value, \
                                        tmp_utest_should_value)) \
      { \
        esp32spicommstest_test_failed = true; \
        return; \
      } \
  } while(false)

#define utest_match_buf(should_be_valuebuf, test_valuebuf, test_valuebuflen) do { \
    if (!esp32spicommstest_assert_print_buf(__func__, __LINE__, \
                                      "memcmp(" #should_be_valuebuf ", " \
                                                #test_valuebuf ", " \
                                                #test_valuebuflen ") == 0", \
                                      should_be_valuebuf, \
                                      test_valuebuf, \
                                      test_valuebuflen)) \
      { \
        esp32spicommstest_test_failed = true; \
        return; \
      } \
  } while(false)

#define RUN_TEST_CASE(g,t) do { \
    if (esp32spicommstest_test_failed) \
      { \
        return; \
      } \
    esp32spicommstest_group_##g##_setup(); \
    esp32spicommstest_test_##g##_##t(); \
    esp32spicommstest_group_##g##_teardown(); \
    if (esp32spicommstest_test_failed) \
      { \
        return; \
      } \
  } while (false)

#define RUN_TEST_GROUP(g) do { \
    extern const char *esp32spicommstest_group_##g; \
    void esp32spicommstest_group_##g##_run(void); \
    run_tests(esp32spicommstest_group_##g, esp32spicommstest_group_##g##_run); \
  } while (false)

#define TEST_GROUP(g)        const char *esp32spicommstest_group_##g = #g; \
                             void esp32spicommstest_group_##g##_run(void)
#define TEST_SETUP(g)        static void esp32spicommstest_group_##g##_setup(void)
#define TEST_TEAR_DOWN(g)    static void esp32spicommstest_group_##g##_teardown(void)
#define TEST(g,t)            static void esp32spicommstest_test_##g##_##t(void)

#define TEST_ASSERT_TRUE(v)    utest_match(bool, true, (v))
#define TEST_ASSERT_FALSE(v)   utest_match(bool, false, (v))
#define TEST_ASSERT_EQUAL(e,v) utest_match(ssize_t, (e), (v))
#define TEST_ASSERT_NOT_EQUAL(e,v) utest_no_match(ssize_t, (e), (v))
#define TEST_ASSERT_EQUAL_UINT8_ARRAY(e,v,s) utest_match_buf((e), (v), (s))

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct esp32_spi_comms_configuration_s
{
  bool thread_running;
  pthread_t thread;
  mqd_t outbound_message_queue;
};

struct esp32spicomms_message_s
{
    /*
     *  Type of message.
     */
    uint8_t message_type;

    /*
     *  Interface that this message is destined for.
     */
    uint8_t interface;

    /*
     *  Function (on the interface) to be executed.
     */
    uint32_t function;

    /*
     *  Status code (for returning messages) from the function.
     */
    uint32_t status_code;

    /*
     *  Unique ID of this message.
     */
    uint32_t message_id;

    /*
     *  Pointer to the payload data to be processed (or returned from) the function.
     */
    uint8_t *payload;

    /*
     *  Number of bytes in the payload.
     */
    uint32_t payload_length;
};

typedef struct esp32spicomms_message_s esp32spicomms_message_t;

struct message_and_semaphore_s
{
  sem_t semaphore;
  esp32spicomms_message_t *message;
};
/****************************************************************************
 * Public Data
 ****************************************************************************/

extern int esp32spicommstest_endp_malloc_cnt;
extern int esp32spicommstest_dcmd_malloc_cnt;
// extern const struct esp32spicommstest_daemon_conf_s esp32spicommstest_daemon_defconf;
// extern struct esp32spicommstest_daemon_conf_s esp32spicommstest_daemon_config;

extern bool esp32spicommstest_test_failed;

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

bool esp32spicommstest_assert_print_value(FAR const char *func,
                                    const int line,
                                    FAR const char *check_str,
                                    long int test_value,
                                    long int should_be);



/*
 *    Thread prototypes.
 */
int esp32spicomms_init(void);
int esp32spicomms_thread_start(struct esp32_spi_comms_configuration_s *);
bool is_esp32spicomms_thread_running(struct esp32_spi_comms_configuration_s *);

/*
 *    Queue prototypes.
 */
int esp32spicomms_create_message_queue(struct esp32_spi_comms_configuration_s *);
int esp32spicomms_delete_message_queue(void);
int esp32spicomms_add_message_to_queue(mqd_t, struct message_and_semaphore_s *);
struct message_and_semaphone *esp32spicomms_get_message_from_queue(mqd_t);

#endif /* __EXAMPLES_ESP32SPICOMMSTEST_DEFINES_H */

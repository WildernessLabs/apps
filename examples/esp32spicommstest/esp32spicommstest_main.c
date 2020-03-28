/****************************************************************************
 * examples/esp32spicommstest/esp32spicommstest_main.c
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

/****************************************************************************
 * Definitions
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
 * Name: run_tests
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
static void run_tests(void)
{
  struct esp32_spi_comms_configuration_s configuration;

  memset(&configuration, 0, sizeof(configuration));

  if (esp32spicomms_create_message_queue(&configuration) != OK)
  {
    printf("ERROR: Failed to create message queue.\n");
    return;
  }
  else
  {
    printf("Message queue created OK.\n");
  }
  

  if (esp32spicomms_thread_start(&configuration) != OK)
  {
    printf("ERROR: Failed to start ESP32 SPI Comms thread.\n");
    return;
  }
  else
  {
    printf("ESP32 SPI Comms thread started OK.\n");
  }
  
  printf("Adding five messages to the message queue for thread processing.\n");
  for (int index = 0; index < 5; index++)
  {
    esp32spicomms_message_t *message = malloc(sizeof(esp32spicomms_message_t));
    message->message_id = index + 1;
    printf("Adding message %u to queue\n", message->message_id);
    struct message_and_semaphore_s *message_and_semaphore = malloc(sizeof(struct message_and_semaphore_s));
    message_and_semaphore->message = message;
    esp32spicomms_add_message_to_queue(configuration.outbound_message_queue, message_and_semaphore);
  }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * esp32spicommstest_main
 ****************************************************************************/

#ifdef BUILD_MODULE
int main(int argc, FAR char *argv[])
#else
int esp32spicommstest_main(int argc, char *argv[])
#endif
{
  printf("ESP32SPI Comms test.\n");
  fflush(stdout);
  fflush(stderr);

  run_tests();

  printf("\n\nESP32 SPI Comms main program complete, thread should pick up the messages.\n");
  fflush(stdout);
  fflush(stderr);


  exit(0);

  return 0;
}


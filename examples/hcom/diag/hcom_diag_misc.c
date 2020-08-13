/****************************************************************************
 * configs/stm32f777-zit6-meadow/src/hcom/hcom_exec_utility_request.c
 * 
 *   Copyright (C) 2019 - 2020 Wilderness Labs. All rights reserved.
 *   Author:  Wilderness Labs
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

#include "../hcom_common.h"

#include <nuttx/arch.h>
#include <nuttx/mtd/mtd.h>
#include <nuttx/userspace.h>

#include <arch/board/boardctl.h>
#include <sys/boardctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/


/* Configuration ************************************************************/
/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

#if defined CONFIG_SYSTEM_NSH
int nsh_main(int argc, char *argv[]);
#endif

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int hcom_diag_misc_setup()
{
  return OK;
}

//=======================================================================================
// userData = 1 enables and userData = 0 disables
void hcom_diag_misc_launch_nsh(uint32_t userData)
{
  // p-m Need to swap out the _nsh_enabled for pthread_join. That way we can
  // use NSH>exit and then restart it again. The way it is, once started the
  // _nsh_enabled flag once set true won't allow nsh to be restarted.
#if defined CONFIG_SYSTEM_NSH
  int _nsh_pid;
  static bool _nsh_enabled = false;

  if(_nsh_enabled)
  {
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, "NSH already enabled",
            thisFile, __LINE__);
    return;
  }

  if(userData == 1)
  {
    // Create a unique task for NSH
    _nsh_pid = task_create("nsh", CONFIG_SYSTEM_NSH_PRIORITY,
                        CONFIG_SYSTEM_NSH_STACKSIZE,
                        (main_t)nsh_main,
                        (FAR char * const *) NULL);
    if(_nsh_pid > 0)
    {
      _nsh_enabled = true;
      syslog(1, "NSH now enabled [pid:%d, pri:%d, stack:%d]\n",
              _nsh_pid, CONFIG_SYSTEM_NSH_PRIORITY, CONFIG_SYSTEM_NSH_STACKSIZE);
      hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
              "NSH now enabled until Meadow restart", thisFile, __LINE__);
    }
    else
    {
      syslog(1, "NSH could not be created\n");
      hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
              "NSH could not be created", thisFile, __LINE__);
    }
  }
  else
  {
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
            "NSH not implemented", thisFile, __LINE__);
    return;
  }
#else
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, 
          "NuttShell not compiled", thisFile, __LINE__);
#endif
}

//============================================================================
// For diagnostic use only
void hcom_diag_misc_print_buffer(const uint8_t buffer[], const int bufLen, uint8_t msgPriority)
{
#if HCOM_COMMON_UTILS_DIAG_PRINT_BUFFER > 0

#define HCOM_UTIL_BYTES_PER_LINE 16
#define HCOM_UTIL_LEADING_SPACES 2
#define HCOM_UTIL_HEXADECIMAL_OFFSET (8 + HCOM_UTIL_LEADING_SPACES)
#define HCOM_UTIL_ASCII_OFFSET (57 + HCOM_UTIL_LEADING_SPACES)
#define HCOM_UTIL_DISPLAY_LENGTH (HCOM_UTIL_ASCII_OFFSET + HCOM_UTIL_BYTES_PER_LINE + 3)

  if ((hcom_logging_utils_get_syslog_mask() & LOG_MASK(msgPriority)) == 0)
    return;

  if(bufLen <= 0)
  {
    hcom_logging_syslog(msgPriority, "hcom_diag_misc_print_buffer() called with 'bufLen:%d'\n", bufLen);
    return;
  }

  if(buffer == NULL)
  {
    hcom_logging_syslog(msgPriority, "hcom_diag_misc_print_buffer() called with 'buffer == NULL'\n");
    return;
  }

  int rowStartOffset, rowByteOffset;
  char lineBuff[HCOM_UTIL_DISPLAY_LENGTH];
  int hexOffset;
  int asciiOffset;

  // One line at a time
  for (rowStartOffset = 0; rowStartOffset < bufLen; rowStartOffset += HCOM_UTIL_BYTES_PER_LINE)
  {
    memset(lineBuff, 0x20, HCOM_UTIL_DISPLAY_LENGTH);

    // Buffer offset address
    snprintf(&lineBuff[HCOM_UTIL_LEADING_SPACES], HCOM_UTIL_DISPLAY_LENGTH, "%08x ", rowStartOffset);

    hexOffset = HCOM_UTIL_HEXADECIMAL_OFFSET;
    asciiOffset = HCOM_UTIL_ASCII_OFFSET;

    for (rowByteOffset = 0; rowByteOffset < HCOM_UTIL_BYTES_PER_LINE; rowByteOffset++)
    {
      off_t buffOffset = rowStartOffset + rowByteOffset;
      if (buffOffset >= bufLen)
        break;        // Reached the end of the buffer's data

      // Grab the next byte to output
      uint8_t nextByte = buffer[buffOffset];

      // Save the hex value (add '.' half way)
      if(rowByteOffset == HCOM_UTIL_BYTES_PER_LINE / 2)
        snprintf(&lineBuff[hexOffset], HCOM_UTIL_DISPLAY_LENGTH - hexOffset, ".%02x", nextByte);
      else
        snprintf(&lineBuff[hexOffset], HCOM_UTIL_DISPLAY_LENGTH - hexOffset, " %02x", nextByte);
      hexOffset += 3;

      // Save the ascii value
      if (nextByte == 0) // Make it easy to spot '0'
        snprintf(&lineBuff[asciiOffset], HCOM_UTIL_DISPLAY_LENGTH - hexOffset, "-");
      else if (nextByte == 0xff)
        snprintf(&lineBuff[asciiOffset], HCOM_UTIL_DISPLAY_LENGTH - hexOffset, "*");
      else if (nextByte < 0x20 || nextByte > 0x7e)  //isprint()
        snprintf(&lineBuff[asciiOffset], HCOM_UTIL_DISPLAY_LENGTH - hexOffset, ".");
      else
        snprintf(&lineBuff[asciiOffset], HCOM_UTIL_DISPLAY_LENGTH - hexOffset, "%c", nextByte);

      asciiOffset++;
      DEBUGASSERT(asciiOffset < HCOM_UTIL_DISPLAY_LENGTH - 1);
    }

    // This row is ready
    lineBuff[hexOffset] = 0x20;   // Replace last hex null with a space
    lineBuff[asciiOffset++] = 0x0a; // line feed
    lineBuff[asciiOffset] = 0x00; // null terminator

  // Output one row at a time
  hcom_logging_syslog(msgPriority, lineBuff);
  }

#endif
}

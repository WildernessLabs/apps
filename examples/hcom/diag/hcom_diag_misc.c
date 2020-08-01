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

int nsh_main(int argc, char *argv[]);

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
void hcom_diag_misc_change_trace_level(uint32_t userData)
{
  char hostMsg[HCOM_SHORT_HOST_STRING_BUFF_LENGTH];
  int stringLen;

  int syslogmask = LOG_MASK(LOG_EMERG) | LOG_MASK(LOG_ALERT) | LOG_MASK(LOG_CRIT) | LOG_MASK(LOG_ERR) |
                   LOG_MASK(LOG_WARNING);

  switch (userData)
  {
    case HCOM_TRACE_LEVEL_NOTICE:
      syslogmask |= LOG_MASK(LOG_NOTICE);
      break;

    case HCOM_TRACE_LEVEL_NOTICE_INFO:
      syslogmask |= LOG_MASK(LOG_NOTICE) | LOG_MASK(LOG_INFO);
      break;

    case HCOM_TRACE_LEVEL_NOTICE_INFO_DEBUG:
      syslogmask |= LOG_MASK(LOG_NOTICE) | LOG_MASK(LOG_INFO) | LOG_MASK(LOG_DEBUG);
      break;
    
    case HCOM_TRACE_LEVEL_DEFAULT:
    default:    // use already calculated syslogmask
      break;
  }

  hcom_utils_clear_then_set_bbr_bits(HCOM_BBREG_RESTART_SYSLOG_CONFIG_VALUE_MASK, syslogmask);

  // Does the user care about the old trace level returned as a mask?
  int newTraceLevel = setlogmask(syslogmask);

  stringLen = snprintf(hostMsg, HCOM_SHORT_HOST_STRING_BUFF_LENGTH,
          "Trace level changed from 0x%02x to 0x%02x",
          newTraceLevel, syslogmask);

  DEBUGASSERT(stringLen < HCOM_SHORT_HOST_STRING_BUFF_LENGTH);
  hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, hostMsg,
          thisFile, __LINE__);

  hcom_utils_f7syslog(LOG_NOTICE, "Trace from 0x%02x to 0x%02x\n\n", newTraceLevel, syslogmask);
}

//=======================================================================================
// userData = 1 enables and userData = 0 disables
void hcom_diag_misc_launch_nsh(uint32_t userData)
{  
#if defined CONFIG_SYSTEM_NSH
  int _nsh_pid;
  static bool _nsh_enabled = false;

  if(_nsh_enabled)
  {
    hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, "NSH already enabled",
            thisFile, __LINE__);
    return;
  }

  if(userData == 1)
  {
    // Create a unique task for NSH
    _nsh_pid = task_create("nsh", CONFIG_SYSTEM_NSH_PRIORITY,
                        CONFIG_SYSTEM_NSH_STACKSIZE,
                        (main_t)nsh_main,
                        (FAR char * const *)NULL);
    if(_nsh_pid > 0)
    {
      _nsh_enabled = true;
      syslog(1, "NSH now enabled [pid:%d, pri:%d, stack:%d]\n",
              _nsh_pid, CONFIG_SYSTEM_NSH_PRIORITY, CONFIG_SYSTEM_NSH_STACKSIZE);
      hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
              "NSH now enabled until Meadow restart", thisFile, __LINE__);
    }
    else
    {
      syslog(1, "NSH could not be created\n");
      hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
              "NSH could not be created", thisFile, __LINE__);
    }
  }
  else
  {
    hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
            "NSH not implemented", thisFile, __LINE__);
    return;
  }
#else
  hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, 
          "NuttShell not compiled", thisFile, __LINE__);
#endif
}

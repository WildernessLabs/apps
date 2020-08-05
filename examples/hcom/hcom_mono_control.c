/****************************************************************************
 * \examples\hcom\hcom_mono_startup.c
 * 
 *   Copyright (C) 2020 Wilderness Labs. All rights reserved.
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

#include "hcom_common.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
bool hcom_mono_ctrl_is_app_exe(void);

/****************************************************************************
 * Public Functions
 ****************************************************************************/
// Check of the file App.exe exists in the files system
bool hcom_mono_ctrl_is_app_exe()
{
#ifdef CONFIG_MTD_PARTITION
  const char app_path[] = "/meadow0/App.exe";
#else
  const char app_path[] = "/meadow/App.exe";
#endif

  int fd = open(app_path, O_RDONLY);
  if (fd == -1) {
    syslog(LOG_ERR, "Mono managed app was not found in %s, skipping Mono initialization...\n",
      app_path);
    return false;
  }
  close(fd);
  return true;
}

//===================================================================
// Determine the state of the mono run flag, set by CLI
bool hcom_mono_ctrl_is_mono_enabled()
{
  return (hcom_utils_read_bbr_and_right_justify(HCOM_BBREG_MONO_START_BIT_MASK) != 0);
}

//=======================================================================================
// Disable Mono from running on next MCU reset
void hcom_mono_rqst_disable_mono(uint32_t userData)
{
  hcom_utils_clear_bbr_bits(HCOM_BBREG_MONO_START_BIT_MASK);

  char *sendMsgToHost = "Mono disabled. Restarting Meadow";
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
          sendMsgToHost, thisFile, __LINE__);

  hcom_utils_set_bbr_bits(HCOM_BBREG_RESTART_CONCLUDED_BIT_MASK);

  // Tell host to begin to reconnect
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_RECONNECT, userData,
          sendMsgToHost, thisFile, __LINE__);

  usleep(500 * 1000);
  hcom_proxy_restart_meadow();
}

//=======================================================================================
// Enable Mono to run on next MCU reset
void hcom_mono_rqst_enable_mono(uint32_t userData)
{
  hcom_utils_set_bbr_bits(HCOM_BBREG_MONO_START_BIT_MASK);

  char *sendMsgToHost = "Mono being enabled. Restarting F7 Micro";
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
          sendMsgToHost, thisFile, __LINE__);

  hcom_utils_set_bbr_bits(HCOM_BBREG_RESTART_CONCLUDED_BIT_MASK);
  
  // Tell host to begin to reconnect
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_RECONNECT, userData,
          sendMsgToHost, thisFile, __LINE__);

  usleep(500 * 1000);
  hcom_proxy_restart_meadow();
}

//======================================================================================
// The host has ask for the mono startup state
void hcom_mono_rqst_is_mono_enabled(uint32_t userData)
{
  char *monoStartupMsg;

  if(hcom_mono_ctrl_is_mono_enabled())
    monoStartupMsg = "On reset, mono will run app.exe";
  else
    monoStartupMsg = "On reset, mono will not run app.exe";

  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
          monoStartupMsg, thisFile, __LINE__);
}


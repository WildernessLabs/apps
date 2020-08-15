/****************************************************************************
 * \examples\hcom\hcom_mono_control.c
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
#include <meadow/hcom_protocol.h>
#include <meadow/hcom_bbreg_defn.h>

#define HCOM_MONO_RUNTIME_TASK_STACKSIZE 32768

#if defined(CONFIG_USERMAIN_PRIORITY)
// CONFIG_USERMAIN_PRIORITY defined via make menuconfig at
// RTOS Features > Tasks and Scheduling > init thread priority
#define HCOM_MONO_RUNTIME_TASK_PRIORITY CONFIG_USERMAIN_PRIORITY
#else
// SCHED_PRIORITY_DEFAULT defined in ...\Meadow\Meadow.OS\nuttx\include\sys\types.h
// currently a value of 100
#define HCOM_MONO_RUNTIME_TASK_PRIORITY SCHED_PRIORITY_DEFAULT
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

int mono_main(int argc, char *argv[]);

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
bool hcom_mono_ctrl_is_app_exe_available(void);
bool hcom_mono_ctrl_should_mono_run(void);
bool hcom_mono_ctrl_did_mono_run_last_time(void);

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int hcom_mono_ctrl_start_mono_main()
{
  int mono_pid;

  // Don't start if there's a reason
  if(!hcom_mono_ctrl_should_mono_run())
  {
    syslog(1, "-->Sorry, won't be creating mono main task\n"); usleep(100 * 1000);
    return OK;    // Reason has been reported already
  }

  // Set the flag that can identify if mono locked up. It will be
  // cleared by mono once mono is running correctly.
  hcom_bb_reg_acc_set_bbr_bits(HCOM_BBREG_MONO_LAST_RUN_LOCKUP_BIT);

  // Create a task to execute mono
  mono_pid = task_create("mono", HCOM_MONO_RUNTIME_TASK_PRIORITY,
                      HCOM_MONO_RUNTIME_TASK_STACKSIZE,
                      (main_t)mono_main,
                      (FAR char * const *) NULL);
  if(mono_pid > 0)
  {
    hcom_logging_syslog(LOG_INFO, "MONO started [pid:%d, pri:%d, stack size:%d]\n",
            mono_pid, HCOM_MONO_RUNTIME_TASK_PRIORITY, HCOM_MONO_RUNTIME_TASK_STACKSIZE);
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
            "MONO started", thisFile, __LINE__);
  }
  else
  {
    hcom_logging_syslog(LOG_ERR, "The mono task to run mono could not be created\n");
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
            "Mono could not be started", thisFile, __LINE__);
    return -1;
  }

  return OK;
}

//====================================================================
// Mono calls the function after it is running correctly
void hcom_mono_ctrl_clear_mono_is_running_flag()
{
  hcom_bb_reg_acc_clear_bbr_bits(HCOM_BBREG_MONO_LAST_RUN_LOCKUP_BIT);
  syslog(1, "mono cleared run flag\n"); usleep(50 * 1000);
}

//====================================================================
// This function will test all the reasons to start and not start mono
bool hcom_mono_ctrl_should_mono_run()
{
  // Is mono enabled?
  if(!hcom_mono_ctrl_is_mono_enabled())
  {
    char *noStartReason = "MONO won't start, it's not enabled";
    hcom_logging_syslog(LOG_WARNING, "%s\n", noStartReason);
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
            noStartReason, thisFile, __LINE__);
    return false;
  }

  // Does app.exe exist?
  if(!hcom_mono_ctrl_is_app_exe_available())
  {
    char *noStartReason = "MONO won't start, no App.exe could be found";
    hcom_logging_syslog(LOG_WARNING, "%s\n", noStartReason);
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
            noStartReason, thisFile, __LINE__);
    return false;
  }

  // Did mono run currectly the last time?
  if(!hcom_mono_ctrl_did_mono_run_last_time())
  {
    char *noStartReason = "MONO won't start, it didn't run correctly last time";
    hcom_logging_syslog(LOG_WARNING, "%s\n", noStartReason);
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
            noStartReason, thisFile, __LINE__);
    return false;
  }

  return true;
}

//===================================================================
// HCOM sets this bit everytime it starts mono. Mono then clears it once
// it's running. If this bit is set on hcom startup then we don't start
// mono because something is wrong and this could prevent any host
// communications from happening.
bool hcom_mono_ctrl_did_mono_run_last_time()
{
  if(hcom_bb_reg_acc_is_bbr_bit_set(HCOM_BBREG_MONO_LAST_RUN_LOCKUP_BIT))
  {
    // It should not be set unless mono locked up
    hcom_bb_reg_acc_clear_bbr_bits(HCOM_BBREG_MONO_LAST_RUN_LOCKUP_BIT);

    return false;
  }

  return true;
}

//===================================================================
// Check of the file App.exe exists in the files system
bool hcom_mono_ctrl_is_app_exe_available()
{
  // p-m need a common place to define these for here and mono_main
#ifdef CONFIG_MTD_PARTITION
  const char app_path[] = "/meadow0/App.exe";
#else
  const char app_path[] = "/meadow/App.exe";
#endif

  int fd = open(app_path, O_RDONLY);
  if (fd == -1) {
    syslog(LOG_ERR, "Mono-managed app was not found in %s, skipping Mono initialization...\n",
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
  return (hcom_bb_reg_acc_read_bbr_and_right_justify(HCOM_BBREG_USER_RQST_MONO_START_BIT) != 0);
}

//=======================================================================================
// The following are called from host 
//=======================================================================================
// Called from host to disable Mono from running on next MCU reset
void hcom_mono_rqst_disable_mono(uint32_t userData)
{
  hcom_bb_reg_acc_clear_bbr_bits(HCOM_BBREG_USER_RQST_MONO_START_BIT);

  char *sendMsgToHost = "Mono disabled. Restarting Meadow";
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
          sendMsgToHost, thisFile, __LINE__);

  // This tells hcom when it starts that a concluded messages needs to
  // be sent to the host
  hcom_bb_reg_acc_set_bbr_bits(HCOM_BBREG_RESTART_INITIATED_BY_HOST_CMD_BIT);

  // Tell host to begin to reconnect
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_RECONNECT, userData,
          sendMsgToHost, thisFile, __LINE__);

  usleep(500 * 1000);
  hcom_nx_restart_meadow();
}

//=======================================================================================
// Called from host to enable Mono to run on next MCU reset
void hcom_mono_rqst_enable_mono(uint32_t userData)
{
  hcom_bb_reg_acc_set_bbr_bits(HCOM_BBREG_USER_RQST_MONO_START_BIT);

  char *sendMsgToHost = "Mono being enabled. Restarting F7 Micro";
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
          sendMsgToHost, thisFile, __LINE__);

  // This tells hcom when it starts that a concluded messages needs to
  // be sent to the host
  hcom_bb_reg_acc_set_bbr_bits(HCOM_BBREG_RESTART_INITIATED_BY_HOST_CMD_BIT);
  
  // Tell host to begin to reconnect
  hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_RECONNECT, userData,
          sendMsgToHost, thisFile, __LINE__);

  usleep(500 * 1000);
  hcom_nx_restart_meadow();
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


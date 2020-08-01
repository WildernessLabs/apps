/****************************************************************************
 * \examples\hcom\hcom_startup_manager.c
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

#include "hcom_common.h"

// #if defined (CONFIG_HCOM_ESP32_COMMS)
// #include "esp32/hcom_esp32_comms.h"
// #endif

// #include <nuttx/kthread.h>
// #include <assert.h>
// #include "task/task.h"

// #if HCOM_TASK_SHOW_CREATED_TASK_PID_NAME > 0
// #include <nuttx/sched.h>
// #include <../sched/sched/sched.h>
// #endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
extern int hcom_main (int argc, char* argv[]);

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static void hcom_manager_shutdown(void);
static int hcom_manager_setup(void);

/****************************************************************************
 * Public Functions
 ****************************************************************************/
// This is the hcom tasks main thread, created by Nuttx when it has finished
// starting the OS. This thread will do all initialization of hcom, then
// becomes the thread that receives CLI messages.
// All other threads are created by this thread or one of it's child threads.
// This means that they are all in the same "task group." See the following
// https://cwiki.apache.org/confluence/display/NUTTX/Tasks+vs.+Threads+FAQ
// https://cwiki.apache.org/confluence/pages/viewpage.action?pageId=158862687
int hcom_main(int argc, char *argv[])
{
  int ret;

// #if HCOM_TASK_SHOW_CREATED_TASK_PID_NAME > 0
  syslog(1, "MGR-->The HCOM Main thread, \"init\" is running. It's PID:%d\n", getpid()); usleep(500 * 1000);
// #endif

 #if defined (CONFIG_RAMLOG_SYSLOG)
  // Sets up some basic initialization for ramlog, but may not create the
  // ramlog read thread etc. as this is not a commonly needed feature, some
  // setup is postponed until needed.
  ret = hcom_diag_ramlog_trace_setup();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup log tracing %d\n", thisFile, __LINE__, ret);
    return ret;
  }
#endif

  ret = hcom_proxy_access_setup();
  if (ret < 0)
  {
    syslog(LOG_CRIT, "%s@%d-setup hcom proxy access:%d\n", thisFile, __LINE__, ret);
    // hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup hcom utils:%d\n", thisFile, __LINE__, ret);
    return ret;
  }

  // Allocates memory for logging buffer
  ret = hcom_utils_setup();
  if (ret < 0)
  {
    syslog(LOG_CRIT, "%s@%d-setup hcom utils:%d\n", thisFile, __LINE__, ret);
    // hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup hcom utils:%d\n", thisFile, __LINE__, ret);
    return ret;
  }

#if HCOM_INCLUDE_TESTS_BBR_IN_STARTUP > 0
  hcom_bbr_tests();
#endif

#if defined(CONFIG_STM32F7_PWR)
  // Restore desired syslog mask from battery backed registers
  ret = hcom_utils_syslog_mask_init();
  if(ret < 0)
  {
    syslog(LOG_CRIT, "%s@%d-hcom_utils_syslog_mask_init failed:%d\n", thisFile, __LINE__, ret);
    return ret; 
  }
#endif

  // General setup
  syslog(1, "About to call hcom_manager_setup\n");
  ret = hcom_manager_setup();
  if (ret < 0)
  {
    syslog(LOG_CRIT, "%s@%d-hcom_manager_setup failed:%d\n", thisFile, __LINE__, ret);
    return ret;
  }

  //-------------------------------------------------------
  // This main thread will now run hcom CLI receive. It only
  // returns on shutdown or serious error
  //-------------------------------------------------------  
  ret = hcom_comms_recv_thread_loop();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-%s main-thread exited:%d\n",
            thisFile, __LINE__, HCOM_THREAD_NAME_HCOM_RECEIVE, ret);
  }

  hcom_manager_shutdown();
  return OK;
}

//=============================================================
int hcom_manager_setup()
{
  int ret;

//   // Determine if there's any special action required by mono_main. This sets up
//   // variables within mono_main.c before it is started by nuttx. When mono_main is started
//   // it checks if special action is necessary.
//   hcom_mono_ctrl_start_mono_if_needed();

//   // Sets the MTD for the file system
//   // Note: the calling thread is the nuttx startup thread. Any activity here may delay the
//   //  remainder of nuttx from starting, which may be determined to be a good thing.
//   ret = hcom_exec_flash_fs_setup(mtd);
//   if (ret < 0)
//   {
//     hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup F/S %d\n", thisFile, __LINE__, ret);
//     return ret;
//   }

  // Doesn't do anything
  ret = hcom_diag_misc_setup();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup misc %d\n", thisFile, __LINE__, ret);
    return ret;
  }
  
  ret = hcom_rqst_misc_setup();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup misc %d\n", thisFile, __LINE__, ret);
    return ret;
  }
//   // Sets a few internal variable states
  ret = hcom_dnld_proc_setup();
//   if (ret < 0)
//   {
//     hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup file download %d\n", thisFile, __LINE__, ret);
//     return ret;
//   }

  // Sets a few internal variable states
  ret = hcom_dnld_write_del_setup();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup file cmds %d\n", thisFile, __LINE__, ret);
    return ret;
  }

  // Allocates memory for the circular buffer
  ret = hcom_proc_rqst_setup();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup host request %d\n", thisFile, __LINE__, ret);
    return ret;
  }

  ret = hcom_route_rqst_setup();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup host request %d\n", thisFile, __LINE__, ret);
    return ret;
  }

//   ret = hcom_fs_setup(mtd);
//   if (ret < 0)
//   {
//     hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup F/S helper %d\n", thisFile, __LINE__, ret);
//     return ret;
//   }

// #if defined(CONFIG_HCOM_MTD_STRESS_TEST)
//   ret = proc_dnld_rqst_testing_setup(mtd);
//   if (ret < 0)
//   {
//     hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup testing %d\n", thisFile, __LINE__, ret);
//     return ret;
//   }
// #endif

  ret = hcom_comms_recv_setup();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup Host comms %d\n", thisFile, __LINE__, ret);
    return ret;
  }

  ret = hcom_comms_send_setup();
  if (ret < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup host msg builder:%d\n", thisFile, __LINE__, ret);
    return ret;
  }

// #if defined(CONFIG_HCOM_FILESYSTEM_INIT)
//   // This will format the file system as needed. This will prevent the nuttx OS from
//   // starting which includes mono. Therefore, mono cannot start until the file system
//   // is at least initialized. This is the desired behavior since mono starting before
//   // the file system would be a problem.
//   ret = hcom_fs_init_file_system();
//   if (ret < 0)
//   {
//     hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup F/S helper %d\n", thisFile, __LINE__, ret);
//     return ret;
//   }
// #endif

// #if defined(CONFIG_HCOM_MONO_OUTPUT_PIPE)
//   // Creates a named pipe (fifo) and creates a receiving thread.
//   ret = hcom_mono_pipe_setup();
//   if (ret < 0)
//   {
//     hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup mono pipe %d\n", thisFile, __LINE__, ret);
//     return ret;
//   }
// #endif

// #if defined(CONFIG_HCOM_MONO_DEBUG_PIPE)
//   // Creates a unix domain socket and creates a receiving thread.
//   ret = hcom_remote_dbg_setup();
//   if (ret < 0)
//   {
//     hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup remote dbg %d\n", thisFile, __LINE__, ret);
//     return ret;
//   }
// #endif

// #if defined (CONFIG_HCOM_ESP32_COMMS)
//   ret = hcom_esp32_uart_comms_setup();
//   if (ret < 0)
//   {
//     hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setup esp32 comms %d\n", thisFile, __LINE__, ret);
//     return ret;
//   }
// #endif

  // // Do this last! - Create a thread to handle receiving and responding to received messages
  // ret = hcom_manager_create_worker_thread();
  // if (ret < 0)
  // {
  //   hcom_utils_f7syslog(LOG_CRIT, "%s@%d-create hcom thread %s\n", thisFile, __LINE__, ret);
  //   return ret;
  // }

  return OK;
}

//============================================================================
// hcom main thread
// int hcom_comms_recv_task_main_thread(int argc, FAR char *argv[])
// {
//  int ret;

// // #if HCOM_TASK_SHOW_CREATED_TASK_PID_NAME > 0
// //   struct tcb_s *rtcb = this_task();
// //   hcom_utils_f7syslog(LOG_NOTICE, "PID:%d is '%s'\n", getpid(), rtcb->name);
// // #endif

//   // //-------------------------------------------------------
//   // // Main thread only returns on shutdown or serious error
//   // //-------------------------------------------------------  
//   // ret = hcom_comms_recv_thread_loop();
//   // if (ret < 0)
//   // {
//   //   hcom_utils_f7syslog(LOG_CRIT, "%s@%d-%s thread exit:%d\n", thisFile, __LINE__, HCOM_THREAD_NAME_HCOM_RECEIVE, ret);
//   // }

//   // hcom_manager_shutdown();

//   // hcom_utils_f7syslog(LOG_INFO, "%s thread exit'\n", HCOM_THREAD_NAME_HCOM_RECEIVE);

//  return OK;      // Thread exit
// }

//=========================================================================
// Notify all interested children that we are shutting down. This closes the
// comms file descriptor which will cause the worker thread to exit.
void hcom_manager_shutdown()
{
  syslog(1, "MGR-->%s@%d-Entered:%s hcom_diag_ramlog_trace_setup\n", thisFile, __LINE__, __func__); usleep(100 * 1000);
  
  // todo - confirm that all functions that need shutdown are called
  hcom_comms_recv_shutdown();
//   hcom_mono_pipe_shutdown();
  hcom_utils_shutdown();
  hcom_route_rqst_shutdown();  
  hcom_proc_rqst_shutdown();
  hcom_comms_send_msg_shutdown();
  hcom_dnld_write_del_shutdown();
//   hcom_fs_shutdown();
//   hcom_remote_dbg_shutdown();
// #if defined (CONFIG_HCOM_ESP32_COMMS)
//   hcom_esp32_uart_comms_shutdown();
// #endif
#if defined (CONFIG_RAMLOG_SYSLOG)
  hcom_diag_ramlog_trace_shutdown();
#endif
}

/****************************************************************************
 * \apps\examples\hcom\diag\hcom_diag_nsh_support.c
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
#include <meadow/hcom_protocol.h>

// #include <nuttx/arch.h>
// #include <nuttx/mtd/mtd.h>
// #include <nuttx/userspace.h>

// #include <arch/board/boardctl.h>
// #include <sys/boardctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if HCOM_NUTT_SHELL_LAUNCHER_INCLUDE_IN_BUILD > 0 &&  !defined (CONFIG_SYSTEM_NSH)
#warning"HCOM_NUTT_SHELL_LAUNCHER_INCLUDE_IN_BUILD is > 0 but CONFIG_SYSTEM_NSH is not defined"
#endif

// #if HCOM_NUTT_SHELL_LAUNCHER_INCLUDE_IN_BUILD > 0


/* Configuration ************************************************************/
/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

static bool _nsh_enabled;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

int nsh_main(int argc, char *argv[]);
static int nsh_main_proxy(int argcx, char *argvx[]);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int hcom_diag_nsh_support_setup()
{
  _nsh_enabled = false;
  return OK;
}

//=======================================================================================
// I'm leaving this code here because it could become useful. The reason this
// doesn't work is that when it was moved to /apps it now shares stdout with
// all other /apps applications including mono. Mono routes all Console.Write()
// calls to stdout. To get Console.Write() calls to output on CLI, stdout was
// redirected, and with it the NSH output was redirected too.
// If NSH is desired either disable mono from running via CLI command or prevent
// the stdout redirection code from running (via a code hack). Then hook up a
// terminal to D0 & D1 (UART 4).
// userData = 1 enables all other values are ignored. The CLI command --NSHEnable
// will automatically set userData to 1.
void hcom_diag_misc_launch_nsh(uint32_t userData)
{
  syslog(1, "===> HCOM Entered NSH launch. userData:%d\n", userData);

  // Currently, NSH can only be enabled once from the CLI. After this the _nsh_enable flag
  // will be set true and prevents nsh from being re-started.
  // A nice but not needed feature would be to find a different method to determine
  // if nsh is running (e.g. pthread_join, waitpid(),  waitid() or atexit()).
  // This is not a big problem especially as this code no longer works, as discribed
  // above.
  int _nsh_pid;

  if(_nsh_enabled)
  {
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, "NSH already enabled",
            thisFile, __LINE__);
    return;
  }

  if(userData == 1)
  {
    // General configuration
    // This UART was configured for NSH at
    //  Application Configuration -> NSH Library -> Console Configuration ->
    //  [*] New Console, [*] Alternate Console Device, and /dev/ttyS3
    // Device Drivers -> Serial Driver Support -> Serial console (No serial console)  --->
    // System Type -> U[S]ART Configuration [*] Disable reording of ttySx devices
    // Result is: /dev/ttyS0 = USART1, /dev/ttyS1 = UART4, /dev/ttyS2 = UART5 & /dev/ttyS3 = USART6
    //
    // When mono starts it reconfigures all the GPIOs. This call
    // will restore the Tx and Rx configuration to the UART
    #define HCOM_DIAG_NSH_SERIAL_DEVICE "/dev/ttyS1"  // This is UART4
    syslog(1, "===> Restoring the UART4's original configuration\n"); usleep(20 * 1000);
    hcom_via_nx_restore_uart_reconfig(MEADOW_RECONFIG_MISCONFIGURED_UART4);

    if(strcmp(HCOM_DIAG_NSH_SERIAL_DEVICE, "/dev/ttyS0") == 0)
      hcom_via_nx_restore_uart_reconfig(MEADOW_RECONFIG_MISCONFIGURED_UART1);
    else if(strcmp(HCOM_DIAG_NSH_SERIAL_DEVICE, "/dev/ttyS1") == 0)
      hcom_via_nx_restore_uart_reconfig(MEADOW_RECONFIG_MISCONFIGURED_UART4);
    else if (strcmp(HCOM_DIAG_NSH_SERIAL_DEVICE, "/dev/ttyS3") == 0)
      hcom_via_nx_restore_uart_reconfig(MEADOW_RECONFIG_MISCONFIGURED_UART6);
    else
    {
      syslog(1, "NSH Serial device '%s'unknown\n", HCOM_DIAG_NSH_SERIAL_DEVICE); usleep(20 * 1000);
      return;
    }

    syslog(1, "===> Launching NSH via task_create\n");

    // Create a unique task for NSH
    _nsh_pid = task_create("nsh", CONFIG_SYSTEM_NSH_PRIORITY,
                        CONFIG_SYSTEM_NSH_STACKSIZE,
                        (main_t)nsh_main_proxy,
                        (FAR char * const *) NULL);
    if(_nsh_pid > 0)
    {
      syslog(1, "===> Launch of NSH returned PID:%d\n", _nsh_pid);
      _nsh_enabled = true;
      hcom_logging_syslog(LOG_INFO, "%s@%d-NSH now enabled [pid:%d, pri:%d, stack:%d]\n",
              thisFile, __LINE__, _nsh_pid, CONFIG_SYSTEM_NSH_PRIORITY, CONFIG_SYSTEM_NSH_STACKSIZE);

      hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
              "NSH enabled until Meadow is restarted", thisFile, __LINE__);
    }
    else
    {
      syslog(1, "===> Launch of NSH failed with ret:%d, errno:%d\n", _nsh_pid, errno);
      hcom_logging_syslog(LOG_ERR, "%s@%d-NSH could not be created\n", thisFile, __LINE__);

      hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
              "NSH could not be created", thisFile, __LINE__);
    }
  }
  else
  {
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0,
            "It is not possible to disable NSH", thisFile, __LINE__);
    return;
  }

// #else // HCOM_NUTT_SHELL_LAUNCHER_INCLUDE_IN_BUILD
//   hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, 
//           "NuttShell not available", thisFile, __LINE__);
// #endif // HCOM_NUTT_SHELL_LAUNCHER_INCLUDE_IN_BUILD
}

//====================================================================
// This is necessary because in the configuration we are not copying
// stdin, stdout or stderr to the new task's file descriptor list.
// This is resolved here where the proper serial port is assigned to
// stdin, stdout and stderr.
int nsh_main_proxy(int argcx, char *argvx[])
{
  int ret;
  int argc = 0;
  char **argv = NULL;

  static int stdout_fd = -1;
  static int stderr_fd = -1;

  // Note: cannot use the hcom_via_nx_xxx() calls without calling
  // hcom_via_nx_upd_driver_open() to obtain a proper handle.
  // Why? Because this is a different task and therefore doesn't
  // have the file descriptors as the hcom task.

  if(stdout_fd < 0)
  {
    do
    {
      stdout_fd = open(HCOM_DIAG_NSH_SERIAL_DEVICE, O_WRONLY);
      if(stdout_fd >= 0)
        break;            // Success

      if(errno != ENOENT)   // ENOENT = Error No Entity -> No such file or directory
      {
        hcom_logging_syslog(LOG_ERR, "%s@%d-Open of %s failed errno:%d\n",
          thisFile, __LINE__, HCOM_DIAG_NSH_SERIAL_DEVICE, errno);
        stdout_fd = -1;
        return 1;
      }
      
      // All errors sleep and try again
      usleep(100 * 1000);
    } while (errno == ENOENT);

    // Assign the serial port write to the stdout fd.
    ret = dup2(stdout_fd, STDOUT_FILENO);
    if (ret < 0)
    {
      hcom_logging_syslog(LOG_ERR, "%s@%d-redirect_writer: dup2 failed ret:%d errno:%d\n",
                thisFile, __LINE__, ret, errno);
      return -errno;
    }
  }

  // stderr
  if(stderr_fd < 0)
  {
    do
    {
      stderr_fd = open(HCOM_DIAG_NSH_SERIAL_DEVICE, O_WRONLY);
      if(stderr_fd >= 0)
        break;            // Success

      if(errno != ENOENT)   // ENOENT = Error No Entity -> No such file or directory
      {
        hcom_logging_syslog(LOG_ERR, "%s@%d-Open of %s failed errno:%d\n",
          thisFile, __LINE__, HCOM_DIAG_NSH_SERIAL_DEVICE, errno);
        stderr_fd = -1;
        return 1;
      }
      
      // All errors sleep and try again
      usleep(100 * 1000);
    } while (errno == ENOENT);

    // Assign the serial port write to the stderr fd too
    ret = dup2(stderr_fd, STDERR_FILENO);
    if (ret < 0)
    {
      hcom_logging_syslog(LOG_ERR, "%s@%d-redirect_writer: dup2 failed ret:%d errno:%d\n",
                thisFile, __LINE__, ret, errno);
      return -errno;
    }
  }

  usleep(100 * 1000);

  // Okay to launch nsh now
  nsh_main(argc, argv);

  // Only here if NSH terminated
  return OK;
}

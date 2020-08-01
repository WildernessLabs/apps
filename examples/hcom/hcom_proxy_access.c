/****************************************************************************
 * \examples\hcom\hcom_proxy_access.c
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

// All of the calls that must be executed on the Nuttx OS side go though here

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <sys/ioctl.h>

#include "hcom_common.h"
#include "../../../nuttx/configs/stm32f777zit6-meadow/src/hcom_proxy/hcom_proxy_shared.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/


/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

static int _hcom_proxy_fd;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

int hcom_proxy_is_mounted(uint32_t partitionId, bool *isMounted);

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int hcom_proxy_access_setup()
{
  // Open the proxy
  _hcom_proxy_fd = open(HCOM_PROXY_UPD_DRIVER_NAME, O_RDONLY);
  if(_hcom_proxy_fd == -1)
  {
    syslog(LOG_ERR, "%s:%s()@%d-%s Failed to open, errno:%d\n",
            thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno); usleep(50 * 1000);
    return _hcom_proxy_fd;
  }

  syslog(LOG_ERR, "%s@%d-SUCCESS %s opened\n",
          thisFile, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME); usleep(50 * 1000);
  return OK;
}

//=============================================================
int hcom_proxy_set_bbr(uint32_t value)
{
  int ret;
  struct hcom_proxy_upd_bbr_value bbr_value;

  bbr_value.bbr_id = HCOM_BATTERY_BACKED_REGISTER_ID;
  bbr_value.value = value;

  ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_SET_BBR_VALUE, (unsigned long)&bbr_value);
  if (ret < 0)
  {
    syslog(LOG_ERR, "%s:%s()@%d-%s Failed to set reg, errno:%d\n",
            thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno);
    return ret;
  }
  return OK;
}

//=============================================================
int hcom_proxy_get_bbr(uint32_t *value)
{
  int ret;
  struct hcom_proxy_upd_bbr_value bbr_value;
  
  bbr_value.bbr_id = HCOM_BATTERY_BACKED_REGISTER_ID;

  ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_GET_BBR_VALUE, (unsigned long)&bbr_value);
  if (ret < 0)
  {
    syslog(LOG_ERR, "%s:%s()@%d-%s Failed to get reg, errno:%d\n",
            thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno);
    return ret;
  }

  *value = bbr_value.value;
  return OK;
}

//=============================================================
int hcom_proxy_update_bbr(uint32_t clearBits, uint32_t setBits)
{
  int ret;
  struct hcom_proxy_upd_bbr_update bbr_update;

  bbr_update.bbr_id = HCOM_BATTERY_BACKED_REGISTER_ID;
  bbr_update.clearBits = clearBits;
  bbr_update.setBits = setBits;
  
  ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_UPDATE_BBR_VALUE, (unsigned long)&bbr_update);
  if (ret < 0)
  {
    syslog(LOG_ERR, "%s:%s()@%d-%s Failed to update reg, errno:%d\n",
            thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno);
    return ret;
  }
  return OK;
}

//=============================================================
// This is a stub for getting the mcu unique id
int hcom_proxy_get_mcu_id(uint8_t uniqueId[12])
{
  int ret;

  ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_GET_MCU_ID, (unsigned long)uniqueId);
  if (ret < 0)
  {
    syslog(LOG_ERR, "%s:%s()@%d-%s Failed to get mcu id, errno:%d\n",
            thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno);
    return ret;
  }

  return OK;
}

//=============================================================
// Is this this partition mounted in the file system?
bool hcom_fs_is_mounted(uint32_t partitionId)
{
  int ret;
  bool isMounted;
  
  ret = hcom_proxy_is_mounted(partitionId, &isMounted);
  if(ret < 0)
  {
    return false;
  }
  return isMounted;
}

// The is mounted proxy caller
int hcom_proxy_is_mounted(uint32_t partitionId, bool *isMounted)
{
  int ret;
  struct hcom_proxy_upd_is_part_mounted is_mounted;
  is_mounted.partitionId = partitionId;

  ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_IS_PART_MOUNTED, (unsigned long) &is_mounted);
  if (ret < 0)
  {
    syslog(LOG_ERR, "%s:%s()@%d-%s Failed to get mcu id, errno:%d\n",
            thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno);
    return ret;
  }

  *isMounted = is_mounted.isMounted;
  return OK;
}

//=============================================================
// This is a stub for restarting meadow
int hcom_proxy_restart_meadow(void)
{
  hcom_proxy_forward_cmd_to_proxy(0, HCOM_PROXY_CLI_CMD_RESTART_MEADOW, 0);
  return OK;
}

//=============================================================
// Those commands that need to be executed on the OS side are
// routed through here
void hcom_proxy_forward_cmd_to_proxy(uint16_t cliCmd, uint32_t proxyCmd, uint32_t userData)
{
  int ret;
  struct hcom_proxy_cmd_data cmdData;

  cmdData.proxyCmd = proxyCmd;
  cmdData.userData = userData;

  cmdData.logLevel = LOG_NONE;
  cmdData.logLen = 0;
  cmdData.send_host_text = hcom_comms_send_simple_text_msg;

  // Forward to hcom_proxy to complete command
  ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_CLI_COMMAND, (unsigned long)&cmdData);
  if (ret < 0)
  {
    if(cmdData.logLen > 0)
    {
      DEBUGASSERT(cmdData.logLevel != LOG_NONE);
      DEBUGASSERT(cmdData.logLen <= HCOM_PROXY_CMD_LOG_MSG_SIZE);
      hcom_utils_f7syslog(cmdData.logLevel, "%s [cli cmd:0x%04x]", cmdData.logMsg, cliCmd);
    }
    else
    {
      hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed to update reg, errno:%d, cli cmd:0x%04x\n",
              thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno, cliCmd);
      return;   // Returned data may not be reliable
    }
  }
}


// NO DIRECT ACCESS TO REGISTERS IF POSSIBLE
// //=============================================================
// int hcom_proxy_set_register(uint32_t address, uint32_t value)
// {
//   int ret;
//   struct hcom_proxy_upd_register_value reg_value;

//   reg_value.address = address;
//   reg_value.value = value;

//   ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_SET_REGISTER, (unsigned long)&reg_value);
//   if (ret < 0)
//   {
//     syslog(LOG_ERR, "%s:%s()@%d-%s Failed to set reg, errno:%d\n",
//             thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno);
//     return ret;
//   }
//   return OK;
// }

// //=============================================================
// int hcom_proxy_get_register(uint32_t address, uint32_t *value)
// {
//   int ret;
//   struct hcom_proxy_upd_register_value reg_value;
  
//   reg_value.address = address;

//   ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_GET_REGISTER, (unsigned long)&reg_value);
//   if (ret < 0)
//   {
//     syslog(LOG_ERR, "%s:%s()@%d-%s Failed to get reg, errno:%d\n",
//             thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno);
//     return ret;
//   }

//   *value = reg_value.value;
//   return OK;
// }

// //=============================================================
// int hcom_proxy_update_register(uint32_t address, uint32_t clearBits, uint32_t setBits)
// {
//   int ret;
//   struct hcom_proxy_upd_register_update reg_update;

//   reg_update.address = address;
//   reg_update.clearBits = clearBits;
//   reg_update.setBits = setBits;
  
//   ret = ioctl(_hcom_proxy_fd, HCOM_PROXY_UPD_UPDATE_REGISTER, (unsigned long)&reg_update);
//   if (ret < 0)
//   {
//     syslog(LOG_ERR, "%s:%s()@%d-%s Failed to update reg, errno:%d\n",
//             thisFile, __func__, __LINE__, HCOM_PROXY_UPD_DRIVER_NAME, errno);
//     return ret;
//   }
//   return OK;
// }


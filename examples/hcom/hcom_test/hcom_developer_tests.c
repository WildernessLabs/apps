/****************************************************************************
 * examples\hcom\host_rqsts\hcom_developer_tests.c
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

#include "../hcom_common.h"

// #include <nuttx/arch.h>
// #include <nuttx/mtd/mtd.h>
// #include <nuttx/userspace.h>

// #include <arch/board/boardctl.h>
// #include <sys/boardctl.h>

#include "../../../nuttx/configs/stm32f777zit6-meadow/src/hcom_proxy/hcom_proxy_shared.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/


/* Configuration ************************************************************/
/****************************************************************************
 * Private Data
 ****************************************************************************/
// static char *thisFile = __FILE__;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/
void hcom_developer_tests_developer_1(uint32_t userData)
{
  // This will control gpio A0-misoports based on userData
  // userData = 1 to 9 turns on gpios
  // userData = -1 to -9 turns on gpios

  int ret;
  static bool isInitialized = false;
  uint8_t gpioDefn;
  uint8_t cmdValue;
  int32_t signedUserData = (int32_t)userData;

  // HcomProxyGpioDefnA0___01,
  // HcomProxyGpioDefnA1___02,
  // HcomProxyGpioDefnA2___03,
  // HcomProxyGpioDefnA3___04,
  // HcomProxyGpioDefnA4___05,
  // HcomProxyGpioDefnA5___06,
  // HcomProxyGpioDefnSCK__07,
  // HcomProxyGpioDefnMOSI_08,
  // HcomProxyGpioDefnMISO_09,
  if(!isInitialized)
  {
    // Configure gpios in array offsets 2 - 10
    for(int gpioOffset = 2; gpioOffset < 11; gpioOffset++)
    {
      ret = hcom_proxy_gpio_config(gpioOffset, HCOM_PROXY_GPIO_CONFIG_OUTPUT);
      if(ret < 0)
      {
        syslog(1, "hcom_proxy_gpio_config value of:%d\n", gpioOffset);
      }
    }
    isInitialized = true;
  }

  if(signedUserData == 0 || signedUserData > 9 || signedUserData < -9)
  {
    syslog(1, "userData of:%d is not valid\n", signedUserData);
    return;
  }

  if(signedUserData > 0)
  {
    gpioDefn = userData + 1;    // userData of 1 is offset of 2
    cmdValue = HCOM_PROXY_GPIO_CMD_VALUE_HIGH;
  }
  else
  {
    gpioDefn = (signedUserData * -1) + 1;
    cmdValue = HCOM_PROXY_GPIO_CMD_VALUE_LOW;
  }

  ret = hcom_proxy_gpio_write(gpioDefn, cmdValue);
  if(ret < 0)
  {
    syslog(1, "hcom_proxy_gpio_write error:%d gpio:%d, value:%d\n", ret, gpioDefn,cmdValue);
  }
}

//==============================================================
void hcom_developer_tests_developer_2(uint32_t userData)
{
  int ret;
  static bool isInitialized = false;

  // Step 1 configure gpios for output
  // HcomProxyGpioDefnEspReset,
  // HcomProxyGpioDefnEspBoot,
  // HcomProxyGpioDefnA0___01,
  // HcomProxyGpioDefnA1___02,
  // HcomProxyGpioDefnA2___03,
  // HcomProxyGpioDefnA3___04,
  // HcomProxyGpioDefnA4___05,
  // HcomProxyGpioDefnA5___06,
  // HcomProxyGpioDefnSCK__07,
  // HcomProxyGpioDefnMOSI_08,
  // HcomProxyGpioDefnMISO_09,
  if(!isInitialized)
  {
    // Configure gpios in array offsets 2 - 10 whicn are only used for testing
    for(int gpioOffset = 2; gpioOffset < 11; gpioOffset++)
    {
      ret = hcom_proxy_gpio_config(gpioOffset, HCOM_PROXY_GPIO_CONFIG_OUTPUT);
      if(ret < 0)
      {
        syslog(1, "hcom_proxy_gpio_config value of:%d\n", gpioOffset);
      }
    }
    isInitialized = true;
  }
  
  // TEST ESP32 GPIOs
  ret = hcom_proxy_gpio_config(0, HCOM_PROXY_GPIO_CONFIG_OUTPUT);
  if(ret < 0)
  {
    syslog(1, "hcom_proxy_gpio_config value of:%d\n", 0);
  }

  ret = hcom_proxy_gpio_config(1, HCOM_PROXY_GPIO_CONFIG_OUTPUT);
  if(ret < 0)
  {
    syslog(1, "hcom_proxy_gpio_config value of:%d\n", 1);
  }
  hcom_proxy_gpio_write(0, 1);
  sleep(1);
  hcom_proxy_gpio_write(0, 0);
  sleep(1);
  hcom_proxy_gpio_write(0, 1);
  sleep(1);
  hcom_proxy_gpio_write(0, 0);
  sleep(1);

  hcom_proxy_gpio_write(1, 1);
  sleep(1);
  hcom_proxy_gpio_write(1, 0);
  sleep(1);
  hcom_proxy_gpio_write(1, 1);
  sleep(1);
  hcom_proxy_gpio_write(1, 0);
  sleep(1);

  // uint32_t gpioOffset4 = 4;
  // for(int cnt = 0; cnt < userData; cnt++)
  // {
  //     ret = hcom_proxy_gpio_write(gpioOffset4, 1);
  //     ret = hcom_proxy_gpio_write(gpioOffset4, 0);
  // }
  // return;

  // Too fast to see
  for(int cnt = 0; cnt < userData; cnt++)
  {
    // Turn all on
    for(int gpioOffset = 2; gpioOffset < 11; gpioOffset++)
    {
      ret = hcom_proxy_gpio_write(gpioOffset, 1);
      if(ret < 0)
      {
        syslog(1, "hcom_proxy_gpio_config value of:%d\n", gpioOffset);
      }
    }

    // Turn all off
    for(int gpioOffset = 2; gpioOffset < 11; gpioOffset++)
    {
      ret = hcom_proxy_gpio_write(gpioOffset, 0);
      if(ret < 0)
      {
        syslog(1, "hcom_proxy_gpio_config value of:%d\n", gpioOffset);
      }
    }
    //usleep(1);
  }
}

//==============================================================
void hcom_developer_tests_developer_3(uint32_t userData)
{

}

//==============================================================
void hcom_developer_tests_developer_4(uint32_t userData)
{
  // Shows all devices within Nuttx system on cli
  hcom_list_all_dev_dir_and_files_start(userData);
}


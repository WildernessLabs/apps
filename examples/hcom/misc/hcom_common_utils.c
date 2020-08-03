/****************************************************************************
 * \examples\hcom\hcom_common_utils.c
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
#include <ctype.h>
#include "hcom_common.h"

#include <nuttx/config.h>
#include "syslog.h"

//#include <nuttx/userspace.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

#if HCOM_COMMON_UTILS_GPIO_D00_D08_DOUT > 0
static void hcom_utils_dbg_gpio_init_d0_d8(void);
#endif
#if HCOM_COMMON_UTILS_GPIO_A0_MISO_DOUT > 0
static void hcom_utils_dbg_gpio_init_a0_miso(void);
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int hcom_common_utils_setup()
{
#if HCOM_COMMON_UTILS_GPIO_D00_D08_DOUT > 0
  hcom_utils_dbg_gpio_init_d0_d8();
#endif
#if HCOM_COMMON_UTILS_GPIO_A0_MISO_DOUT > 0
  hcom_utils_dbg_gpio_init_a0_miso();
#endif

  return OK;
}

//============================================================================
void hcom_common_utils_shutdown()
{
}

//===================================================================
// This function takes a value (0-31) for the battery backed reg,
// reads the value then based on the mask clears all other bits and
// right justifies the remaining bit(s).
uint32_t hcom_utils_read_bbr_and_right_justify(uint32_t bitMask)
{
  uint32_t regValue;
 
  if(bitMask == 0)
    return 0;

  // syslog(1, "bitMask:0x%08x\n", bitMask);
  
  int ret = hcom_proxy_get_bbr(&regValue);
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
  }

  // syslog(1, ">>>*** bbr reg:0x%08x\n", regValue);
  // syslog(1, ">>>*** bitMask:0x%08x\n", bitMask);

  if(regValue == 0)
    return 0;

  regValue &= bitMask;    // Save only bit in mask
  // syslog(1, ">>>*** regValue &= bitMask:0x%08x\n", regValue);

  // We have the 32-bit value, shift right based on the mask
  uint32_t tempMask = bitMask;
  while((tempMask & 0x00000001) == 0)
  {
    tempMask >>= 1;
    regValue >>= 1;
  }

  // syslog(1, ">>>*** return Value:0x%08x\n", regValue);
  return regValue;
}

//===================================================================
// Reads any of the 32 battery backed registers
uint32_t hcom_utils_bbr_read()
{
  uint32_t regValue;
  int ret = hcom_proxy_get_bbr(&regValue);
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s@%d-%s Failed, ret:%d errno:%d\n",
            thisFile, __LINE__, ret, errno);
  }
  return regValue;
}

//===================================================================
// Writes any of the 32 battery backed registers
void hcom_utils_bbr_write(uint32_t value)
{
  int ret = hcom_proxy_set_bbr(value);
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s@%d-%s Failed, ret:%d errno:%d\n",
            thisFile, __LINE__, ret, errno);
  }
}

//===================================================================
// Reads the state of a bit then clears that bit and returns it's
// original state.
bool hcom_utils_is_bbr_bits_set_n_clear(uint32_t value)
{
  uint32_t regValue;
  int ret = hcom_proxy_get_bbr(&regValue);
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s@%d-%s Failed, ret:%d errno:%d\n",
            thisFile, __LINE__, ret, errno);
  }
  ret = hcom_proxy_set_bbr(regValue & (~value));
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s@%d-%s Failed, ret:%d errno:%d\n",
            thisFile, __LINE__, ret, errno);
  }
  return (value & regValue) != 0;
}

//===================================================================
// Reads bits in register and returns their state
bool hcom_utils_is_bbr_bit_set(uint32_t value)
{
  uint32_t regValue;
  int ret = hcom_proxy_get_bbr(&regValue);
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s@%d-%s Failed, ret:%d errno:%d\n",
            thisFile, __LINE__, ret, errno);
  }
  return (value & regValue) != 0;
}

//===================================================================
// Set bit(s) in any bit in battery backed register
void hcom_utils_set_bbr_bits(uint32_t value)
{
  int ret = hcom_proxy_update_bbr(0, value);
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s@%d-%s Failed, ret:%d errno:%d\n",
            thisFile, __LINE__, ret, errno);
  }
}

//===================================================================
// Clears the specified bits
void hcom_utils_clear_bbr_bits(uint32_t value)
{
  int ret = hcom_proxy_update_bbr(value, 0);
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s@%d-%s Failed, ret:%d errno:%d\n",
            thisFile, __LINE__, ret, errno);
  }
}

//===================================================================
// Clears then sets the specified bits
void hcom_utils_clear_then_set_bbr_bits(uint32_t clearBits, uint32_t setBits)
{
  int ret = hcom_proxy_update_bbr(clearBits, setBits);
  if(ret < 0)
  {
    hcom_logging_syslog(LOG_ERR, "%s@%d-%s Failed, ret:%d errno:%d\n",
            thisFile, __LINE__, ret, errno);
  }
}

//===================================================================
// This is called during startup, before the hcom thread is created,
// to check if we are running under the QEMU virtualization model.
// 
// #define QEMU_BOOT_INFO_MAGIC 0x12341234
// #define QEMU_BOOT_INFO_OFFSET_FROM_SDRAM_END 1024
// #define QEMU_BOOT_INFO_ADDRESS (CONFIG_HEAP2_BASE + CONFIG_HEAP2_SIZE - QEMU_BOOT_INFO_OFFSET_FROM_SDRAM_END)

// bool hcom_utils_boot_time_qemu_check()
// {
//     // As part of the booting process, QEMU writes a token value
//     // to the first page of SDRAM. This logic is implemented at
//     // qemu/hw/arm/meadow.c:meadow_machine_reset.

//     uint32_t *addr = (uint32_t *)QEMU_BOOT_INFO_ADDRESS; 
//     return *addr == QEMU_BOOT_INFO_MAGIC;
// }

#if HCOM_COMMON_UTILS_GPIO_A0_MISO_DOUT > 0
//=================================================================
void hcom_utils_dbg_gpio_init_a0_miso()
{
  stm32_configgpio(MEADOW_DIAG_GPIO_A0___01_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_A1___02_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_A2___03_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_A3___04_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_A4___05_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_A5___06_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_SCK__07_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_MOSI_08_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_MISO_09_OUTPUT);
  usleep(100 * 1000);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_A0___01_OUTPUT, false);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_A1___02_OUTPUT, false);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_A2___03_OUTPUT, false);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_A3___04_OUTPUT, false);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_A4___05_OUTPUT, false);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_A5___06_OUTPUT, false);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_SCK__07_OUTPUT, false);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_MOSI_08_OUTPUT, false);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_MISO_09_OUTPUT, false);
}
#endif

#if HCOM_COMMON_UTILS_GPIO_D00_D08_DOUT > 0
//=================================================================
void hcom_utils_dbg_gpio_init_d0_d8()
{
  stm32_configgpio(MEADOW_DIAG_GPIO_D00__10_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_D01__11_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_D02__12_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_D03__13_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_D04__14_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_D05__15_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_D06__16_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_D07__17_OUTPUT);
  stm32_configgpio(MEADOW_DIAG_GPIO_D08__18_OUTPUT);
  usleep(100 * 1000);
}

//=================================================================
// Assumes gpio (D8 on meadow) can output as binary
void hcom_utils_dbg_gpio_1led_update(bool ledOn)
{
  // Note: !ledOn because low (false) turns led on for an open drain
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D08__18_OUTPUT, !ledOn);
}

//=================================================================
// Assumes gpio (D0-D7 on meadow) can output 8-bit as binary
void hcom_utils_dbg_gpio_8bit_update(uint8_t newValue, bool ledOn)
{ 
  // Note: Low (false) turns led on for an open drain
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D00__10_OUTPUT, (newValue & 0x01) > 0 ?  false : true);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D01__11_OUTPUT, (newValue & 0x02) > 0 ?  false : true);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D02__12_OUTPUT, (newValue & 0x04) > 0 ?  false : true);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D03__13_OUTPUT, (newValue & 0x08) > 0 ?  false : true);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D04__14_OUTPUT, (newValue & 0x10) > 0 ?  false : true);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D05__15_OUTPUT, (newValue & 0x20) > 0 ?  false : true);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D06__16_OUTPUT, (newValue & 0x40) > 0 ?  false : true);
  stm32_gpiowrite(MEADOW_DIAG_GPIO_D07__17_OUTPUT, (newValue & 0x80) > 0 ?  false : true);

  hcom_utils_dbg_gpio_1led_update(ledOn);
  usleep(50 * 1000);  // Give time to see the results

  // if(ledOn)
  // {
  //   for(int i = 0; i < 5; i++)
  //   {
  //     // flash
  //     hcom_utils_dbg_gpio_1led_update(i % 2 == 0 ? true : false);
  //     usleep(100 * 1000);
  //   }
  //   hcom_utils_dbg_gpio_1led_update(false);
  // }
  // else
  // {
  //   hcom_utils_dbg_gpio_1led_update(false);
  //   usleep(50 * 1000);
  // }
}
#endif

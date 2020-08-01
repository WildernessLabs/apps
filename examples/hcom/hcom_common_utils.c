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

#include <nuttx/userspace.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#define HCOM_COMMON_UTILS_USE_HCOM_PROXY 1

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

static int _syslogMask;
static sem_t _f7syslogSem;    /* Implements event waiting */
static char *_f7syslogTextBuf;

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
int hcom_utils_setup()
{
  _f7syslogTextBuf = malloc(HCOM_PROTOCOL_REQUEST_MAX_SIMPLE_DATA_LEN);
  sem_init(&_f7syslogSem, 0, 1);

#if HCOM_COMMON_UTILS_GPIO_D00_D08_DOUT > 0
  hcom_utils_dbg_gpio_init_d0_d8();
#endif
#if HCOM_COMMON_UTILS_GPIO_A0_MISO_DOUT > 0
  hcom_utils_dbg_gpio_init_a0_miso();
#endif

  return OK;
}

//============================================================================
void hcom_utils_shutdown()
{
  free(_f7syslogTextBuf);
  sem_destroy(&_f7syslogSem);
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
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
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
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
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
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
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
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
  }
  ret = hcom_proxy_set_bbr(regValue & (~value));
  if(ret < 0)
  {
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
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
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
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
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
  }
}

//===================================================================
// Clears the specified bits
void hcom_utils_clear_bbr_bits(uint32_t value)
{
  int ret = hcom_proxy_update_bbr(value, 0);
  if(ret < 0)
  {
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
  }
}

//===================================================================
// Clears then sets the specified bits
void hcom_utils_clear_then_set_bbr_bits(uint32_t clearBits, uint32_t setBits)
{
  int ret = hcom_proxy_update_bbr(clearBits, setBits);
  if(ret < 0)
  {
    hcom_utils_f7syslog(LOG_ERR, "%s:%s()@%d-%s Failed, errno:%d\n");
  }
}

// //===================================================================
// // This is called during startup, before the hcom thread is created,
// // to check if we are running under the QEMU virtualization model.
// // 

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

//==========================================================
// This function sets up the syslog mask as desired
int hcom_utils_syslog_mask_init()
{
  int syslogMaskPrev;
  bool isPowerOnRestart;

#if defined(CONFIG_STM32F7_PWR)
  // Check if this is a reboot or a power-on restart. The MCU on Power-on
  // clears all 32 battery backed registers to 0.
  if(hcom_utils_bbr_read() == 0)
  {
    // Power-on restart
    isPowerOnRestart = true;

    // Set and save the syslog level to the default value
    _syslogMask = LOG_MASK(LOG_EMERG) | LOG_MASK(LOG_ALERT) | LOG_MASK(LOG_CRIT) |
               LOG_MASK(LOG_ERR) | LOG_MASK(LOG_WARNING);
    // This works here because we know that the bits are all zeros
    hcom_utils_set_bbr_bits(_syslogMask);
  }
  else
  {
    // Rebooted - it's safe to use the battery backed registers values
    isPowerOnRestart = false;
    _syslogMask = hcom_utils_read_bbr_and_right_justify(HCOM_BBREG_RESTART_SYSLOG_CONFIG_VALUE_MASK);
  }

  // Save for emergency debugging :-)
  // _syslogMask = LOG_MASK(LOG_EMERG) | LOG_MASK(LOG_ALERT) | LOG_MASK(LOG_CRIT) |
  //             LOG_MASK(LOG_ERR) | LOG_MASK(LOG_WARNING) | LOG_MASK(LOG_NOTICE) | 
  //             LOG_MASK(LOG_INFO);  // | LOG_MASK(LOG_DEBUG);

  // Sets new mask and returns the previous syslog_mask
  syslogMaskPrev = setlogmask(_syslogMask);
  if (syslogMaskPrev < 0)
  {
    hcom_utils_f7syslog(LOG_CRIT, "%s@%d-setlogmask err:0x%08x\n", thisFile, __LINE__, syslogMaskPrev);
    return syslogMaskPrev;    // not old mask be error
  }
#else
  syslog(1, "CONFIG_STM32F7_PWR not defined\n");
  // Without battery backed registers the best we can do is defaults
  isPowerOnRestart = true;
  _syslogMask = LOG_MASK(LOG_EMERG) | LOG_MASK(LOG_ALERT) | LOG_MASK(LOG_CRIT) |
              LOG_MASK(LOG_ERR) | LOG_MASK(LOG_WARNING);
  syslogMaskPrev = setlogmask(_syslogMask);
#endif

// p-m the following should have #if defined(CONFIG_STM32F7_PWR) because it access
// battery backed registers
  // This is an informational syslog message
  char *traceCombo[] = {
    "none",
    "Host",
    "UART1",
    "Host+UART1"};
  // Check the 2 host and uart1 bits
  uint32_t toValue = hcom_utils_read_bbr_and_right_justify(HCOM_BBREG_TRACE_MSG_TO_EITHER_BIT_MASK);
  char *traceDest = traceCombo[toValue];

  // Provide some information that may be useful
  hcom_utils_f7syslog(LOG_INFO, "Meadow %s (%s@%s) %s, Mono:%s, Trace level:0x%02x(was 0x%02x), to:%s, type:%s\n",
        HCOM_DEVICE_INFO_MEADOW_OS_VERSION, __DATE__, __TIME__, 
        isPowerOnRestart ? "power-on restart" :"rebooted",
        hcom_mono_ctrl_is_mono_enabled() ? "Enabled" : "Disabled",
        _syslogMask, syslogMaskPrev, traceDest,
#if defined CONFIG_RAMLOG_SYSLOG
        "ramlog");
#else
        "syslog");
#endif

  return OK;
}

//============================================================================
// For diagnostic use only
void hcom_utils_diag_print_buffer(const uint8_t buffer[], const int bufLen, uint8_t msgPriority)
{
#if HCOM_COMMON_UTILS_DIAG_PRINT_BUFFER > 0

#define HCOM_UTIL_BYTES_PER_LINE 16
#define HCOM_UTIL_LEADING_SPACES 2
#define HCOM_UTIL_HEXADECIMAL_OFFSET (8 + HCOM_UTIL_LEADING_SPACES)
#define HCOM_UTIL_ASCII_OFFSET (57 + HCOM_UTIL_LEADING_SPACES)
#define HCOM_UTIL_DISPLAY_LENGTH (HCOM_UTIL_ASCII_OFFSET + HCOM_UTIL_BYTES_PER_LINE + 3)

  if(bufLen <= 0)
  {
    hcom_utils_f7syslog(msgPriority, "hcom_utils_diag_print_buffer() called with 'bufLen:%d'\n", bufLen);
    return;
  }

  if(buffer == NULL)
  {
    hcom_utils_f7syslog(msgPriority, "hcom_utils_diag_print_buffer() called with 'buffer == NULL'\n");
    return;
  }

  if ((_syslogMask & LOG_MASK(msgPriority)) == 0)
    return;

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
  hcom_utils_f7syslog(msgPriority, lineBuff);
  }

#endif
}

//===================================================================
// This is part of memory saving effort to remove from all the syslog
// messages the need to identify the type of log. It's done here and
// not in 100+ places.
static int hcom_utils_syslog_priority_to_text(int priority, char *textPri)
{
  switch (priority)
  {
  case LOG_EMERG:
    strcpy(textPri, "(Emerg) ");
    break;
  case LOG_ALERT:
    strcpy(textPri, "(Alert) ");
    break;
  case LOG_CRIT:
    strcpy(textPri, "(Crit) ");
    break;
 case LOG_ERR:
    strcpy(textPri, "(Error) ");
    break;
  case LOG_WARNING:
    strcpy(textPri, "(Warn) ");
    break;
  case LOG_NOTICE:
    strcpy(textPri, "(Note) ");
    break;
  case LOG_INFO:
    strcpy(textPri, "(Info) ");
    break;
   case LOG_DEBUG:
    strcpy(textPri, "(Debug) ");
    break;  
  default:
    strcpy(textPri, "(Pri ?) ");
    break;
  }

  return strlen(textPri);
}

//===================================================================
// Builds the string for syslogs
static int hcom_util_build_syslog_string(int priority, FAR const IPTR char * fmtStr, va_list argsList,
        char* finalString, int maxStringLen)
{
  // Adding the prefix here saves memory by removing
  // the text at the start of each message
  char labelPrefix[16];
  int prefixLen = hcom_utils_syslog_priority_to_text(priority, labelPrefix);
  int fmtLength = strlen(fmtStr);

  char *finalFmt = malloc(prefixLen + fmtLength + 1); // room for '\0'
  DEBUGASSERT(finalFmt != NULL);

  memcpy(finalFmt, labelPrefix, prefixLen);
  memcpy(finalFmt + prefixLen, fmtStr, fmtLength + 1); // include fmt's '\0'

  // Create the complete message with prefix
  int stringLen = vsnprintf(finalString, maxStringLen - 1, finalFmt, argsList);

  // The snprintf return is considered to be written completely if and only if the returned value
  // is non-negative and less than buf_size. Otherwise, the string may be truncated.
  DEBUGASSERT(stringLen < HCOM_PROTOCOL_REQUEST_MAX_SIMPLE_DATA_LEN);

  free(finalFmt);
  return stringLen;
}

//=====================================================================
// Wait for the thread writing to exit
static void hcom_utils_f7syslog_takesem(void)
{
  int ret;

  do
    {
      /* Take the semaphore (perhaps waiting) */
      ret = sem_wait(&_f7syslogSem);

      /* The only case that an error should occur here is if the wait was
       * awakened by a signal.
       */
      DEBUGASSERT(ret == OK || ret == -EINTR);
    }
  while (ret == -EINTR);
}

//===================================================================
// This function is for those places where hcom is processing text to
// send to the host PC. If hcom_utils_f7syslog had been used these calls
// will introduce recursion as each syslog will create another syslog.
void hcom_utils_f7syslog_x(int priority, FAR const IPTR char *fmt, ...)
{
#if defined CONFIG_RAMLOG_SYSLOG
  // Can't send to syslog because this will be a recursive. Can't send
  // directly to the host because this also would be recursive too.
  return;
#else
  if ((_syslogMask & LOG_MASK(priority)) == 0)
  {
    return;   // Nothing to do
  }

  // If nuttx is configured for outputting syslogs to the console
  // then this works. But, these cannot be routed to the host PC.
  va_list args;
  va_start(args, fmt);
  vsyslog(priority, fmt, args);
  va_end(args);
#endif
}

//===================================================================
// Use this for syslogs writes that can be routed to host.
void hcom_utils_f7syslog(int priority, FAR const IPTR char *fmt, ...)
{
  // Prevent multiple threads from garbling message
  hcom_utils_f7syslog_takesem();

  if ((_syslogMask & LOG_MASK(priority)) == 0)
  {
    sem_post(&_f7syslogSem);
    return;   // Nothing to do
  }

  va_list args;
  va_start(args, fmt);
  int stringLen = hcom_util_build_syslog_string(priority, fmt, args, _f7syslogTextBuf,
            HCOM_PROTOCOL_REQUEST_MAX_SIMPLE_DATA_LEN);
  va_end(args);  

  // Depending on the nuttx configuration these messages may go to the
  // ramlog, to the serial console or the bit-bucket.
  //
  // If ramlog is configured then the message is sent to syslog only. Nuttx
  // adds a syslog timestamp (if so configured) and puts it in the nuttx
  // ramlog buffer. The hcom_host_trace_ramlog read function reads it
  // from ramlog and forwards the message to the host PC, if requested.
  //
  // If syslog is configured then the call to syslog writes the message to
  // the configured UART for serial output. On return from the syslog call
  // this code (immediately below) forwards it to the host PC, if trace is
  // enabled
  //
  syslog(priority, _f7syslogTextBuf);

#if defined CONFIG_RAMLOG_SYSLOG
  stringLen = stringLen;    // Keep conpiler from warning (I know there's a better way...)
#else
// p-m - removed following because BBR not functional
  stringLen = stringLen;    // p-m - added
  // syslog - check if these should be routed to host.
  // Note: no timestamp for these messages
  // if(hcom_utils_is_bbr_bit_set(HCOM_BBREG_TRACE_MSG_TO_HOST_BIT_MASK))
  {
    // Strip off cr/lf since Meadow.CLI takes care of this
    if(_f7syslogTextBuf[stringLen - 1] == 0x0a || _f7syslogTextBuf[stringLen - 1] == 0x0d)
      stringLen--;
    if(_f7syslogTextBuf[stringLen - 1] == 0x0a || _f7syslogTextBuf[stringLen - 1] == 0x0d)
      stringLen--;
  
    hcom_comms_send_raw_string_msg(HCOM_HOST_REQUEST_TEXT_TRACE_MSG, 0, _f7syslogTextBuf,
            stringLen, thisFile, __LINE__);
  }
#endif
  sem_post(&_f7syslogSem);
}

#if defined (CONFIG_RAMLOG_SYSLOG)
//===================================================================
// hcom_utils_safe_ramlog solves the problem that with ramlog 
// enabled, we can't use syslog while processing the syslog message,
// because it will cause "feedback", every syslog message would
// generated another ramlog message. So, we go directly to the host
// without going to syslog -> ramlog -> host PC.
//===================================================================
void hcom_utils_safe_ramlog(int priority, FAR const IPTR char *fmt,
          va_list args)
{
  // Check priority but otherwise ignore syslog.
  if ((_syslogMask & LOG_MASK(priority)) == 0)
    return;   // Nothing to do
  
  char *_safeRamlogText;
  _safeRamlogText = malloc(HCOM_PROTOCOL_REQUEST_MAX_SIMPLE_DATA_LEN);
  DEBUGASSERT(_safeRamlogText != NULL);

  int stringLen = hcom_util_build_syslog_string(priority, fmt, args, _safeRamlogText,
            HCOM_PROTOCOL_REQUEST_MAX_SIMPLE_DATA_LEN);

  // Note: no time stamp to these messages
  if(_safeRamlogText[stringLen - 1] == 0x0a || _safeRamlogText[stringLen - 1] == 0x0d)
    stringLen--;
  if(_safeRamlogText[stringLen - 1] == 0x0a || _safeRamlogText[stringLen - 1] == 0x0d)
    stringLen--;

// p-m - removed following because sending to host not implemented yet
  // hcom_comms_send_raw_string_msg(HCOM_HOST_REQUEST_TEXT_TRACE_MSG, 0, _safeRamlogText,
  //         stringLen, thisFile, __LINE__);
  free(_safeRamlogText);
}
#endif

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

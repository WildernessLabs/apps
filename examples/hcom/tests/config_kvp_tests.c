/****************************************************************************
 * \apps\examples\hcom\tests\bbreg_tests.c
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
#include <meadow/hcom_upd_shared.h>
#include <meadow/hcom_shared_common.h>

#if HCOM_INCLUDE_CONFIG_FILE_KVP_TESTS > 0

/****************************************************************************
 * Configuration file key value pair tests
 ****************************************************************************/

#define HCOM_TEST_KVP_TEST_BUFFER_LEN 32

//================================================================
void hcom_tests_config_kvp_get(uint32_t userData)
{
/*
; Test config file for ini_example.c and INIReaderTest.cpp

[protocol]             ; Protocol configuration
version=6              ; IPv6

[user]
name = Bob Smith       ; Spaces around '=' are stripped
email = bob@smith.com  ; And comments (like this) ignored
active = true          ; Test a boolean
pi = 3.14159           ; Test a floating point number
*/

  int ret;

  char *sectionName = "user";
  char *keyName = "email";
  char returnValueBuf[HCOM_TEST_KVP_TEST_BUFFER_LEN];
  int returnBufLen = HCOM_TEST_KVP_TEST_BUFFER_LEN;

  syslog(2, "===>>> Entered hcom_tests_config_kvp_get()\n");

// #define MEADOW_CONFIG_ERROR_NO_KEY_FOUND -1
// #define MEADOW_CONFIG_ERROR_CFG_FILE_OPEN -2
// #define MEADOW_CONFIG_ERROR_DEST_BUF_TOO_SMALL -3
// #define MEADOW_CONFIG_ERROR_MEM_ALLOC_ERROR -4

  ret = hcom_via_nx_cfg_kvp_get_value(hcom_via_nx_get_fd(), sectionName,
        keyName, returnValueBuf, returnBufLen);
  if (ret != OK)
  {
  if (ret == MEADOW_CONFIG_ERROR_NO_KEY_FOUND)
    syslog(1, "No matching key found:%d\n", ret);
  else if (ret == MEADOW_CONFIG_ERROR_CFG_FILE_OPEN)
    syslog(1, "File could not be opened:%d\n", ret);
  else if (ret == MEADOW_CONFIG_ERROR_DEST_BUF_TOO_SMALL)
    syslog(1, "Buffer too small, truncated results:%d\n", ret);
  else if (ret == MEADOW_CONFIG_ERROR_MEM_ALLOC_ERROR)
    syslog(1, "Memory allocation error:%d\n", ret);
  else if (ret == MEADOW_CONFIG_ERROR_CFG_LINE_TOO_LONG)
    syslog(1, "Line within file too long:%d\n", ret);
  else if (ret == MEADOW_CONFIG_ERROR_CFG_FILE_READ_ERR)
    syslog(1, "File read error:%d\n", ret);
  else
    syslog(1, "Encountered a parsing error on line:%d\n", ret);
    return;
  }

  syslog(2, "===>>> Returned from KVP test Value':%s', ret:%d\n", returnValueBuf, ret);
}

#endif
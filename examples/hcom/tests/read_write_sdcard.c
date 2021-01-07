/****************************************************************************
 * \apps\examples\hcom\tests\read_write_sdcard.c
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

// Tests the  basic functions of the sd card interface available on the
// Embedded module

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "../hcom_common.h"
#include <nuttx/config.h>
#include <sys/mount.h>

/* Configuration ************************************************************/
#if defined (HCOM_INCLUDE_SD_CARD_TESTS_IN_BUILD)

#define HCOM_TESTS_SD_CARD_DEV_NAME "/dev/mmcsd0"
#define HCOM_TESTS_SD_CARD_MOUNT_POINT "/sdc"
#define HCOM_TESTS_SD_CARD_FILE_NAME "/sdc/sdctest.txt"

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

static int _sd_fd = -1;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
static void hcom_tests_sd_card_start(uint32_t userData);
static void hcom_tests_sd_card_mount(uint32_t userData);
static void hcom_tests_sd_card_open(uint32_t userData);
static void hcom_tests_sd_card_write(uint32_t userData);
static void hcom_tests_sd_card_read(uint32_t userData);
static void hcom_tests_sd_card_delete(uint32_t userData);
static void hcom_tests_sd_card_close(uint32_t userData);

/****************************************************************************
 * Public Functions
 ****************************************************************************/
void hcom_tests_sd_card_entry(uint32_t userData)
{
  switch(userData)
  {
    case 0:
    hcom_tests_sd_card_start(userData);
    break;

    case 1:   // Mount
    hcom_tests_sd_card_mount(userData);
    break;

    case 2:   // Open
    hcom_tests_sd_card_open(userData);
    break;

    case 3:   // Write
    hcom_tests_sd_card_write(userData);
    break;

    case 4:   // Read
    hcom_tests_sd_card_read(userData);
    break;

    case 5:   // Erase file
    hcom_tests_sd_card_delete(userData);
    break;

    case 6:   // Close file
    hcom_tests_sd_card_close(userData);
    break;

    default:
    syslog(2, "Unknown sd card test command:%d\n", userData);
  }
}

//==================================================================
// Mount and open the sdcard
void hcom_tests_sd_card_start(uint32_t userData)
{
  syslog(2, "------------MOUNT--------------\n");
  hcom_tests_sd_card_mount(userData);
  syslog(2, "------------OPEN--------------\n");
  hcom_tests_sd_card_open(userData);
  syslog(2, "------------WRITE--------------\n");
  hcom_tests_sd_card_write(userData);
  // Flush doesn't fix the problem
  // syslog(2, "------------FLUSH--------------\n");
  // hcom_tests_sd_card_flush(userData);
  syslog(2, "------------READ--------------\n");
  hcom_tests_sd_card_read(userData);
  syslog(2, "------------CLOSE--------------\n");
  hcom_tests_sd_card_close(userData);
  syslog(2, "------------DONE--------------\n");
}

//==================================================================
// Mount and open the sdcard
void hcom_tests_sd_card_mount(uint32_t userData)
{
  // e.g. mount("/dev/ram0", "/mnt", "vfat", 0, NULL);  // Needs backing block device
  int ret = mount(HCOM_TESTS_SD_CARD_DEV_NAME, HCOM_TESTS_SD_CARD_MOUNT_POINT, "vfat", 0, NULL);
  if (ret < 0)
  {
    syslog(2, "%s@%d-mount failed for /sdc/sdctestfile.txt, errno:%d\n",
            thisFile, __LINE__, HCOM_TESTS_SD_CARD_FILE_NAME, errno);
  }
  else
  {
    syslog(2, "sd card: mount test Success\n");
  }
}

//==================================================================
// Write some text to the sdcard
void hcom_tests_sd_card_open(uint32_t userData)
{
  if(_sd_fd > -1)
  {
    syslog(2, "sd card _sd_fd: Already open test failed\n", _sd_fd);
    return;
  }
  
  // 777 everyone has read write and execute permission
  // 644 owner has read and write permission, group has read and others have read
  // _sd_fd = open(HCOM_TESTS_SD_CARD_FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0777);
  _sd_fd = open(HCOM_TESTS_SD_CARD_FILE_NAME, O_RDWR | O_CREAT, 0777);
  if (_sd_fd < 0)
  {
    syslog(2, "%s@%d-open failed for %s, errno:%d\n",
            thisFile, __LINE__, HCOM_TESTS_SD_CARD_FILE_NAME, errno);
    return;
  }

  syslog(2, "sd card: open test Success\n");
}

//==================================================================
// Write some text to the sdcard
void hcom_tests_sd_card_write(uint32_t userData)
{
  ssize_t nbytes = 0;
  // 30 chars + /n
  char fileWriteData[] = "Write test of sd card 30 char\n";
  size_t fileWriteSize = strlen(fileWriteData);

  nbytes = write(_sd_fd, fileWriteData, fileWriteSize);
  if (nbytes < 0)
  {
    syslog(2, "%s@%d-failed to write:%s, errno:%d\n",
             thisFile, __LINE__, HCOM_TESTS_SD_CARD_FILE_NAME, errno);
    return;
  }

  if (nbytes < fileWriteSize)
  {
    syslog(2, "%s@%d-'%s' wrote %d of %d bytes\n",
             thisFile, __LINE__, HCOM_TESTS_SD_CARD_FILE_NAME, nbytes, fileWriteSize);
  }
  else
  {
    syslog(2, "%s@%d-sd card:Success-Wrote %d bytes to %s\n", 
              thisFile, __LINE__, nbytes, HCOM_TESTS_SD_CARD_FILE_NAME);
  }
}

//==================================================================
// Read some text to the sdcard
void hcom_tests_sd_card_flush(uint32_t userData)
{
  int ret = fsync(_sd_fd);
  if (ret < 0 )
  {
    syslog(2, "sd card flush test failed:%d errno:%d\n", ret, errno);
  }
  syslog(2, "sd card: flush Success\n");
}

//==================================================================
// Read some text to the sdcard
void hcom_tests_sd_card_read(uint32_t userData)
{
  uint8_t buffer[128];
  ssize_t readReturn;

  readReturn = read(_sd_fd, buffer, 127);
  if (readReturn < 0 )
  {
    syslog(2, "sd card read test failed:%d errno:%d\n", readReturn, errno);
  }
  else if (readReturn == 0)
  {
    // EOF
    syslog(2, "%s@%d read %d bytes (EOF), errno:%d\n",
            thisFile, __LINE__, readReturn, errno);
    return;
  }
  else
  {
    buffer[readReturn] = '\0';
    syslog(2, "sd card: read test read:\n%s\n", buffer);
  }
}

//==================================================================
// Read some text to the sdcard
void hcom_tests_sd_card_delete(uint32_t userData)
{
  int ret = unlink(HCOM_TESTS_SD_CARD_FILE_NAME);
  if (ret < 0)
  {
    syslog(2, "%s@%d-unlink of %s failed, errno %d\n",
             thisFile, __LINE__, HCOM_TESTS_SD_CARD_FILE_NAME, get_errno());
    return;
  }

  syslog(2, "Deleted '%s'\n", HCOM_TESTS_SD_CARD_FILE_NAME);
}

//==================================================================
// Read some text to the sdcard
void hcom_tests_sd_card_close(uint32_t userData)
{
  int ret = close(_sd_fd);

  _sd_fd = 1;

  syslog(2, "Close(%s) ret:%d\n", HCOM_TESTS_SD_CARD_FILE_NAME, ret);
}

#endif

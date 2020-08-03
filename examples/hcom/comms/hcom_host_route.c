/****************************************************************************
 * \examples\hcom\commands\hcom_save_parse_request.c
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
#include "../../../nuttx/configs/stm32f777zit6-meadow/src/hcom_proxy/hcom_proxy_shared.h"

#if defined (CONFIG_HCOM_ESP32_COMMS)
#include "../esp32/hcom_esp32_comms.h"
#endif
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static char *thisFile = __FILE__;

static bool _shutting_down;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int hcom_host_route_setup()
{
  _shutting_down = false;
  return OK;
}

//====================================================================
void hcom_host_route_shutdown()
{
  _shutting_down = true;
}

//========================================================================
// Parse the manditory header. This commands that need other data will
// parse the individual optional header
void hcom_host_route_request_by_type(const uint8_t *recvOrigData, const size_t recvOrigDataSize)
{
  int msgOffset = 0;
  
  uint16_t protocolVersion = recvOrigData[msgOffset] + (recvOrigData[msgOffset + 1] << 8);
  msgOffset += sizeof(uint16_t);

  if(protocolVersion != (uint16_t)HCOM_PROTOCOL_HCOM_VERSION_NUMBER)
  {
    char hostMsg[HCOM_SHORT_HOST_STRING_BUFF_LENGTH];
    int stringLen = snprintf(hostMsg, HCOM_SHORT_HOST_STRING_BUFF_LENGTH, 
        "HCOM received unsupported protocol version:%04x, expected:%04x",
        protocolVersion, (uint16_t)HCOM_PROTOCOL_HCOM_VERSION_NUMBER);

    DEBUGASSERT(stringLen < HCOM_SHORT_HOST_STRING_BUFF_LENGTH);
    hcom_logging_syslog(LOG_ERR, "%s\n", hostMsg);
    hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_ERROR, 0, hostMsg,
            thisFile, __LINE__);
    return;
  }

  uint16_t requestType = recvOrigData[msgOffset] + (recvOrigData[msgOffset + 1] << 8);
  msgOffset += sizeof(uint16_t);

//uint16_t extraData = recvOrigData[msgOffset] + (recvOrigData[msgOffset + 1] << 8);
  msgOffset += sizeof(uint16_t);

  uint32_t userData = recvOrigData[msgOffset] + (recvOrigData[msgOffset + 1] << 8) +
                      (recvOrigData[msgOffset + 2] << 16) + (recvOrigData[msgOffset + 3] << 24);
  msgOffset += sizeof(uint32_t);

  const uint8_t *recvPayload = recvOrigData + msgOffset;
  const size_t recvPayloadSize = recvOrigDataSize - msgOffset;

  switch (requestType)
  {
    case HCOM_MDOW_REQUEST_START_FILE_TRANSFER:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_dnld_proc_begin(recvPayload, recvPayloadSize, userData, requestType);
      break;
      
    // Note: Start file transfer provides the 'Accepted' message and
    // end file transfer the 'Concluded' message
    case HCOM_MDOW_REQUEST_END_FILE_TRANSFER:
      hcom_dnld_proc_end(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_DELETE_FILE_BY_NAME:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_dnld_write_del_remove_file_start(recvPayload, recvPayloadSize, userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    //-------------------------------------------------
#if defined (CONFIG_HCOM_ESP32_COMMS)
    // ESP32 follow
    case HCOM_MDOW_REQUEST_START_ESP_FILE_TRANSFER:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_dnld_proc_begin(recvPayload, recvPayloadSize, userData, requestType);
      break;

    // Note: Start file transfer provides the 'Accepted' message and
    // end file transfer the 'Concluded' message
    case HCOM_MDOW_REQUEST_END_ESP_FILE_TRANSFER:
      hcom_dnld_proc_end(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;
    
    case HCOM_MDOW_REQUEST_READ_ESP_MAC_ADDRESS:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_esp32_exec_read_esp32_mac(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_RESTART_ESP32:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_esp32_exec_restart_esp32(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;
#endif

    //---------------------------------------------------
    case HCOM_MDOW_REQUEST_VERIFY_ERASED_FLASH:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_proxy_forward_cmd_to_proxy(requestType, HCOM_PROXY_CLI_CMD_VERIFY_EX_FLASH, userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

// p-m These are not needed
//       // Partitions the entire flash chip with the number of partitions that
//       // are defined by userData.
//     case HCOM_MDOW_REQUEST_PARTITION_FLASH_FS:
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
// //       hcom_exec_flash_fs_partition(userData);
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
//       break;

//       // Mount the file system for testing.
//     case HCOM_MDOW_REQUEST_MOUNT_FLASH_FS:
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
// //       hcom_exec_flash_fs_mount(userData);
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
//       break;

//     case HCOM_MDOW_REQUEST_FORMAT_FLASH_FILE_SYS:
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
// //       hcom_exec_flash_fs_format(userData);
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
//       break;

//     case HCOM_MDOW_REQUEST_INITIALIZE_FLASH_FS:
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
// //       hcom_exec_flash_fs_initialize(userData);
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
//       break;

//     case HCOM_MDOW_REQUEST_CREATE_ENTIRE_FLASH_FS:
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
// //       hcom_exec_flash_fs_create(userData);
//       hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);

    case HCOM_MDOW_REQUEST_CHANGE_TRACE_LEVEL:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_misc_rqst_change_trace_level(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_LIST_PARTITION_FILES:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_list_files_in_partition(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_LIST_PART_FILES_AND_CRC:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_list_files_and_crc_in_partition(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_MONO_RUN_STATE:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
//       hcom_mono_rqst_is_mono_enabled(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_GET_DEVICE_INFORMATION:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_misc_rqst_get_device_info(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_BULK_FLASH_ERASE:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_proxy_forward_cmd_to_proxy(requestType, HCOM_PROXY_CLI_CMD_ERASE_EX_FLASH, userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    // The following commands send the HCOM_HOST_REQUEST_TEXT_CONCLUDED message when Meadow restarts
    case HCOM_MDOW_REQUEST_RESET_PRIMARY_MCU:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_proxy_forward_cmd_to_proxy(requestType, HCOM_PROXY_CLI_CMD_RESTART_MEADOW, userData);
      break;

    case HCOM_MDOW_REQUEST_PART_RENEW_FILE_SYS:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_proxy_forward_cmd_to_proxy(requestType, HCOM_PROXY_CLI_CMD_RENEW_FILE_SYS, userData);
      break;

// NOT IMPLEMENTED
    case HCOM_MDOW_REQUEST_ENTER_DFU_MODE:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_misc_rqst_enter_dfu_mode(userData);   // Forces restart
      break;

    case HCOM_MDOW_REQUEST_MONO_DISABLE:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_mono_rqst_disable_mono(userData);   // Forces restart
      break;

    case HCOM_MDOW_REQUEST_MONO_ENABLE:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_mono_rqst_enable_mono(userData);   // Forces restart
      break;

    case HCOM_MDOW_REQUEST_MONO_FLASH:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_proxy_forward_cmd_to_proxy(requestType, HCOM_PROXY_CLI_CMD_MONO_FLASH, userData);
      break;

    case HCOM_MDOW_REQUEST_NO_TRACE_TO_HOST:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_trace_do_not_send_trace_to_host(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_SEND_TRACE_TO_HOST:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_trace_send_trace_to_host(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_NO_TRACE_TO_UART:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_trace_do_not_send_trace_to_uart1(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_SEND_TRACE_TO_UART:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_trace_send_trace_to_uart1(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_HOST_REQUEST_MONO_DEBUGGER_MSG:
      // Accepted and concluded not needed here!
//       hcom_remote_dbg_recv_host_send_to_mono(recvPayload, recvPayloadSize, userData);
      break;

    case HCOM_MDOW_REQUEST_ENABLE_DISABLE_NSH:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_diag_misc_launch_nsh(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_DEVELOPER_1:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
//       proc_dnld_rqst_testing_developer_1(userData);
      hcom_proxy_forward_cmd_to_proxy(requestType, HCOM_PROXY_CLI_CMD_DEVELOPER_1, userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_DEVELOPER_2:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
//       proc_dnld_rqst_testing_developer_2(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_DEVELOPER_3:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
//       proc_dnld_rqst_testing_developer_3(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_DEVELOPER_4:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
      hcom_list_all_dev_dir_and_files_start();
//       proc_dnld_rqst_testing_developer_4(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

#if defined(CONFIG_HCOM_MTD_STRESS_TEST)
    case HCOM_MDOW_REQUEST_S25FL_QSPI_INIT:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
//       proc_dnld_rqst_testing_flash_qspi_init(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_S25FL_QSPI_WRITE:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
//       proc_dnld_rqst_testing_flash_qspi_write(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;

    case HCOM_MDOW_REQUEST_S25FL_QSPI_READ:
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_ACCEPTED, 0, thisFile, __LINE__);
//       proc_dnld_rqst_testing_flash_qspi_read(userData);
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
      break;
#endif

    default:
    {
      char hostMsg[HCOM_SHORT_HOST_STRING_BUFF_LENGTH];
      int stringLen = snprintf(hostMsg, HCOM_SHORT_HOST_STRING_BUFF_LENGTH, "Unknown cmd:0x%04x received", requestType);

      DEBUGASSERT(stringLen < HCOM_SHORT_HOST_STRING_BUFF_LENGTH);
      hcom_host_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_REJECTED, 0, hostMsg,
              thisFile, __LINE__);

      hcom_logging_syslog(LOG_ERR, "%s@%d-Received unsupported cmd:0x%04x\n",
             thisFile, __LINE__, requestType);
      hcom_logging_utils_print_buffer(recvOrigData, recvOrigDataSize, LOG_ERR);
      
      hcom_host_send_header_msg(HCOM_HOST_REQUEST_TEXT_CONCLUDED, 0, thisFile, __LINE__);
    }
  }
}

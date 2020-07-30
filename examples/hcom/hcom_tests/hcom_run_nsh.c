


#include "../hcom_common.h"

#if HCOM_INCLUDE_TESTS_NSH_AVAILABLE > 0

int nsh_main(int argc, char *argv[]);

static char *thisFile = __FILE__;
static int nsh_pid;
static bool nsh_enabled = false;

//=======================================================================================
// CURRENTLY CALLED VIA DEVELOPER 3
// userData = 1 enables and userData = 0 disables
void hcom_exec_rqst_misc_enable_disable_nsh(uint32_t userData)
{  
  char *sendMsgToHost;

  if(nsh_enabled)
  {
    sendMsgToHost = "NSH already enabled";
    hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, sendMsgToHost,
            thisFile, __LINE__);
    return;
  }

  if(userData == 1)
  {
    // Create a unique task for NSH
    nsh_pid = task_create("init", 100,
                        2048,
                        (main_t)nsh_main,
                        (FAR char * const *)NULL);

    DEBUGASSERT(nsh_pid > 0);
    nsh_enabled = true;
  }
  else
  {
    sendMsgToHost = "NSH disable not implemented";
    hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, sendMsgToHost,
            thisFile, __LINE__);
    return;
  }
  
  sendMsgToHost = "NSH now enabled";
  hcom_comms_send_simple_string_msg(HCOM_HOST_REQUEST_TEXT_INFORMATION, 0, sendMsgToHost,
          thisFile, __LINE__);
}
#endif
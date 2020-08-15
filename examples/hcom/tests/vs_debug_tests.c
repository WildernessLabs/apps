/****************************************************************************
 * \examples\hcom\hcom_test\hcom_vs_debug.c
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
// One of the developer CLI commands lands here. The userData determines what
// to do.
int MonoVsRemoteDebugTests(uint32_t userData)
{
  return OK;
}

#if HCOM_MONO_DEBUGGING_TEST_CODE > 1
int 
//====================================================
// Called mono_main
int MonoDebugTestConnect()
{
  struct sockaddr_un myaddr;
  socklen_t addrlen;
  int ret;

  _sockfd = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (_sockfd < 0)
  {
    syslog(LOG_ERR, "M->client:socket creation failed _sockfd:%d errno:%d\n", _sockfd, errno);
    goto errout_with_nothing;
  }

  /* Connect the socket to the server */
  addrlen = strlen(HCOM_REMOTE_DBG_SOCKET_NAME);
  if (addrlen > UNIX_PATH_MAX - 1)
    addrlen = UNIX_PATH_MAX - 1;

  myaddr.sun_family = AF_LOCAL;
  strncpy(myaddr.sun_path, HCOM_REMOTE_DBG_SOCKET_NAME, addrlen);
  myaddr.sun_path[addrlen] = '\0';
  addrlen += sizeof(sa_family_t) + 1;

  syslog(0, "M->client: Connect to %s...\n", HCOM_REMOTE_DBG_SOCKET_NAME);

  int attemptCnt = 0;
  do
  {
    attemptCnt++;
    ret = connect(_sockfd, (struct sockaddr *)&myaddr, addrlen);
    if (ret < 0)
    {
      syslog(LOG_INFO, "M->client:connect failed retry in 1 sec. ret: %d, errno:%d attempted %d\n",
       ret, errno, attemptCnt);
      sleep(1);
    }
  } while(ret < 0);

  syslog(LOG_INFO, "M->client:hcom-d connect attempted %d SUCCESSFUL\n", attemptCnt);
  usleep(50 * 1000);
  _connected = true;  
  return OK;

errout_with_nothing:
  return -1;
}

//==========================================================
int MonoDebugTestSend(uint8_t *sendBuffer, int sendSize)
{
  int nbytessent;
  
  /* Then send one message */
  nbytessent = send(_sockfd, sendBuffer, sendSize, 0);
  if (nbytessent < 0)
  {
    syslog(0, "M->client:send failed: %d\n", errno);
    goto errout_with_socket;
  }
  else if (nbytessent != sendSize)
  {
    syslog(0, "M->client:Bad send length: %d Expected: %d\n", nbytessent, sendSize);
    goto errout_with_socket;
  }

  syslog(0, "M->client:Sent %d bytes to hcom-d\n", sendSize);
  return OK;

errout_with_socket:
  syslog(0, "M->client:Exit error\n");
  return 1;
}

//-------------------------------------------------------
// This call blocks
int MonoDebugTestReceive(uint8_t *recvBuffer)
{
  int nbytesrecvd;  
  
  syslog(0, "M->client:Waiting to receiving from hcom-d\n");
  nbytesrecvd = recv(_sockfd, recvBuffer, MONO_DEBUG_TEST_RECV_BUFF_SIZE, 0);
  if (nbytesrecvd < 0)
  {
    syslog(0, "M->client:recv failed: %d\n", errno);
    goto errout_with_socket;
  }
  else if (nbytesrecvd == 0)
  {
    syslog(0, "M->client:The server closed the connection\n");
    goto errout_with_socket;
  }

  syslog(0, "M->client:Received %d bytes\n", nbytesrecvd);
  
  //close(_sockfd);
  return nbytesrecvd;

errout_with_socket:
  syslog(0, "M->client:RECEIVE Exit error\n");
  return -1;
}

//---------------------------------------------------
// Receives and echos back to host
static int MonoDebugTestExecute(void)
{
  int ret;
  FAR uint8_t *inbuf;
  FAR uint8_t *outbuf;

  outbuf = (uint8_t*)malloc(MONO_DEBUG_TEST_RECV_BUFF_SIZE);
  if (outbuf == NULL)
  {
    syslog(0, "M->client:failed to allocate outbuf %d long must exit\n", MONO_DEBUG_TEST_RECV_BUFF_SIZE);
    exit(1);
  }

  inbuf  = (uint8_t*)malloc(MONO_DEBUG_TEST_RECV_BUFF_SIZE);
  if (inbuf == NULL)
  {
    syslog(0, "M->client:failed to allocate inbuf %d long must exit\n", MONO_DEBUG_TEST_RECV_BUFF_SIZE);
    exit(1);
  }

  syslog(0, "M->client: inbuf and outbuf allocated\n");
  
  int xmitCount = 0;

  while(true)
  {
    ret = MonoDebugTestConnect();
    if(ret < 0)
    {
      syslog(0, "M->client:Connection could not be made\n");
      return -1;
    }
    
    // Loop forever
    do
    {
      xmitCount++;

      // Blocking call
      ret = MonoDebugTestReceive(inbuf);
      if(ret < 0)
      {
        syslog(0, "M->client:Receive failed\n");
        continue;
      }

      // This is an echo client so we set whatever we receive
      // Send
      ret = MonoDebugTestSend(inbuf, ret);
      if(ret < 0)
      {
        syslog(0, "M->client:Send failed\n");
      }
    } while(xmitCount % 5 != 0);
    
    syslog(0, "M->client:Closing socket. Will reconnect %d.\n", xmitCount);
    close(_sockfd);
  }

  exit(1);
}
#endif

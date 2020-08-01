############################################################################
# apps/examples/hcom/Makefile
#
#   Copyright (C) 2008, 2010-2013 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

-include $(TOPDIR)/Make.defs

# hcom built-in application info

CONFIG_EXAMPLES_HCOM_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_EXAMPLES_HCOM_STACKSIZE ?= 2048

APPNAME = hcom

PRIORITY  = $(CONFIG_EXAMPLES_HCOM_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_HCOM_STACKSIZE)

# hcom example

HCOM_DIR = ../../../hcom/
ASRCS =
CSRCS = hcom_common_utils.c \
	hcom_mono_control.c \
	hcom_proxy_access.c \
	comms/hcom_comms_receive.c \
	comms/hcom_comms_send.c \
	comms/hcom_comms_support.c \
	diag/hcom_diag_misc.c \
	diag/hcom_diag_trace_ramlog.c \
	file_access/hcom_dnld_process_file.c \
	file_access/hcom_dnld_write_and_del.c \
	file_access/hcom_list_files.c \
	hcom_tests/hcom_bbr_tests.c \
	host_rqsts/hcom_misc_requests.c \
	host_rqsts/hcom_proc_requests.c \
	host_rqsts/hcom_route_requests.c

MAINSRC = hcom_startup_manager.c

CONFIG_EXAMPLES_HCOM_PROGNAME ?= hcom$(EXEEXT)
PROGNAME = $(CONFIG_EXAMPLES_HCOM_PROGNAME)

include $(APPDIR)/Application.mk

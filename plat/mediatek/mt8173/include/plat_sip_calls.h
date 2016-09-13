/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PLAT_SIP_CALLS_H__
#define __PLAT_SIP_CALLS_H__

/*******************************************************************************
 * Plat SiP function constants
 ******************************************************************************/
#define MTK_PLAT_SIP_NUM_CALLS			13

#define MTK_SIP_PWR_ON_MTCMOS			0x82000402
#define MTK_SIP_PWR_OFF_MTCMOS			0x82000403
#define MTK_SIP_PWR_MTCMOS_SUPPORT		0x82000404
#define MTK_SIP_SET_HDCP_KEY_NUM		0x82000405
#define MTK_SIP_CLR_HDCP_KEY			0x82000406
#define MTK_SIP_SET_HDCP_KEY_EX			0x82000407

#define MTK_SIP_GET_CCI_REVISION		0x82000410
#define MTK_SIP_CCI_CONTROL_OVERRIDE		0x82000411
#define MTK_SIP_CCI_SPECULATION_CONTROL		0x82000412
#define MTK_SIP_CCI_SECURE_ACCESS		0x82000413
#define MTK_SIP_CCI_STATUS			0x82000414
#define MTK_SIP_CCI_IMPRECISE_ERROR		0x82000415

#define MTK_SIP_CCI_SLAVE_REG			0x82000420

#endif /* __PLAT_SIP_CALLS_H__ */

/*
 * libmm-log
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Sangchul Lee <sc11.lee@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef __MM_LOG_H__
#define  __MM_LOG_H__

#ifdef __cplusplus
    extern "C" {
#endif

/**
 * Enumerations for log direction
 */
typedef enum  {
    LOG_DIRECTION_NONE = 0,
    LOG_DIRECTION_CONSOLE,
    LOG_DIRECTION_FILE,
    LOG_DIRECTION_SYSLOG,
    LOG_DIRECTION_SYSTEM,
    LOG_DIRECTION_VIEWER,
    LOG_DIRECTION_CNT,
}log_direction_t;

/**
 * Enumerations for log owner
 */
typedef enum {
    LOG_NONE       = 0x00000000,
    LOG_AVAUDIO    = 0x00000001,
    LOG_AVVIDEO    = 0x00000002,
    LOG_AVCAMERA   = 0x00000004,
    LOG_PLAYER     = 0x00000008,
    LOG_CAMCORDER  = 0x00000010,
    LOG_SOUND      = 0x00000020,
    LOG_FILE       = 0x00000040,
    LOG_MHAL       = 0x00000080, /* WILL BE REMOVED */
    LOG_IMAGE      = 0x00000100,
    LOG_COMMON     = 0x00000200,
    LOG_AVSINK     = 0x00000400,
    LOG_AVSRC      = 0x00000800,
    LOG_SOUNDSVR   = 0x00001000,
    LOG_SOUNDRUN   = 0x00002000,
    LOG_SOUNDCODEC = 0x00004000,
    LOG_GSTPLUGIN  = 0x00008000,
    LOG_PLATFORM   = 0x00010000,
    LOG_CODEC      = 0x00020000,
    LOG_SERVER	   = 0x00040000,
    LOG_MEDIACALL  = 0x00080000,
    LOG_SESSIONMGR = 0x00100000,
    LOG_RADIO	   = 0x00200000,
    LOG_TRANSCODE   = 0x00300000,
    LOG_ALL        = 0xFFFFFFFF,
}log_owner_t;

/**
 * Enumerations for log class
 */
typedef enum {
    LOG_CLASS_NONE      = 0x00,
    LOG_CLASS_INFO      = 0x01,
    LOG_CLASS_WARNING   = 0x02,
    LOG_CLASS_ERR       = 0x04,
    LOG_CLASS_CRITICAL  = 0x08,
    LOG_CLASS_ALL       = 0xFF,
}log_class_t;

#if defined(MM_DEBUG_FLAG)
/**
 * This function print log.
 *
 * @param   owner       [in]    owner of log message.
 * @param   class       [in]    class of log message.
 * @param   msg         [in]    message to print.
 *
 * @remark  Print log message. Similar to printf except owner and class.
 * @see     log_print_rel
 */
#define log_print_dbg(owner, class, msg, args...) log_print_rel((owner), (class), (msg), ##args)

/**
 * This function assert condition.
 *
 * @param   condition       [in]    condition for check
 *
 * @remark  If condition is not true, system is aborted. Same to assert function.
 * @see     log_assert_rel
 */
#define log_assert_dbg(condition)  log_assert_rel((condition))

#else
#define log_print_dbg(owner, class, msg, args...)
#define log_assert_dbg(condition)
#endif

#ifndef USE_DLOG
#define USE_DLOG
#endif

#ifdef USE_DLOG

#include <dlog.h>

#define __log_by_owner(owner,class, msg, args...) \
	do { \
		switch(owner) { \
		case LOG_AVAUDIO    : SLOG (class, "MMFW_AVAUDIO", msg, ##args); break; \
		case LOG_AVVIDEO    : SLOG (class, "MMFW_AVVIDEO", msg, ##args); break; \
		case LOG_AVCAMERA   : SLOG (class, "MMFW_AVCAMERA", msg, ##args); break; \
		case LOG_PLAYER     : SLOG (class, "MMFW_PLAYER", msg, ##args); break; \
		case LOG_CAMCORDER  : SLOG (class, "MMFW_CAMCORDER", msg, ##args); break; \
		case LOG_FILE       : SLOG (class, "MMFW_FILE", msg, ##args); break; \
		case LOG_IMAGE      : SLOG (class, "MMFW_IMAGE", msg, ##args); break; \
		case LOG_COMMON     : SLOG (class, "MMFW_COMMON", msg, ##args); break; \
		case LOG_SOUND      : SLOG (class, "MMFW_SOUND", msg, ##args); break; \
		case LOG_SOUNDSVR   : SLOG (class, "MMFW_SOUNDSVR", msg, ##args); break; \
		case LOG_SOUNDRUN   : SLOG (class, "MMFW_SOUNDRUN", msg, ##args); break; \
		case LOG_SOUNDCODEC : SLOG (class, "MMFW_SOUNDCODEC", msg, ##args); break; \
		case LOG_SERVER     : SLOG (class, "MMFW_SERVER", msg, ##args); break; \
		case LOG_MEDIACALL  : SLOG (class, "MMFW_MEDIACALL", msg, ##args); break; \
		case LOG_SESSIONMGR : SLOG (class, "MMFW_SESSIONMGR", msg, ##args); break; \
		case LOG_RADIO	    : SLOG (class, "MMFW_RADIO", msg, ##args); break; \
		case LOG_TRANSCODE	    : SLOG (class, "MMFW_TRANSCODE", msg, ##args); break; \
		default             : SLOG (class, "MMFW_UNKNOWN", msg, ##args); break; \
		} \
	} while(0)

#define log_print_rel(owner, class, msg, args...)  \
	do {	\
		if (class == LOG_CLASS_INFO) { \
			__log_by_owner(owner,LOG_DEBUG,msg,##args); \
		} else if (class == LOG_CLASS_WARNING) {	\
			__log_by_owner(owner,LOG_WARN,msg,##args); \
		} else if (class == LOG_CLASS_ERR) {	\
			__log_by_owner(owner,LOG_ERROR,msg,##args); \
		} else if (class == LOG_CLASS_CRITICAL) {	\
			__log_by_owner(owner,LOG_FATAL,msg,##args); \
		} else { \
			__log_by_owner(owner,LOG_INFO,msg,##args); \
		} \
    } while(0)

#define  log_assert_rel(condition) \
	do {	\
    	static char msg[256];	\
		if (!(condition))	{ \
			snprintf(msg, 255, "Assertion Fail at %s (%d)\n", __FILE__, __LINE__);	\
			log_print_rel(0, LOG_CLASS_CRITICAL, msg, NULL);	\
			abort();	\
		}	\
    } while(0)

#else
/**
 * This function print log.
 *
 * @param   owner       [in]    owner of log message.
 * @param   class       [in]    class of log message.
 * @param   msg         [in]    message to print.
 *
 * @remark  Print log message. Similar to printf except owner and class.
 * @see     log_print_dbg
 */
#define log_print_rel(owner, class, msg, args...)  _log_print_rel((owner), (class), (msg), ##args)

/**
 * This function assert condition.
 *
 * @param   condition       [in]    condition for check
 *
 * @remark  If condition is not true, system is aborted. Same to assert function.
 * @see     log_assert_dbg
 */
#define  log_assert_rel(condition)  _log_assert_rel((condition), __FILE__, __LINE__)

/* Do not directly use. */
void _log_print_rel(log_owner_t vowner, log_class_t vclass, char *msg, ...);
void _log_assert_rel(int condition, char *str, int line);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MM_LOG_H__ */


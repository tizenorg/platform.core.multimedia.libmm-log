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
    LOG_AVSYSTEM    = 0x00000001,
    LOG_SOUNDSERVER = 0x00000002,
    LOG_FOCUSSERVER = 0x00000004,
    LOG_PLAYER     = 0x00000008,
    LOG_CAMCORDER  = 0x00000010,
    LOG_SOUND      = 0x00000020,
    LOG_FILEINFO       = 0x00000040,
    LOG_UTILITY      = 0x00000100,
    LOG_COMMON     = 0x00000200,
    LOG_MEDIA_CALL  = 0x00080000,
    LOG_SESSION = 0x00100000,
    LOG_RADIO	   = 0x00200000,
    LOG_TRANSCODE   = 0x00300000,
    LOG_STRRECORDER = 0x00400000,
    LOG_SESSIONMGR = 0x00800000,
    LOG_MIRACAST = 0x01000000,
    LOG_WFD = 0x02000000,
    LOG_JPEG_MSM8974 = 0x03000000,
    LOG_UCM = 0x04000000,
    LOG_ALL        = 0xFFFFFFFF,
}log_owner_t;

/**
 * Enumerations for log class
 */
typedef enum {
    LOG_CLASS_NONE      = 0x00,
    LOG_CLASS_VERBOSE      = 0x01,
    LOG_CLASS_DEBUG      = 0x02,
    LOG_CLASS_INFO      = 0x04,
    LOG_CLASS_WARNING   = 0x08,
    LOG_CLASS_ERR       = 0x10,
    LOG_CLASS_CRITICAL  = 0x20,
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
#define log_print_dbg(owner, class, msg, args...) log_print_rel(owner, class, msg, ##args)

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

#define _SLOG(class, tag , format, arg...) \
	SLOG(class, tag ,format, ##arg)

#define _SECURE_SLOG(class, tag , format, arg...) \
	SECURE_SLOG(class, tag ,format, ##arg)

#include <dlog.h>

#define mm_log_by_owner(owner,class, msg, args...) \
	do { \
		switch(owner) { \
		case LOG_AVSYSTEM    : _SLOG (class, "AVSYSTEM", msg, ##args); break; \
		case LOG_SOUNDSERVER : _SLOG (class, "SOUND_SERVER", msg, ##args); break; \
		case LOG_FOCUSSERVER : _SLOG (class, "FOCUS_SERVER", msg, ##args); break; \
		case LOG_PLAYER     : _SLOG (class, "MM_PLAYER", msg, ##args); break; \
		case LOG_CAMCORDER  : _SLOG (class, "MM_CAMCORDER", msg, ##args); break; \
		case LOG_SOUND      : _SLOG (class, "MM_SOUND", msg, ##args); break; \
		case LOG_FILEINFO       : _SLOG (class, "MM_FILEINFO", msg, ##args); break; \
		case LOG_UTILITY      : _SLOG (class, "MM_UTILITY", msg, ##args); break; \
		case LOG_COMMON     : _SLOG (class, "MM_COMMON", msg, ##args); break; \
		case LOG_MEDIA_CALL  : _SLOG (class, "MM_MEDIA_CALL", msg, ##args); break; \
		case LOG_SESSION : _SLOG (class, "MM_SESSION", msg, ##args); break; \
		case LOG_RADIO    : _SLOG (class, "MM_RADIO", msg, ##args); break; \
		case LOG_TRANSCODE    : _SLOG (class, "MM_TRANSCODE", msg, ##args); break; \
		case LOG_SESSIONMGR   : _SLOG (class, "MM_SESSIONMGR", msg, ##args); break; \
		case LOG_MIRACAST   : _SLOG (class, "MM_MIRACAST", msg, ##args); break; \
		case LOG_WFD   : _SLOG (class, "MM_WFD", msg, ##args); break; \
		case LOG_JPEG_MSM8974   : _SLOG (class, "MM_JPEG_MSM8974", msg, ##args); break; \
		case LOG_STRRECORDER : _SLOG (class, "MM_STREAMRECORDER", msg, ##args); break; \
		case LOG_UCM : _SLOG (class, "UCM", msg, ##args); break; \
		default             : _SLOG (class, "MM_UNKNOWN", msg, ##args); break; \
		} \
	} while(0)

#define mm_secure_log_by_owner(owner,class, msg, args...) \
	do { \
		switch(owner) { \
		case LOG_AVSYSTEM    : _SECURE_SLOG (class, "AVSYSTEM", msg, ##args); break; \
		case LOG_SOUNDSERVER : _SECURE_SLOG (class, "SOUND_SERVER", msg, ##args); break; \
		case LOG_PLAYER     : _SECURE_SLOG (class, "MM_PLAYER", msg, ##args); break; \
		case LOG_CAMCORDER  : _SECURE_SLOG (class, "MM_CAMCORDER", msg, ##args); break; \
		case LOG_SOUND      : _SECURE_SLOG (class, "MM_SOUND", msg, ##args); break; \
		case LOG_FILEINFO       : _SECURE_SLOG (class, "MM_FILEINFO", msg, ##args); break; \
		case LOG_UTILITY      : _SECURE_SLOG (class, "MM_UTILITY", msg, ##args); break; \
		case LOG_COMMON     : _SECURE_SLOG (class, "MM_COMMON", msg, ##args); break; \
		case LOG_MEDIA_CALL  : _SECURE_SLOG (class, "MM_MEDIA_CALL", msg, ##args); break; \
		case LOG_SESSION : _SECURE_SLOG (class, "MM_SESSION", msg, ##args); break; \
		case LOG_RADIO    : _SECURE_SLOG (class, "MM_RADIO", msg, ##args); break; \
		case LOG_TRANSCODE    : _SECURE_SLOG (class, "MM_TRANSCODE", msg, ##args); break; \
		case LOG_SESSIONMGR   : _SECURE_SLOG (class, "MM_SESSIONMGR", msg, ##args); break; \
		case LOG_MIRACAST   : _SECURE_SLOG (class, "MM_MIRACAST", msg, ##args); break; \
		case LOG_WFD   : _SECURE_SLOG (class, "MM_WFD", msg, ##args); break; \
		case LOG_STRRECORDER : _SECURE_SLOG (class, "MM_STREAMRECORDER", msg, ##args); break; \
		case LOG_UCM : _SECURE_SLOG (class, "UCM", msg, ##args); break; \
		default             : _SECURE_SLOG (class, "MM_UNKNOWN", msg, ##args); break; \
		} \
	} while(0)

#ifdef __cplusplus
}
#endif

#endif /* __MM_LOG_H__ */


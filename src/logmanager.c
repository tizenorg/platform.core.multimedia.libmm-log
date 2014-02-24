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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mm_log.h"

#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdarg.h>
#include <pthread.h>

//#define	ENABLE_DEBUG_MESSAGE

//#include <iniparser/iniparser.h> //:-:hyunil46.park 090203
#include <iniparser.h> //:+:hyunil46.park 090203

#ifdef ENABLE_DEBUG_MESSAGE	
/* debug-message-framework */
#include <mid-map.h>
#include <debug-message-sys-assert.h>
#include <debug-message.h>
#endif

#ifndef USE_KERNEL_OBJECT
#define LOG_SHARED_MMAP         "/logmanager"
#else
#define LOG_SHARED_LOGKEY       "/usr/bin/mmlogviewer"
#define LOG_SHARED_PREFIX       "0x5648"
#endif

#include <assert.h>

#define LOG_DIRECTION_KEY       "setting:direction"
#define LOG_OWNER_FILTER_KEY    "setting:ownerfilter"
#define LOG_CLASS_FILTER_KEY    "setting:classfilter"
#define LOG_LOGFILE_KEY         "setting:logfile"
#define LOG_DEFAULT_FILE_PATH   "/tmp/mmlogs.log"

#define LOG_COLOR_INFO_KEY      "color:classinfomation"
#define LOG_COLOR_WARNING_KEY   "color:classwarning"
#define LOG_COLOR_ERROR_KEY     "color:classerror"
#define LOG_COLOR_CRITICAL_KEY  "color:clsascritical"
#define LOG_COLOR_ASSERT_KEY    "color:classassert"

#define FIFO_PATH               "/tmp/logman.fifo"
#define BUF_LEN 4096

#define log_gettid() (long int)syscall(__NR_gettid)

#define __owner_res_index(owner, index) \
    switch(owner) { \
    case LOG_NONE       : index = 1; break; \
    case LOG_AVAUDIO    : index = 2; break; \
    case LOG_AVVIDEO    : index = 3; break; \
    case LOG_AVCAMERA   : index = 4; break; \
    case LOG_PLAYER     : index = 5; break; \
    case LOG_CAMCORDER  : index = 6; break; \
    case LOG_SOUND      : index = 7; break; \
    case LOG_FILE       : index = 8; break; \
    case LOG_MHAL       : index = 9; break; \
    case LOG_IMAGE      : index = 10; break; \
    case LOG_COMMON     : index = 11;break; \
    case LOG_AVSINK     : index = 12;break; \
    case LOG_AVSRC      : index = 13;break; \
    case LOG_SOUNDSVR   : index = 14;break; \
    case LOG_SOUNDRUN   : index = 15;break; \
    case LOG_SOUNDCODEC : index = 16;break; \
    case LOG_GSTPLUGIN  : index = 17;break; \
    case LOG_PLATFORM   : index = 18;break; \
    case LOG_CODEC      : index = 19;break; \
    case LOG_SERVER     : index = 20;break; \
    case LOG_MEDIACALL  : index = 21;break; \
    case LOG_SESSIONMGR : index = 22;break; \
    default             : index = 0; break; \
    }

static char * owner_res[] =
{
    "Unknown", /* NONE */
    "Log manager",
    "AV Audio",
    "AV Video",
    "AV Camera",
    "Player",
    "Camcorder",
    "Sound",
    "File",
    "MHAL",
    "Image",
    "Common",
    "AV Sink",
    "AV Source",
    "Sound Svr",
    "Sound Run",
    "Sound Codec",
    "GST Plugin",
    "Platform",
    "Codec",
    "Server",
    "Media Call",
    "SessionMgr"
 };

static char * class_res[] =
{
    "ASSERTION", /* NONE */
    "Info",
    "Warning",
    NULL,
    "Error",
    NULL,
    NULL,
    NULL,
    "Critical"
};

typedef struct _logmanager_obj
{
    int use_default;
    int owner_mask;
    int class_mask;
    int direction;
    char colors[9];
    int fd_count;
    char filename[1025];
} logmanager_obj_t;

static void __conv_color(int index, char *colors, const char* str);
static int __create_shm(const char *path, const int prefix, const int size);
static int __remove_shm(const char *path, const int prefix, const int size);
static void* __get_shm(const char *path, const int prefix, const int size);
static logmanager_obj_t* __get_data(void);
typedef void (*print_internal_t)(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata);
static void print_dummy(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata);
#ifdef ENABLE_DEBUG_MESSAGE
static void print_system(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata);
#endif
static void print_console(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata);
static void print_file(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata);
static void print_syslog(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata);
static void print_viewer(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata);

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static print_internal_t print_list[6] =
{ print_dummy, print_console, print_file, print_syslog, 
	#ifdef ENABLE_DEBUG_MESSAGE
	print_system, 
	#else
	print_console, 
	#endif
	print_viewer };
static char work_buf1[BUF_LEN+1];
static char work_buf2[BUF_LEN+1];
static int logfd = -1;
static int logfd_count = 0;

EXPORT_API
void dump_logmanager(void)
{
    static char *str_direction[] = {"", "Console", "File", "Syslog", "System", "Viewer"};
    logmanager_obj_t *pdata = __get_data();

    fprintf(stdout, "======================================================================\n");
    fprintf(stdout, "                   Logmanager Setting Information                     \n");
    fprintf(stdout, "======================================================================\n");
    fprintf(stdout, " Use default                   : %-4s\n", pdata->use_default == 1 ? "YES":"NO");
    fprintf(stdout, " Log output                    : %-10s\n", str_direction[pdata->direction]);
    fprintf(stdout, " Owner Bits Mask               : 0x%08X\n", pdata->owner_mask);
    fprintf(stdout, " Class Bits Mask               : 0x%08X\n", pdata->class_mask);
    if (pdata->direction == LOG_DIRECTION_FILE)
    {
        fprintf(stdout, " File name                     : %s\n", pdata->filename);
        fprintf(stdout, " File internal                 : %d\n", pdata->fd_count);
    }
}

int init_logmanager(void)
{
    /* Create & Get data storage */
#ifndef USE_KERNEL_OBJECT
    int shmfd = 0;
#endif
    logmanager_obj_t *pdata = NULL;
    char *filename = NULL;

    /* Load Configure file */
    dictionary *inifile = NULL;

#ifndef USE_KERNEL_OBJECT
    shmfd = shm_open(LOG_SHARED_MMAP, O_CREAT|O_RDWR|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    if (shmfd < 0)
    {
        if (errno == EEXIST)
        {
            /* Normal operation condition. */
//            fprintf(stderr, "Shared memory already exist. But, this is normal operation.\n");
        }
        else
        {
            perror("Shared memory create fail\n");
        }
        return -1;
    }
    ftruncate(shmfd, sizeof(logmanager_obj_t));

    pdata = (logmanager_obj_t*) mmap(0, sizeof(logmanager_obj_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_LOCKED, shmfd, 0);
    if (pdata == MAP_FAILED)
    {
        perror("Get ptr fail\n");
        return -1;
    }
#else
    if (__create_shm(LOG_SHARED_LOGKEY, LOG_SHARED_PREFIX, sizeof(logmanager_obj_t)) < 0)
        perror("Shared memory create fail\n");
    pdata = __get_data();
    if (pdata == NULL)
    {
        perror("Get ptr fail\n");
        return -1;
    }
#endif
    memset(pdata->colors, 0, sizeof(pdata->colors));

    inifile = iniparser_load(LOG_INI_FILE);

    if (inifile)
    {
        pdata->use_default = 0;
        pdata->direction  = iniparser_getint(inifile, LOG_DIRECTION_KEY, LOG_DIRECTION_NONE);
        pdata->owner_mask = iniparser_getint(inifile, LOG_OWNER_FILTER_KEY, LOG_NONE);
        pdata->class_mask = iniparser_getint(inifile, LOG_CLASS_FILTER_KEY, LOG_CLASS_NONE);

        if (pdata->direction == LOG_DIRECTION_FILE)
        {
            filename = iniparser_getstring(inifile, LOG_LOGFILE_KEY, NULL);

            if (filename && (strlen (filename) < sizeof (pdata->filename))) // :+:by prevent
            {
                //strcpy(pdata->filename, filename); // :-:by prevent
				strncpy(pdata->filename, filename, strlen(filename)); // :+:by prevent

                //pdata->fd_count = ++pdata->fd_count;
                pdata->fd_count += 1;
            }
            else
            {
                pdata->use_default = 1;
            }
        }
        /* COLOR SETTINGS */
        __conv_color(LOG_CLASS_INFO,      pdata->colors, iniparser_getstring(inifile, LOG_COLOR_INFO_KEY, NULL));
        __conv_color(LOG_CLASS_WARNING,   pdata->colors, iniparser_getstring(inifile, LOG_COLOR_WARNING_KEY, NULL));
        __conv_color(LOG_CLASS_ERR,       pdata->colors, iniparser_getstring(inifile, LOG_COLOR_ERROR_KEY, NULL));
        __conv_color(LOG_CLASS_CRITICAL,  pdata->colors, iniparser_getstring(inifile, LOG_COLOR_CRITICAL_KEY, NULL));
        __conv_color(LOG_CLASS_NONE,      pdata->colors, iniparser_getstring(inifile, LOG_COLOR_ASSERT_KEY, NULL));

        iniparser_freedict(inifile);
    }

    if (inifile == NULL || pdata->use_default) /* Not exist setting file, use default */
    {
	   pdata->use_default = 1;
        pdata->owner_mask  = LOG_ALL;
        pdata->class_mask  = LOG_CLASS_ERR | LOG_CLASS_CRITICAL;
        pdata->direction   = LOG_DIRECTION_SYSLOG;
        __conv_color(LOG_CLASS_INFO,      pdata->colors, "DEFAULT");
        __conv_color(LOG_CLASS_WARNING,   pdata->colors, "BLUE");
        __conv_color(LOG_CLASS_ERR,       pdata->colors, "RED");
        __conv_color(LOG_CLASS_CRITICAL,  pdata->colors, "RED");
        __conv_color(LOG_CLASS_NONE,      pdata->colors, "RED");
        pdata->fd_count += 1;
	   strncpy(pdata->filename, LOG_DEFAULT_FILE_PATH, sizeof(pdata->filename)-1);
    }
    inifile = NULL;
#ifndef USE_KERNEL_OBJECT
    if(munmap(pdata, sizeof(logmanager_obj_t)) == -1)
    {
        perror("Unmap fail\n");
        return 1;
    }
#endif
    return 0;
}

int fini_logmanager(void)
{
#ifndef USE_KERNEL_OBJECT
    int shmfd = 0;
#endif
    logmanager_obj_t *pdata = NULL;

#ifndef USE_KERNEL_OBJECT
    shmfd = shm_open(LOG_SHARED_MMAP, O_RDWR, 0);
    if (shmfd < 0)
    {
        perror("Shared memory get fail\n");
        return -1;
    }

    pdata = (logmanager_obj_t*) mmap(0, sizeof(logmanager_obj_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_LOCKED, shmfd, 0);
    if (pdata == MAP_FAILED)
    {
        perror("Get ptr fail\n");
        return -1;
    }

    memset(pdata, 0, sizeof(logmanager_obj_t));

    if(munmap(pdata, sizeof(logmanager_obj_t)) == -1)
    {
        perror("Unmap fail\n");
        return 1;
    }

    shm_unlink(LOG_SHARED_MMAP);
#else
    if (__remove_shm(LOG_SHARED_LOGKEY, LOG_SHARED_PREFIX, sizeof(logmanager_obj_t)) < 0)
    {
        perror("Remove Shm fail\n");
    }
#endif
    return 0;
}

EXPORT_API
int reload_logmanager(void)
{
#ifndef USE_KERNEL_OBJECT
    int shmfd = 0;
#endif
    logmanager_obj_t *pdata = NULL;
    dictionary *inifile = NULL;
    char *filename = NULL;
    int fd_count;

#ifndef USE_KERNEL_OBJECT
    shmfd = shm_open(LOG_SHARED_MMAP, O_RDWR, 0);
    if (shmfd < 0)
    {
        perror("Shared memory get fail\n");
        return -1;
    }

    pdata = (logmanager_obj_t*) mmap(0, sizeof(logmanager_obj_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_LOCKED, shmfd, 0);
    if (pdata == MAP_FAILED)
    {
        perror("Get ptr fail\n");
        return -1;
    }
#else
    pdata = __get_data();
    if (pdata == NULL)
    {
        perror("Get ptr fail\n");
        return -1;
    }
#endif
    fd_count = pdata->fd_count;

    memset(pdata, 0, sizeof(logmanager_obj_t));

    inifile = iniparser_load(LOG_INI_FILE);

    if (inifile)
    {
        pdata->use_default = 0;
        pdata->direction  = iniparser_getint(inifile, LOG_DIRECTION_KEY, LOG_DIRECTION_NONE);
        pdata->owner_mask = iniparser_getint(inifile, LOG_OWNER_FILTER_KEY, LOG_NONE);
        pdata->class_mask = iniparser_getint(inifile, LOG_CLASS_FILTER_KEY, LOG_CLASS_NONE);

        if (pdata->direction == LOG_DIRECTION_FILE)
        {
            filename = iniparser_getstring(inifile, LOG_LOGFILE_KEY, NULL);

            if (filename)
            {
                strncpy(pdata->filename, filename, 1024);
                pdata->fd_count = ++fd_count < 0 ? 1: fd_count;
            }
            else
            {
                pdata->use_default = 0;
            }
        }
        /* COLOR SETTINGS */
        __conv_color(LOG_CLASS_INFO,      pdata->colors, iniparser_getstring(inifile, LOG_COLOR_INFO_KEY, NULL));
        __conv_color(LOG_CLASS_WARNING,   pdata->colors, iniparser_getstring(inifile, LOG_COLOR_WARNING_KEY, NULL));
        __conv_color(LOG_CLASS_ERR,       pdata->colors, iniparser_getstring(inifile, LOG_COLOR_ERROR_KEY, NULL));
        __conv_color(LOG_CLASS_CRITICAL,  pdata->colors, iniparser_getstring(inifile, LOG_COLOR_CRITICAL_KEY, NULL));
        __conv_color(LOG_CLASS_NONE,      pdata->colors, iniparser_getstring(inifile, LOG_COLOR_ASSERT_KEY, NULL));

        iniparser_freedict(inifile);
    }

    if (inifile == NULL || pdata->use_default) /* If an error occur, use default */
    {
	   pdata->use_default = 1;
        pdata->owner_mask  = LOG_ALL;
        pdata->class_mask  = LOG_CLASS_ERR | LOG_CLASS_CRITICAL;
        pdata->direction   = LOG_DIRECTION_SYSLOG;
        __conv_color(LOG_CLASS_INFO,      pdata->colors, "DEFAULT");
        __conv_color(LOG_CLASS_WARNING,   pdata->colors, "BLUE");
        __conv_color(LOG_CLASS_ERR,       pdata->colors, "RED");
        __conv_color(LOG_CLASS_CRITICAL,  pdata->colors, "RED");
        __conv_color(LOG_CLASS_NONE,      pdata->colors, "RED");
        pdata->fd_count += 1;
	   strncpy(pdata->filename, LOG_DEFAULT_FILE_PATH, sizeof(pdata->filename)-1);
    }

    inifile = NULL;
    if(munmap(pdata, sizeof(logmanager_obj_t)) == -1)
    {
        perror("Unmap fail\n");
        return 1;
    }

    return 0;
}

static void print_dummy(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata)
{
    /* Dummy. Null function. */
}

#ifdef ENABLE_DEBUG_MESSAGE
static void print_system(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata)
{
    char *owner_str = NULL;
    char *class_str = NULL;
    int owner_res_index = -1;
    int debug_flag = DEBUG_VERBOSE;

    __owner_res_index(mowner, owner_res_index);

    owner_str = owner_res[owner_res_index];
    class_str = class_res[mclass];

    if (mclass > 3)
        debug_flag = DEBUG_EXCEPTION;
    if (!owner_str || !class_str)
    {
        fprintf(stderr, "!!!LOGMANAGER ERROR!!! - Unknown owner or class %x:%x- !!!LOGMANAGER ERROR!!!\n", mowner, mclass);
        return;
    }

    debug_message(MID_FMULTIMEDIA, debug_flag, "\033[%dm%s: [%d,%d]{%s} %s\033[0m", mcolor, owner_str, (int)getpid(), (int)log_gettid(), class_str, work_buf1);
}
#endif

static void print_console(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata)
{
    char *owner_str = NULL;
    char *class_str = NULL;
    int owner_res_index = -1;

    __owner_res_index(mowner, owner_res_index);

    owner_str = owner_res[owner_res_index];
    class_str = class_res[mclass];

    if (!owner_str || !class_str)
    {
        fprintf(stderr, "!!!LOGMANAGER ERROR!!! - Unknown owner or class %x:%x- !!!LOGMANAGER ERROR!!!\n", mowner, mclass);
        return;
    }

    fprintf(stderr, "\033[%dm%s: [%d,%d]{%s} %s\033[0m", mcolor, owner_str, (int)getpid(), (int)log_gettid(), class_str, work_buf1);
}

static void print_file(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata)
{
    char *owner_str = NULL;
    char *class_str = NULL;
    int owner_res_index = -1;

    if (pdata->fd_count == 0)
        return;

    if (logfd_count != pdata->fd_count)
    {
        if (logfd != -1)
        {
            logfd = close(logfd);
            if (logfd ==  -1) /* Long file open fail! Use default */
            {
                perror("Previous log file close fail.");
                return;
            }
        }
        logfd = open(pdata->filename, O_CREAT|O_APPEND|O_WRONLY|O_SYNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (logfd ==  -1) /* Long file open fail! Use default */
        {
            perror("Log file open fail.");
            return;
        }
        logfd_count = pdata->fd_count;
    }

    __owner_res_index(mowner, owner_res_index);

    owner_str = owner_res[owner_res_index];
    class_str = class_res[mclass];

    if (!owner_str || !class_str)
    {
        fprintf(stderr, "!!!LOGMANAGER ERROR!!! - Unknown owner or class %x:%x- !!!LOGMANAGER ERROR!!!\n", mowner, mclass);
        return;
    }

    snprintf(work_buf2, BUF_LEN, "%s: [%d,%d]{%s} %s", owner_str, (int)getpid(), (int)log_gettid(), class_str, work_buf1);
    write(logfd, work_buf2, strlen(work_buf2));
}

static void print_syslog(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata)
{
    char *owner_str = NULL;
    char *class_str = NULL;
    int owner_res_index = -1;
    int sysowner;

    __owner_res_index(mowner, owner_res_index);

    owner_str = owner_res[owner_res_index];
    class_str = class_res[mclass];

    if (!owner_str || !class_str)
    {
        fprintf(stderr, "!!!LOGMANAGER ERROR!!! - Unknown owner or class %x:%x- !!!LOGMANAGER ERROR!!!\n", mowner, mclass);
        return;
    }

    switch(mclass)
    {
    case LOG_CLASS_INFO:
        sysowner = LOG_INFO;
        break;
    case LOG_CLASS_WARNING:
        sysowner = LOG_WARNING;
        break;
    case LOG_CLASS_ERR:
        sysowner = LOG_ERR;
        break;
    case LOG_CLASS_CRITICAL:
        sysowner = LOG_CRIT;
        break;
    default:
        sysowner = LOG_DEBUG;
        return;
    }

    openlog("MMFW", LOG_NDELAY, LOG_USER);
    syslog(sysowner, "%s:[%d,%d]{%s} %s", owner_str, (int)getpid(), (int)log_gettid(),  class_str, work_buf1);
    closelog();
}

static void print_viewer(const int mowner, const int mclass, const int mcolor, const logmanager_obj_t *pdata)
{
    char *owner_str = NULL;
    char *class_str = NULL;
    int owner_res_index = -1;
    int fd = -1;

    __owner_res_index(mowner, owner_res_index);

    owner_str = owner_res[owner_res_index];
    class_str = class_res[mclass];

    if (!owner_str || !class_str)
    {
        fprintf(stderr, "!!!LOGMANAGER ERROR!!! - Unknown owner or class %x:%x- !!!LOGMANAGER ERROR!!!\n", mowner, mclass);
        return;
    }  fd = open(FIFO_PATH, O_WRONLY, O_NONBLOCK);
    if (fd == -1)
    {
        /* If pipe is not exist, open pipe error. This mean not active viewer. */
//        perror("PIPE OPEN");
        return;
    }

    snprintf(work_buf2, BUF_LEN, "\033[%dm%s: [%d,%d]{%s} %s\033[0m", mcolor, owner_str, (int)getpid(), (int)log_gettid(), class_str, work_buf1);
    write(fd, work_buf2, strlen(work_buf2));
    fd = close(fd);
    if (fd == -1)
    {
        perror("PIPE CLOSE");
    }
}

static void __conv_color(int index, char *colors, const char* str)
{
    switch(strlen(str))
    {
    case 3: /* RED */
        if(!strcmp(str, "RED"))
            colors[index] = 31;
        break;
    case 4: /* BLUE CYAN */
        if(!strcmp(str, "BLUE"))
            colors[index] = 34;
        else if(!strcmp(str, "CYAN"))
            colors[index] = 36;
        break;
    case 5: /* BLACK GREEN WHITE B_RED */
        if(!strcmp(str, "BLACK"))
            colors[index] = 30;
        else if(!strcmp(str, "GREEN"))
            colors[index] = 32;
        else if(!strcmp(str, "WHITE"))
            colors[index] = 97;
        else if(!strcmp(str, "B_RED"))
            colors[index] = 101;
        break;
    case 6: /* B_GRAY B_CYAN B_BLUE */
        if(!strcmp(str, "B_GRAY"))
            colors[index] = 100;
        else if(!strcmp(str, "B_CYAN"))
            colors[index] = 106;
        else if(!strcmp(str, "B_BLUE"))
            colors[index] = 104;
       break;
    case 7: /* MAGENTA REVERSE B_GREEN default */
        if(!strcmp(str, "MAGENTA"))
            colors[index] = 35;
        else if(!strcmp(str, "REVERSE"))
            colors[index] = 7;
        else if(!strcmp(str, "B_GREEN"))
            colors[index] = 102;
        else if(!strcmp(str, "default"))
            colors[index] = 0;
        break;
    case 8: /* B_YELLOW */
        if(!strcmp(str, "B_YELLOW"))
            colors[index] = 103;
        break;
    case 9: /* B_MAGENTA */
        if(!strcmp(str, "B_MAGENTA"))
            colors[index] = 105;
        break;
    default:
        colors[index] = 0;
        break;
    }
}

static logmanager_obj_t* __get_data(void)
{
#ifndef USE_KERNEL_OBJECT
    int shmfd = 0;
#endif
    static logmanager_obj_t *pdata = NULL;

    if(!pdata)
    {
#ifndef USE_KERNEL_OBJECT
        shmfd = shm_open(LOG_SHARED_MMAP, O_RDWR, 0);
        if (shmfd < 0)
        {
            perror("Shared memory get fail\n");
            return NULL;
        }

        pdata = (logmanager_obj_t*) mmap(0, sizeof(logmanager_obj_t), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_LOCKED, shmfd, 0);
        if (pdata == MAP_FAILED)
        {
            perror("Get ptr fail\n");
            return NULL;
        }
#else
        pdata = (logmanager_obj_t*)__get_shm(LOG_SHARED_LOGKEY, LOG_SHARED_PREFIX, sizeof(logmanager_obj_t));
#endif
    }
    return pdata;
}

static struct sigaction sigint_action;  /* Backup pointer of SIGINT handler */
static struct sigaction sigabrt_action; /* Backup pointer of SIGABRT signal handler */
static struct sigaction sigsegv_action; /* Backup pointer of SIGSEGV fault signal handler */
static struct sigaction sigterm_action; /* Backup pointer of SIGTERM signal handler */
static struct sigaction sigsys_action;  /* Backup pointer of SIGSYS signal handler */
void __init_logmanager(void);
void __fini_logmanager(void);

void _signal_logmanager(int sig)
{
    __fini_logmanager();

    switch(sig)
    {
        case SIGINT:
                sigaction(SIGINT, &sigint_action, NULL);
            break;
        case SIGABRT:
                sigaction(SIGABRT, &sigabrt_action, NULL);
            break;
        case SIGSEGV:
                sigaction(SIGSEGV, &sigsegv_action, NULL);
            break;
        case SIGTERM:
                sigaction(SIGTERM, &sigterm_action, NULL);
            break;
        case SIGSYS:
                sigaction(SIGSYS, &sigsys_action, NULL);
            break;
        default:
            fprintf(stderr, "Signal is not supported\n");
    }
    raise(sig);
}

void __init_logmanager(void)
{
    struct sigaction action;
    action.sa_handler = _signal_logmanager;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    sigaction(SIGINT, &action, &sigint_action);
    sigaction(SIGABRT, &action, &sigabrt_action);
    sigaction(SIGSEGV, &action, &sigsegv_action);
    sigaction(SIGTERM, &action, &sigterm_action);
    sigaction(SIGSYS, &action, &sigsys_action);
    init_logmanager();
}

void __fini_logmanager(void)
{
    logmanager_obj_t *pdata;
#ifndef USE_KERNEL_OBJECT
    pdata = __get_data();
    if (pdata)
    {
        if(munmap(pdata, sizeof(logmanager_obj_t)) == -1)
        {
            perror("Unmap fail\n");
        }
    }
#endif
    if  (logfd != -1)
    {
        logfd = close(logfd);
        if (logfd ==  -1)
        {
            perror("Previous log file close fail.");
        }
    }
}

static int __create_shm(const char *path, const int prefix, const int size)
{
    int shmid = -1;
    int *segptr = NULL;
    key_t key;

    if (size < 1 || path == NULL || access(path, R_OK) != 0)
    {
        return -1;
    }

    key = ftok(path, prefix);

    if ((shmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0600)) == -1)
    {
        if (errno == EEXIST)
        {
            fprintf(stderr,"Already initialized.\n");
            if ((shmid = shmget(key, size, 0)) == -1)
            {
                fprintf(stderr, "Initialization fail.\n");
            }
            else
            {
                segptr = shmat(shmid, 0, 0);
                assert(segptr != NULL);
            }
        }
        else
        {
            if(errno == EACCES)
                fprintf(stderr, "Require ROOT permission.\n");
            else if(errno == ENOMEM)
                fprintf(stderr, "System memory is empty.\n");
            else if(errno == ENOSPC)
                fprintf(stderr, "Resource is empty.\n");
        }
    }
    else
    {
        shmctl(shmid, SHM_LOCK, 0);
        segptr = shmat(shmid, 0, 0);
        assert(segptr != NULL);
        memset((void*)segptr, 0, size);
    }

    if (shmid != -1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static int __remove_shm(const char *path, const int prefix, const int size)
{
    int shmid = -1;
    key_t key;

    if (size < 1 || path == NULL || access(path, R_OK) != 0)
    {
        return -1;
    }
    key = ftok(path, prefix);

    if ((shmid = shmget(key, size, 0)) == -1)
    {
        if(errno == ENOENT)
            fprintf(stderr, "Not initialized.\n");
        else if(errno == EACCES)
            fprintf(stderr, "Require ROOT permission.\n");
        else if(errno == ENOSPC)
            fprintf(stderr, "Resource is empty.\n");
    }
    else
    {
        assert(shmctl(shmid, IPC_RMID, 0) == 0);
    }

    if (shmid != -1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static void* __get_shm(const char *path, const int prefix, const int size)
{
    int shmid = -1;
    key_t key;

    if (size < 1 || path == NULL || access(path, R_OK) != 0)
    {
        return NULL;
    }
    key = ftok(path, prefix);

    if ((shmid = shmget(key, size, 0)) == -1)
    {
        if(errno == ENOENT)
            fprintf(stderr, "Not initialized.\n");
        else if(errno == EACCES)
            fprintf(stderr, "Require ROOT permission.\n");
        else if(errno == ENOSPC)
            fprintf(stderr, "Resource is empty.\n");
        return NULL;
    }
    return (void*)shmat(shmid, 0, 0);
}

EXPORT_API
void _log_print_rel(log_owner_t vowner, log_class_t vclass, char *msg, ...)
{
    va_list arg;
    logmanager_obj_t *pdata =  __get_data();

    if (!pdata)
    {
        fprintf(stderr, "!!!LOGMANAGER ERROR!!! - Cannot read settings - !!!LOGMANAGER ERROR!!!\n");
        return;
    }

    if (vowner == -1 && vclass == -1)
    {
        vowner = vclass = 0;
    }
    else
    {
        vowner &= pdata->owner_mask;
        vclass &= pdata->class_mask;

        if (vowner == 0 || vclass == 0)
            return;
    }

    va_start(arg, msg);
    pthread_mutex_lock(&g_mutex);
    vsnprintf(work_buf1, BUF_LEN-100, msg, arg);
    print_list[pdata->direction](vowner, vclass, pdata->colors[vclass], pdata);
    pthread_mutex_unlock(&g_mutex);
    va_end(arg);
}

EXPORT_API
void _log_assert_rel(int condition, char *str, int line)
{
    logmanager_obj_t *pdata =  __get_data();
    static char msg[256];

    if (!pdata)
    {
        fprintf(stderr, "!!!LOGMANAGER ERROR ASSERT!!! - Cannot read settings - !!!LOGMANAGER ERROR ASSERT!!!\n");
        return;
    }

    if (!condition)
    {
	#ifdef ENABLE_DEBUG_MESSAGE
        if (pdata->direction == LOG_DIRECTION_SYSTEM)
        {
            sys_assert("Assertion Fail at %s (%d)\n", str, line);
        }
        else
	#endif
        {
            snprintf(msg, 255, "Assertion Fail at %s (%d)\n", str, line);
            _log_print_rel(-1, -1, msg, NULL);
            abort();
        }
    }
}


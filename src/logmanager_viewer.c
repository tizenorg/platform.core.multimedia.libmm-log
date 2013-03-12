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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define FIFO_FILE "/tmp/logman.fifo"

#define OP_USAGE -1
#define OP_VIEWER 1
#define OP_RELOAD 2
#define OP_DUMP 3

void dump_logmanager(void);
int reload_logmanager(void);
static int get_options(int argc, char* argv[]);
static int usage(int argc, char* argv[]);
static void run_viewer(void);

static struct sigaction sigint_action;  /* Backup pointer of SIGINT handler */
static int fd;

void interrupt_signal(int sig)
{
    close(fd);
    remove(FIFO_FILE);

    sigaction(SIGINT, &sigint_action, NULL);
    raise(sig);
    exit(0);
}

int main(int argc, char **argv)
{
    switch(get_options(argc,argv))
    {
    case OP_VIEWER:
        run_viewer();
        break;
    case OP_RELOAD:
        reload_logmanager();
        break;
    case OP_DUMP:
        dump_logmanager();
        break;
    default:
        break;
    }
    return 0;
}

static int get_options(int argc, char* argv[])
{
    int ch = 0;
    int operation = OP_USAGE;

    if(argc != 2)
        return usage(argc, argv);

    while((ch = getopt(argc, argv, "vrd")) != EOF)
    {
        switch(ch)
        {
        case 'v':
            operation = OP_VIEWER;
            break;
        case 'r':
            operation = OP_RELOAD;
            break;
        case 'd':
            operation = OP_DUMP;
            break;
        default:
            return usage(argc, argv);
        }
    }
    if(optind != argc)
        return usage(argc, argv);

    return operation;
}

static int usage(int argc, char* argv[])
{
    fprintf(stderr, "Usage : %s option\n", argv[0]);
    fprintf(stderr, "[OPTIONS]\n");
    fprintf(stderr, "  -v : Run logviewer\n");
    fprintf(stderr, "  -r : Reload configuration\n");
    fprintf(stderr, "  -d : Dump current log settings\n");
    return OP_USAGE;
}

static void run_viewer(void)
{
    char readbuf[4097];
    int byte_read = 0;
    mode_t pre_mask;
    struct sigaction action;

    action.sa_handler = interrupt_signal;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);

    sigaction(SIGINT, &action, &sigint_action);

    pre_mask = umask(0);
    if (mknod(FIFO_FILE, S_IFIFO|0666, 0) == -1)
    {
        perror ("mknod failed\n");
    }
    umask(pre_mask);

    fd = open(FIFO_FILE, O_RDONLY|O_NONBLOCK);

    if (fd == -1)
    {
        perror("PIPE CREATE\n");
    }
    else
    {
        memset(readbuf, 0, 4097);
        while(1)
        {
            byte_read = read(fd,readbuf, 4096);
            if(byte_read > 0 && byte_read <= 4096 && readbuf[0] != '\0')
            {
                readbuf[byte_read] = '\0';
                printf("%s", readbuf);
                memset(readbuf, 0, 4097);
            }
            else
            {
                usleep(1);
            }
        }
    }
}


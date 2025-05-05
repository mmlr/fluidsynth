/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "fluid_sys.h"
#include "fluid_sys_generic.h"

#include <sys/stat.h>

pthread_mutex_t _posix_atomic_lock = PTHREAD_MUTEX_INITIALIZER;

bool fluid_file_test(const char *path, int flags)
{
    fluid_stat_buf_t buffer;
    int result = fluid_stat(path, &buffer);
    if (result != 0)
        return false;

    if ((flags & FLUID_FILE_TEST_EXISTS) != 0)
        return true;

    return S_ISREG(buffer.st_mode) != 0;
}


bool fluid_shell_parse_argv(const char *command_line, int *argcp, char ***argvp)
{
    return false;
}

void fluid_strfreev(char **argvp)
{
    free(argvp);
}

void fluid_msleep(unsigned int msecs)
{
    struct timespec ts;
    ts.tv_sec = msecs / 1000;
    ts.tv_nsec = msecs % 1000 * 1000;
    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &ts) != 0 && errno == EINTR)
        ;
}

double fluid_utime()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000 * 1000 + ts.tv_nsec;
}

fluid_thread_t *
new_fluid_thread(const char *name, fluid_thread_func_t func, void *data, int prio_level, int detach)
{
    fluid_thread_info_t *info = NULL;
    pthread_t *thread = NULL;

    if (func == NULL)
        return NULL;

    thread = FLUID_NEW(pthread_t);
    if (thread == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    if (prio_level > 0)
    {
        info = FLUID_NEW(fluid_thread_info_t);
        if (info == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            FLUID_FREE(thread);
            return NULL;
        }

        info->func = func;
        info->data = data;
        info->prio_level = prio_level;

        func = fluid_thread_high_prio;
        data = info;
    }

    if (pthread_create(thread, NULL, func, data) != 0)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the thread: %s",
                  fluid_strerror(errno));
        FLUID_FREE(info);
        FLUID_FREE(thread);
        return NULL;
    }

    if (detach)
        pthread_detach(*thread);

    return thread;
}

void delete_fluid_thread(fluid_thread_t *thread)
{
    FLUID_FREE(thread);
}

int fluid_thread_join(fluid_thread_t *thread)
{
    void *result;
    return pthread_join(*thread, &result) == 0 ? FLUID_OK : FLUID_ERR;
}

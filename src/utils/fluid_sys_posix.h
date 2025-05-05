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


/*
 * @file fluid_sys_posix.h
 *
 * This header contains the POSIX/pthread OS abstraction.
 */

#ifndef _FLUID_SYS_POSIX_H
#define _FLUID_SYS_POSIX_H

#include "fluidsynth_priv.h"
#include "fluid_stub_functions.h"

#include <endian.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>

#define FALSE (0)
#define TRUE (!FALSE)

#ifdef LADSPA
#error "LADSPA is not yet supported with the posix OS abstraction"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void *fluid_pointer_t;

/* Endian detection */
#define FLUID_IS_BIG_ENDIAN       (BYTE_ORDER == BIG_ENDIAN)

#define FLUID_LE32TOH(x)          le32toh(x)
#define FLUID_LE16TOH(x)          le16toh(x)

/*
 * Utility functions
 */

#define FLUID_FILE_TEST_EXISTS      1
#define FLUID_FILE_TEST_IS_REGULAR  2

bool fluid_file_test(const char *path, int flags);
bool fluid_shell_parse_argv(const char *command_line, int *argcp, char ***argvp);
void fluid_strfreev(char **argvp);

#define fluid_strerror strerror
#define fluid_setenv setenv


/* Time functions */

void fluid_msleep(unsigned int msecs);
double fluid_utime(void);


/* Muteces */

typedef pthread_mutex_t fluid_mutex_t;

#define FLUID_MUTEX_INIT            PTHREAD_MUTEX_INITIALIZER
#define fluid_mutex_init(_m)        pthread_mutex_init(&(_m), NULL)
#define fluid_mutex_destroy(_m)     pthread_mutex_destroy(&(_m))
#define fluid_mutex_lock(_m)        pthread_mutex_lock(&(_m))
#define fluid_mutex_unlock(_m)      pthread_mutex_unlock(&(_m))

/* Recursive lock capable mutex */
typedef pthread_mutex_t fluid_rec_mutex_t;

#define fluid_rec_mutex_init(_m)    _fluid_rec_mutex_init(&(_m))
#define fluid_rec_mutex_destroy(_m) pthread_mutex_destroy(&(_m))
#define fluid_rec_mutex_lock(_m)    pthread_mutex_lock(&(_m))
#define fluid_rec_mutex_unlock(_m)  pthread_mutex_unlock(&(_m))

static FLUID_INLINE void
_fluid_rec_mutex_init(fluid_mutex_t *mutex)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &attr);
}

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef pthread_mutex_t fluid_cond_mutex_t;

#define fluid_cond_mutex_lock       pthread_mutex_lock
#define fluid_cond_mutex_unlock     pthread_mutex_unlock
#define new_fluid_cond_mutex        new_fluid_cond_mutex_generic
#define delete_fluid_cond_mutex     delete_fluid_cond_mutex_generic

/* Thread condition signaling */
typedef pthread_cond_t fluid_cond_t;

#define fluid_cond_init(cond)       pthread_cond_init(&cond, NULL)
#define fluid_cond_destroy(cond)    pthread_cond_destroy(&cond)
#define fluid_cond_signal           pthread_cond_signal
#define fluid_cond_broadcast        pthread_cond_broadcast
#define fluid_cond_wait             pthread_cond_wait
#define new_fluid_cond              new_fluid_cond_generic
#define delete_fluid_cond           delete_fluid_cond_generic

/* Thread private data */

typedef pthread_key_t fluid_private_t;
#define fluid_private_init(_priv)       pthread_key_create(&_priv, NULL)
#define fluid_private_free(_priv)       pthread_key_delete(_priv)
#define fluid_private_get               pthread_getspecific
#define fluid_private_set               pthread_setspecific


/* Atomic operations */

#define fluid_atomic_int_inc(_pi) fluid_atomic_int_add(_pi, 1)
#define fluid_atomic_int_get(_pi) fluid_atomic_int_add(_pi, 0)

#define fluid_atomic_int_set(_pi, _val) \
    _fluid_atomic_int_set((fluid_atomic_int_t *)_pi, _val)
#define fluid_atomic_int_dec_and_test(_pi) \
    _fluid_atomic_int_dec_and_test((fluid_atomic_int_t *)_pi)
#define fluid_atomic_int_compare_and_exchange(_pi, _old, _new) \
    _fluid_atomic_int_compare_and_exchange((fluid_atomic_int_t *)_pi, _old, _new)
#define fluid_atomic_int_add(_pi, _add) \
    _fluid_atomic_int_add((fluid_atomic_int_t *)_pi, _add)
#define fluid_atomic_int_exchange_and_add fluid_atomic_int_add

/* This is clearly terrible, but there are no atomic operations provided in the
 * earlier POSIX specs. Also, this should use a spinlock instead of a mutex, but
 * there also is no PTHREAD_SPINLOCK_INITIALIZER...
 */
#define POSIX_ATOMIC_WRAPPER(op) \
    extern pthread_mutex_t _posix_atomic_lock; \
    pthread_mutex_lock(&_posix_atomic_lock); \
    op; \
    pthread_mutex_unlock(&_posix_atomic_lock);

static FLUID_INLINE void
_fluid_atomic_int_set(fluid_atomic_int_t *pi, int val)
{
    POSIX_ATOMIC_WRAPPER( *pi = val)
}

static FLUID_INLINE bool
_fluid_atomic_int_dec_and_test(fluid_atomic_int_t *pi)
{
    bool result;
    POSIX_ATOMIC_WRAPPER(result = --(*pi) == 0)
    return result;
}

static FLUID_INLINE bool
_fluid_atomic_int_compare_and_exchange(fluid_atomic_int_t *pi, int old, int _new)
{
    bool result;
    POSIX_ATOMIC_WRAPPER(
        if (*pi != old)
        {
            result = false;
        }
        else
        {
            *pi = _new;
            result = true;
        }
    )

    return result;
}

static FLUID_INLINE int
_fluid_atomic_int_add(fluid_atomic_int_t *pi, int add)
{
    int previous;
    POSIX_ATOMIC_WRAPPER(
        previous = *pi;
        *pi += add;
    )

    return previous;
}

static FLUID_INLINE bool
fluid_atomic_pointer_compare_and_exchange(void **pp, void *old, void *_new)
{
    bool result;
    POSIX_ATOMIC_WRAPPER(
        if (*pp != old)
        {
            result = false;
        }
        else
        {
            *pp = _new;
            result = true;
        }
    )

    return result;
}

static FLUID_INLINE void *
fluid_atomic_pointer_get(void **pp)
{
    void *result;
    POSIX_ATOMIC_WRAPPER(result = *pp)
    return result;
}

static FLUID_INLINE void
fluid_atomic_pointer_set(void **pp, void *val)
{
    POSIX_ATOMIC_WRAPPER(*pp = val);
}


/* Threads */

typedef void *fluid_pointer_t;

fluid_pointer_t fluid_thread_high_prio(fluid_pointer_t data);

/* other thread implementations might change this for their needs */
typedef void *fluid_thread_return_t;
typedef fluid_thread_return_t (*fluid_thread_func_t)(void *data);

/* static return value for thread functions which requires a return value */
#define FLUID_THREAD_RETURN_VALUE (NULL)

typedef pthread_t fluid_thread_t;

#define FLUID_THREAD_ID_NULL            NULL                    /* A NULL "ID" value */
#define fluid_thread_id_t               fluid_thread_t          /* Data type for a thread ID */
#define fluid_thread_get_id             pthread_self            /* Get unique "ID" for current thread */

/* whether or not the implementation can be thread safe at all */
#define FLUID_THREAD_SAFE_CAPABLE 1

/* File access */
typedef struct stat fluid_stat_buf_t;

#define fluid_stat stat


/* Debug functions */
#define fluid_assert assert

#ifdef __cplusplus
}
#endif
#endif /* _FLUID_SYS_POSIX_H */

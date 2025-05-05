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
 * @file fluid_sys_generic.h
 *
 * This header contains generic versions for functions in the OS abstraction.
 * They build purely on top of other OSAL provided functions and can be shared
 * across implementations as needed.
 */

#ifndef _FLUID_SYS_GENERIC_H
#define _FLUID_SYS_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex_generic(void)
{
    fluid_cond_mutex_t *mutex;
    mutex = FLUID_NEW(fluid_cond_mutex_t);
    fluid_mutex_init(*mutex);
    return mutex;
}

static FLUID_INLINE void
delete_fluid_cond_mutex_generic(fluid_cond_mutex_t *m)
{
    fluid_return_if_fail(m != NULL);
    fluid_mutex_destroy(*m);
    fluid_free(m);
}

static FLUID_INLINE fluid_cond_t *
new_fluid_cond_generic(void)
{
    fluid_cond_t *cond;
    cond = FLUID_NEW(fluid_cond_t);
    fluid_cond_init(*cond);
    return cond;
}

static FLUID_INLINE void
delete_fluid_cond_generic(fluid_cond_t *cond)
{
    fluid_return_if_fail(cond != NULL);
    fluid_cond_destroy(*cond);
    fluid_free(cond);
}

#ifdef __cplusplus
}
#endif
#endif /* _FLUID_SYS_GENERIC_H */

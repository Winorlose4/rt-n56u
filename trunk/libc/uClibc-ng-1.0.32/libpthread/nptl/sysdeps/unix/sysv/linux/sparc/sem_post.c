/* sem_post -- post to a POSIX semaphore.  SPARC version.
   Copyright (C) 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>

int
sem_post (sem_t *sem)
{
  struct sparc_new_sem *isem = (struct sparc_new_sem *) sem;
  int nr;

  if (__atomic_is_v9)
    nr = atomic_increment_val (&isem->value);
  else
    {
      __sparc32_atomic_do_lock24 (&isem->lock);
      nr = ++(isem->value);
      __sparc32_atomic_do_unlock24 (&isem->lock);
    }
  atomic_full_barrier ();
  if (isem->nwaiters > 0)
    {
      int err = lll_futex_wake (&isem->value, 1,
				isem->private ^ FUTEX_PRIVATE_FLAG);
      if (__builtin_expect (err, 0) < 0)
	{
	  __set_errno (-err);
	  return -1;
	}
    }
  return 0;
}

﻿/* hash-common.c - Common code for hash algorithms
 * Copyright (C) 2008 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#include "g10lib.h"
#include "hash-common.h"


/* Run a selftest for hash algorithm ALGO.  If the resulting digest
   matches EXPECT/EXPECTLEN and everything else is fine as well,
   return NULL.  If an error occurs, return a static text string
   describing the error.

   DATAMODE controls what will be hashed according to this table:

     0 - Hash the supplied DATA of DATALEN.
     1 - Hash one million times a 'a'.  DATA and DATALEN are ignored.

*/
const char *
_gcry_hash_selftest_check_one (int algo,
                               int datamode, const void *data, size_t datalen,
                               const void *expect, size_t expectlen)
{
  const char *result = NULL;
  gcry_error_t err = 0;
  gcry_md_hd_t hd;
  unsigned char *digest;

  if (_gcry_md_get_algo_dlen (algo) != expectlen)
    return "digest size does not match expected size";

  err = _gcry_md_open (&hd, algo, 0);
  if (err)
    return "gcry_md_open failed";

  switch (datamode)
    {
    case 0:
      _gcry_md_write (hd, data, datalen);
      break;

    case 1: /* Hash one million times an "a". */
      {
        char aaa[1000];
        int i;

        /* Write in odd size chunks so that we test the buffering.  */
        memset (aaa, 'a', 1000);
        for (i = 0; i < 1000; i++)
          _gcry_md_write (hd, aaa, 1000);
      }
      break;

    default:
      result = "invalid DATAMODE";
    }

  if (!result)
    {
      digest = _gcry_md_read (hd, algo);

      if ( memcmp (digest, expect, expectlen) )
        result = "digest mismatch";
    }

  _gcry_md_close (hd);

  return result;
}


/* Common function to write a chunk of data to the transform function
   of a hash algorithm.  Note that the use of the term "block" does
   not imply a fixed size block.  */
void
_gcry_md_block_write (void *context, const void *inbuf_arg, size_t inlen)
{
  const unsigned char *inbuf = inbuf_arg;
  gcry_md_block_ctx_t *hd = context;
  unsigned int stack_burn = 0;

  if (sizeof(hd->buf) < hd->blocksize)
    BUG();

  if (hd->buf == NULL || hd->bwrite == NULL)
    return;

  if (hd->count == hd->blocksize)  /* Flush the buffer. */
    {
      stack_burn = hd->bwrite (hd, hd->buf);
      _gcry_burn_stack (stack_burn);
      stack_burn = 0;
      hd->count = 0;
      if (!++hd->nblocks)
        hd->nblocks_high++;
    }
  if (!inbuf)
    return;

  if (hd->count)
    {
      for (; inlen && hd->count < hd->blocksize; inlen--)
        hd->buf[hd->count++] = *inbuf++;
      _gcry_md_block_write (hd, NULL, 0);
      if (!inlen)
        return;
    }

  while (inlen >= hd->blocksize)
    {
      stack_burn = hd->bwrite (hd, inbuf);
      hd->count = 0;
      if (!++hd->nblocks)
        hd->nblocks_high++;
      inlen -= hd->blocksize;
      inbuf += hd->blocksize;
    }
  _gcry_burn_stack (stack_burn);
  for (; inlen && hd->count < hd->blocksize; inlen--)
    hd->buf[hd->count++] = *inbuf++;
}

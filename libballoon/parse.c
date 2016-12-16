/*
 * Copyright (c) 2016, Henry Corrigan-Gibbs
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "base64.h"
#include "errors.h"
#include "parse.h"

static int 
encode (uint8_t *dst, const uint8_t *src, size_t srclen)
{
  int retval = b64_ntop (dst, 3*srclen, src, srclen);
  if (retval == -1) {
    fprintf(stderr, "Base64 encoding failed.\n");
    return 1;
  }
  if (retval > (int)(3 * srclen)-1) {
    fprintf(stderr, "Buffer too small.\n");
    return 1;
  }
  
  return 0;
}

int
write_hash (uint8_t *blob, size_t bloblen,
      const uint8_t *out, size_t outlen,
      const uint8_t *salt, size_t saltlen, 
      uint32_t s_cost, uint32_t t_cost, uint32_t n_threads)
{
  uint8_t salt64[3*saltlen];
  uint8_t out64[3*outlen];
  int retval;
  if ((retval = encode (salt64, salt, saltlen)))
    return retval;
  if ((retval = encode (out64, out, outlen)))
    return retval;

  if (snprintf ((char *)blob, bloblen, "$balloon$v=1$s=%u,t=%u,p=%u$%s$%s", s_cost, t_cost, n_threads, salt64, out64) < 0)
    return ERROR_SNPRINTF;

  return 0;
}


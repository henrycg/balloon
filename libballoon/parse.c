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

/*-
 * Copyright (c) 1990, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* OPENBSD ORIGINAL: lib/libc/string/strsep.c */


#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include "constants.h"
#include "errors.h"
#include "parse.h"

/*
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim.  
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strsep returns NULL.
 */
char *
my_strsep(char **stringp, const char *delim)
{
  char *s;
  const char *spanp;
  int c, sc;
  char *tok;

  if ((s = *stringp) == NULL)
    return (NULL);
  for (tok = s;;) {
    c = *s++;
    spanp = delim;
    do {
      if ((sc = *spanp++) == c) {
        if (c == 0)
          s = NULL;
        else
          s[-1] = 0;
        *stringp = s;
        return (tok);
      }
    } while (sc != 0);
  }
  /* NOTREACHED */
}



static int 
encode (char *dst, const uint8_t *src, size_t srclen)
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
write_blob (char *blob, size_t bloblen,
      const uint8_t salt[SALT_LEN], 
      const uint8_t *out, size_t outlen,
      uint32_t s_cost, uint32_t t_cost, uint32_t n_threads)
{
  char salt64[3*SALT_LEN];
  char out64[3*outlen];
  int retval;
  if ((retval = encode (salt64, salt, SALT_LEN)))
    return retval;
  if ((retval = encode (out64, out, outlen)))
    return retval;

  retval = snprintf ((char *)blob, bloblen, "$balloon$v=1$s=%u,t=%u,p=%u$%s$%s", s_cost, t_cost, n_threads, salt64, out64);

  if (retval < 0 || retval == (int)bloblen)
    return ERROR_SNPRINTF;

  return 0;
}

size_t
n_tokens (const char *str, size_t strlen, uint8_t delim)
{
  int n_tokens = 1;
  for (size_t i = 0 ; i < strlen; i++) {
    if (str[i] == delim) n_tokens++;
  }

  return n_tokens;
}

void
tokenize (char **tokens, char *str, uint8_t delim)
{
  const char delimstr[] = { delim, '\0' };

  int i = 0;
  char *token;
  char *strp = &str[0];
  while ((token = my_strsep (&strp, delimstr)) != NULL) {
    tokens[i++] = token;
  }
}

int
int_parse(const char *intstr, uint32_t *intp)
{
  const size_t len = strlen (intstr);
  for (size_t i = 0; i < len; i++) {
    if (!isdigit(intstr[i])) return ERROR_PARSE;
  }

  char *end;
  uint64_t bigint = strtoul(intstr, &end, 10);

  // Make sure that we read the entire string.
  if (end[0] != '\0' || bigint == ULONG_MAX)
    return ERROR_PARSE;

  if (bigint > ((1ul<<32)-1ul))
    return ERROR_PARSE;

  *intp = (uint32_t)bigint;

  return ERROR_NONE;
}

int
parse_options (const char *optstr_in, size_t optlen,
  uint32_t *s_cost, uint32_t *t_cost, uint32_t *n_threads)
{
  char optstr[optlen];
  strncpy (optstr, optstr_in, optlen);

  // Count the number of ,-separated tokens 
  int n = n_tokens (optstr, optlen, ',');

  // Put the tokens into an array
  char *tokens[n];
  tokenize (tokens, optstr, ',');

  for (int i=0; i<n; i++) {
    char *token = tokens[i];
    if (strlen (token) < 3) 
      return ERROR_PARSE;

    char type = token[0];
    char eq = token[1];
    if (type != 'p' && type != 't' && type != 's') 
      return ERROR_PARSE;
    if (eq != '=') 
      return ERROR_PARSE;
  }

  int error;
  // All tokens have length at least 3
  for (int i=0; i<n; i++) {
    char *token = tokens[i];
    uint32_t *intp;
    switch (token[0]) {
      case 's':
        intp = s_cost;
        break;
      case 't':
        intp = t_cost;
        break;
      case 'p':
        intp = n_threads;
        break;
      default:
        return ERROR_PARSE;
    }
  
    if ((error = int_parse(&token[2], intp)) != ERROR_NONE)
      return ERROR_PARSE;
  }
  
  return ERROR_NONE;
} 

int
read_blob (const char *blob_in, size_t bloblen,
      uint8_t salt[SALT_LEN], uint8_t *out, size_t outlen,
      uint32_t *s_cost, uint32_t *t_cost, uint32_t *n_threads)
{
  if (blob_in[bloblen-1] != '\0')
    return ERROR_PARSE;

  char blob[bloblen];
  strncpy (blob, blob_in, bloblen);

  // Format is: 
  //    $balloon$v=1$t={time},s={space},p={parallelism}$salt$hash


  // Count the number of $-separated tokens 
  int n = n_tokens (blob_in, bloblen, '$');

  // Put the tokens into an array
  char *tokens[n];

  tokenize (tokens, blob, '$');

  printf("t0 %s .\n", tokens[0]);
  // Check the header
  if (strlen (tokens[0]) != 0)
    return ERROR_PARSE;
  printf("so far.\n");
  if (strlen (tokens[1]) != 7 || strncmp (tokens[1], "balloon", 7))
    return ERROR_PARSE;
  if (strlen (tokens[2]) != 3 || strncmp (tokens[2], "v=1", 3))
    return ERROR_PARSE;

  int optlen = strlen (tokens[3]); 
  char optstr[optlen];
  strncpy (optstr, tokens[3], optlen);

  int error;
  if ((error = parse_options (optstr, optlen+1, s_cost, t_cost, n_threads) != ERROR_NONE))
    return error;

  // Parse salt and password from Base64
  if (b64_pton (salt, SALT_LEN, tokens[4]) <= 0)
    return ERROR_PARSE;
  if (b64_pton (out, outlen, tokens[5]) <= 0)
    return ERROR_PARSE;

  return ERROR_NONE;
}

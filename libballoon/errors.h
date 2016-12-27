/*
 * Copyright (c) 2015, Henry Corrigan-Gibbs
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

#ifndef __ERRORS_H__
#define __ERRORS_H__

enum error_codes {
  ERROR_NONE,                       // 0

  ERROR_INCOMPATIBLE_OPTIONS,       // 1

  ERROR_NULL_POINTER,               // 2
  ERROR_MALLOC,                     // 3

  ERROR_MCOST_TOO_SMALL,            // 4
  ERROR_MCOST_TOO_BIG,              // 5

  ERROR_TCOST_TOO_SMALL,            // 6
  ERROR_TCOST_TOO_BIG,              // 7

  ERROR_INLEN_TOO_BIG,              // 8

  ERROR_NTHREADS_TOO_BIG,           // 9

  ERROR_CANNOT_EXTRACT_BEFORE_MIX,  // 10

  ERROR_OPENSSL_HASH,               // 11
  ERROR_OPENSSL_AES,                // 12

  ERROR_BITSTREAM_UNINITIALIZED,    // 13
  ERROR_BITSTREAM_FINALIZED,        // 14
  ERROR_BITSTREAM_MAX_TOO_SMALL,    // 15

  ERROR_PTHREAD,                    // 17
  ERROR_URANDOM,                    // 18
  ERROR_SNPRINTF,                   // 19

  ERROR_PARSE                       // 20
};

#endif

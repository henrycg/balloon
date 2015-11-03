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


#include <stdio.h>
#include <stdlib.h>

extern "C" int bitstream_rand_byte(struct bitstream *, unsigned char *);

class secure_random {
  public:
    // types
    typedef unsigned char result_type;

    // construct/copy/destruct
    secure_random (struct bitstream *bits) : _b(bits) {};

    // public member functions
    static constexpr result_type min (void) { return 0x00; };
    static constexpr result_type max (void) { return 0xff; };
    inline unsigned char operator()() {
      unsigned char out;
      if (bitstream_rand_byte (_b, &out)) {
        fprintf(stderr, "Unexpected error!\n");
        exit (-1);
      }
      return out;
    };

  private:
    struct bitstream *_b;
};


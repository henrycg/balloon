
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


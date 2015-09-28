/*
 * Argon2 source code package
 * 
 * This work is licensed under a Creative Commons CC0 1.0 License/Waiver.
 * 
 * You should have received a copy of the CC0 Public Domain Dedication along with
 * this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */


#include <stdint.h>
#include <string.h>

#include "argon2-core.h"

#include "blake-round-mka.h"
#include "blake2-impl.h"
#include "blake2.h"

static void 
XorBlock(unsigned char *out, const unsigned char* blockA, const unsigned char* blockB) {
  for (unsigned i = 0; i < ARGON2_BLOCK_SIZE; i++) {
    out[i] = blockA[i] ^ blockB[i];
  }
}

void 
Argon2FillBlock(unsigned char *out, const unsigned char* blockA, const unsigned char* blockB) {
    unsigned char tmp[ARGON2_BLOCK_SIZE];
    unsigned char blockR[ARGON2_BLOCK_SIZE];
    XorBlock (tmp, blockA, blockB);
    memcpy (blockR, tmp, ARGON2_BLOCK_SIZE);

    // Apply Blake2 on columns of 64-bit words: (0,1,...,15) , then (16,17,..31)... finally (112,113,...127)
    for (unsigned i = 0; i < 8; ++i) {
        BLAKE2_ROUND_NOMSG(blockR[16 * i], blockR[16 * i + 1], blockR[16 * i + 2], blockR[16 * i + 3],
                blockR[16 * i + 4], blockR[16 * i + 5], blockR[16 * i + 6], blockR[16 * i + 7],
                blockR[16 * i + 8], blockR[16 * i + 9], blockR[16 * i + 10], blockR[16 * i + 11],
                blockR[16 * i + 12], blockR[16 * i + 13], blockR[16 * i + 14], blockR[16 * i + 15]);
    }
    // Apply Blake2 on rows of 64-bit words: (0,1,16,17,...112,113), then (2,3,18,19,...,114,115).. finally (14,15,30,31,...,126,127)
    for (unsigned i = 0; i < 8; i++) {
        BLAKE2_ROUND_NOMSG(blockR[2 * i], blockR[2 * i + 1], blockR[2 * i + 16], blockR[2 * i + 17],
                blockR[2 * i + 32], blockR[2 * i + 33], blockR[2 * i + 48], blockR[2 * i + 49],
                blockR[2 * i + 64], blockR[2 * i + 65], blockR[2 * i + 80], blockR[2 * i + 81],
                blockR[2 * i + 96], blockR[2 * i + 97], blockR[2 * i + 112], blockR[2 * i + 113]);
    }

    // out = F(A^B) ^ A ^ B
    XorBlock (out, blockR, tmp);
}


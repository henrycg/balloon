/*
 * Argon2 source code package
 * 
 * This work is licensed under a Creative Commons CC0 1.0 License/Waiver.
 * 
 * You should have received a copy of the CC0 Public Domain Dedication along with
 * this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */


#ifndef __ARGON2_CORE_H__
#define __ARGON2_CORE_H__

#include <stdint.h>

/*************************Argon2 internal constants**************************************************/

/* Memory block size in bytes */
static const uint32_t ARGON2_BLOCK_SIZE = 1024;

/*
 * Function fills a new memory block
 * @param prev_block Pointer to the previous block
 * @param ref_block Pointer to the reference block
 * @param next_block Pointer to the block to be constructed
 * @pre all block pointers must be valid
 */
void Argon2FillBlock(unsigned char *blockR, const unsigned char* blockA, const unsigned char* blockB);

#endif


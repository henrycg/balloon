#ifndef __HASH_STATE_DOUBLE_PAR_H__
#define __HASH_STATE_DOUBLE_PAR_H__

#include <stddef.h>

#include "bitstream.h"
#include "options.h"

int hash_state_double_par_init (struct hash_state *s, struct baghash_options *opts);

int hash_state_double_par_free (struct hash_state *s);

int hash_state_double_par_fill (struct hash_state *s, 
    const void *in, size_t inlen,
    const void *salt, size_t saltlen);

int hash_state_double_par_mix (struct hash_state *s);

#endif


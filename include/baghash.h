#ifndef __BAGHASH_H__
#define __BAGHASH_H__

#include <stddef.h>

int BagHash (void *out, size_t outlen, const void *in, size_t inlen, 
    const void *salt, size_t saltlen, unsigned int t_cost, unsigned int m_cost);

#endif /* __BAGHASH_H__ */

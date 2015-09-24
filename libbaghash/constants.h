#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#define OUTLEN_MIN 16
#define OUTLEN_MAX 128

#define INLEN_MIN 4
#define INLEN_MAX (1<<20)

#define SALTLEN_MIN 4
#define SALTLEN_MAX (1<<20)

// Key length (in bits) for the AES-CTR cipher used
// to fill up the buffers initially.
#define AES_CTR_KEY_LEN 256

#endif


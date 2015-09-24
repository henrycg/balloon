
#include <assert.h>
#include <baghash.h>

#include "errors.h"
#include "hash_state.h"
#include "util.h"

#define TCOST_MIN 3
#define MCOST_MIN 64
#define BLOCK_SIZE 1024

struct {
  size_t buflen;
  unsigned char *buffer;
} hash_state_t;

int 
BagHash (void *out, size_t outlen, 
    const void *in, size_t inlen, 
    const void *salt, size_t saltlen, 
    unsigned int t_cost, unsigned int m_cost)
{
  int error;

  // Sanity check parameters
  if (!out || !in || !salt)
    return ERROR_NULL_POINTER;

  if ((error = validate_parameters (outlen, inlen, saltlen)))
    return error;

  if (t_cost < TCOST_MIN)
    t_cost = TCOST_MIN;
  if (m_cost < MCOST_MIN)
    m_cost = MCOST_MIN;

  struct hash_state state;
  if ((error = hash_state_init (&state, m_cost, BLOCK_SIZE)))
    return error;

  // Fill buffer of m_cost blocks with pseudo-random stuff derived
  // from the password.
  if ((error = hash_state_fill (&state, in, inlen, salt, saltlen)))
    return error;

  // Mix the buffer t_cost times
  for (unsigned int i = 0; i < t_cost; i++) {
    hash_state_mix (&state);
  }
 
  // Extract the output from the hash state
  hash_state_extract (&state, out, outlen);
 
  hash_state_free (&state);

  return ERROR_NONE;
}



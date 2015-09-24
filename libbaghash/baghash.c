
#include <assert.h>
#include <baghash.h>

#include "constants.h"
#include "errors.h"
#include "hash_state.h"
#include "util.h"
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
  if (m_cost >= MCOST_MAX)
    return ERROR_MCOST_TOO_BIG;

  struct hash_state state;
  if ((error = hash_state_init (&state, m_cost, BLOCK_SIZE, salt, saltlen)))
    return error;

  // Fill buffer of m_cost blocks with pseudo-random stuff derived
  // from the password and salt.
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



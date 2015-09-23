
#include <assert.h>
#include <baghash.h>

#include "errors.h"

#define TCOST_MIN 3
#define MCOST_MIN 64

int 
BagHash (void *out, size_t outlen, 
    const void *in, size_t inlen, 
    const void *salt, size_t saltlen, 
    unsigned int t_cost, unsigned int m_cost)
{
  // Sanity check parameters
  if (!out || !in || !salt)
    return ERROR_NULL_POINTER;

  int error = validate_parameters (outlen, inlen, saltlen);
  if (error)
    return error;

  if (t_cost < TCOST_MIN)
    tcost = TCOST_MIN;
  if (m_cost < MCOST_MIN)
    mcost = MCOST_MIN;

  // Fill buffer of m_cost blocks
  // with pseudo-random stuff derived
  // from the password
  
  // Mix the buffers t_cost times
 
  // Hash the resulting state of the buffer
  // together and return it
}



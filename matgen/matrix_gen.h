#ifndef __MATRIX_GEN_H__
#define __MATRIX_GEN_H__

#include <stdlib.h>

struct matrix_generator *
matrix_generator_init (struct bitstream *b, size_t n_rows, int c);

void
matrix_generator_free (struct matrix_generator *m);

int 
matrix_generator_row_weight (struct matrix_generator *m, size_t *out);

#endif

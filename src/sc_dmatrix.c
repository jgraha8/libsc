/*
  This file is part of the SC Library.
  The SC Library provides support for parallel scientific applications.

  Copyright (C) 2007,2008 Carsten Burstedde, Lucas Wilcox.

  The SC Library is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  The SC Library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the SC Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sc.h>
#include <sc_blas.h>
#include <sc_dmatrix.h>

static sc_dmatrix_t *
sc_dmatrix_new_internal (sc_bint_t m, sc_bint_t n, bool init_zero)
{
  sc_bint_t           i;
  sc_dmatrix_t       *rdm;
  size_t              size = (size_t) (m * n);

  SC_ASSERT (m >= 0 && n >= 0);

  rdm = SC_ALLOC (sc_dmatrix_t, 1);

  if (m > 0 && n > 0) {
    rdm->e = SC_ALLOC (double *, m + 1);

    if (init_zero) {
      rdm->e[0] = SC_ALLOC_ZERO (double, size);
    }
    else {
      rdm->e[0] = SC_ALLOC (double, size);
    }

    for (i = 1; i < m; ++i)
      rdm->e[i] = rdm->e[i - 1] + n;
    rdm->e[m] = NULL;           /* safeguard */
  }
  else {
    rdm->e = NULL;
  }

  rdm->m = m;
  rdm->n = n;

  return rdm;
}

sc_dmatrix_t       *
sc_dmatrix_new (sc_bint_t m, sc_bint_t n)
{
  return sc_dmatrix_new_internal (m, n, false);
}

sc_dmatrix_t       *
sc_dmatrix_new_zero (sc_bint_t m, sc_bint_t n)
{
  return sc_dmatrix_new_internal (m, n, true);
}

sc_dmatrix_t       *
sc_dmatrix_clone (sc_dmatrix_t * dmatrix)
{
  sc_bint_t           totalsize, inc;
  sc_dmatrix_t       *clone;

  totalsize = dmatrix->m * dmatrix->n;
  clone = sc_dmatrix_new (dmatrix->m, dmatrix->n);

  inc = 1;
  BLAS_DCOPY (&totalsize, dmatrix->e[0], &inc, clone->e[0], &inc);

  return clone;
}

void
sc_dmatrix_reshape (sc_dmatrix_t * dmatrix, sc_bint_t m, sc_bint_t n)
{
  SC_ASSERT (dmatrix->m * dmatrix->n == m * n);
  dmatrix->m = m;
  dmatrix->n = n;
}

void
sc_dmatrix_destroy (sc_dmatrix_t * dmatrix)
{
  if (dmatrix->e != NULL) {
    SC_FREE (dmatrix->e[0]);
    SC_FREE (dmatrix->e);
  }

  SC_FREE (dmatrix);
}

bool
sc_dmatrix_is_symmetric (sc_dmatrix_t * A, double tolerance)
{
  sc_bint_t           i, j;
  double              diff;

  SC_ASSERT (A->m == A->n);

  for (i = 0; i < A->n; ++i) {
    for (j = i + 1; j < A->n; ++j) {
      diff = fabs (A->e[i][j] - A->e[j][i]);
      if (diff > tolerance) {
#ifdef SC_DEBUG
        fprintf (stderr, "sc dmatrix not symmetric by %g\n", diff);
#endif
        return false;
      }
    }
  }

  return true;
}

void
sc_dmatrix_set_zero (sc_dmatrix_t * dmatrix)
{
  sc_dmatrix_set_value (dmatrix, 0.0);
}

void
sc_dmatrix_set_value (sc_dmatrix_t * dmatrix, double value)
{
  sc_bint_t           i, size;

  size = dmatrix->m * dmatrix->n;
  for (i = 0; i < size; ++i)
    dmatrix->e[0][i] = value;
}

void
sc_dmatrix_scale (double alpha, sc_dmatrix_t * X)
{
  sc_bint_t           totalsize, inc;

  totalsize = X->m * X->n;

  inc = 1;
  BLAS_DSCAL (&totalsize, &alpha, X->e[0], &inc);
}

void
sc_dmatrix_copy (sc_dmatrix_t * X, sc_dmatrix_t * Y)
{
  sc_bint_t           totalsize, inc;

  SC_ASSERT ((X)->m == (Y)->m && (X)->n == (Y)->n);

  totalsize = X->m * X->n;

  inc = 1;
  BLAS_DCOPY (&totalsize, X->e[0], &inc, Y->e[0], &inc);
}

void
sc_dmatrix_add (double alpha, sc_dmatrix_t * X, sc_dmatrix_t * Y)
{
  sc_bint_t           totalsize, inc;

  SC_ASSERT ((X)->m == (Y)->m && (X)->n == (Y)->n);

  totalsize = X->m * X->n;

  inc = 1;
  BLAS_DAXPY (&totalsize, &alpha, X->e[0], &inc, Y->e[0], &inc);
}

void
sc_dmatrix_vector (sc_trans_t transa, double alpha, sc_dmatrix_t * A,
                   sc_dmatrix_t * X, double beta, sc_dmatrix_t * Y)
{
  sc_bint_t           Arows, Acols, inc = 1;
#ifdef SC_DEBUG
  sc_bint_t           dimX = SC_MAX (X->m, X->n);
  sc_bint_t           dimY = SC_MAX (Y->m, Y->n);
#endif

  Arows = (transa == SC_NO_TRANS) ? A->m : A->n;
  Acols = (transa == SC_NO_TRANS) ? A->n : A->m;

  SC_ASSERT (Acols != 0 && Arows != 0);
  SC_ASSERT (Acols == dimX && Arows == dimY);

  BLAS_DGEMV (&sc_antitranschar[transa], &A->n, &A->m, &alpha,
              A->e[0], &A->n, X->e[0], &inc, &beta, Y->e[0], &inc);
}

void
sc_dmatrix_multiply (sc_trans_t transa, sc_trans_t transb, double alpha,
                     sc_dmatrix_t * A, sc_dmatrix_t * B, double beta,
                     sc_dmatrix_t * C)
{
  sc_bint_t           Acols, Crows, Ccols;
#ifdef SC_DEBUG
  sc_bint_t           Arows, Brows, Bcols;

  Arows = (transa == SC_NO_TRANS) ? A->m : A->n;
  Brows = (transb == SC_NO_TRANS) ? B->m : B->n;
  Bcols = (transb == SC_NO_TRANS) ? B->n : B->m;
#endif

  Acols = (transa == SC_NO_TRANS) ? A->n : A->m;
  Crows = C->m;
  Ccols = C->n;

  SC_ASSERT (Acols == Brows && Arows == Crows && Bcols == Ccols);
  SC_ASSERT (transa >= 0 && transa < SC_TRANS_ANCHOR);
  SC_ASSERT (transb >= 0 && transb < SC_TRANS_ANCHOR);

  SC_ASSERT (Acols != 0 && Crows != 0 && Ccols != 0);

  BLAS_DGEMM (&sc_transchar[transb], &sc_transchar[transa], &Ccols,
              &Crows, &Acols, &alpha, B->e[0], &B->n, A->e[0], &A->n, &beta,
              C->e[0], &C->n);
}

void
sc_dmatrix_print (sc_dmatrix_t * dmatrix, FILE * fp)
{
  sc_bint_t           i, j, m, n;

  m = dmatrix->m;
  n = dmatrix->n;

  for (i = 0; i < m; ++i) {
    for (j = 0; j < n; ++j) {
      fprintf (fp, " %16.8e", dmatrix->e[i][j]);
    }
    fprintf (fp, "\n");
  }
}

/* EOF sc_dmatrix.c */
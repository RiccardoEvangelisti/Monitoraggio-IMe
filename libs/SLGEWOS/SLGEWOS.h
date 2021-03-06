#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
//#include "libs/selfie.h"

#include "matrix.h"
#include "vector.h"

/**
 * @brief Inizializzazione della Inibition Table (prima parte)
 * 
 * @param n 
 * @param A 
 * @param s 
 * @param F 
 * @param K 
 * @param b 
 * @param X 
 */
void SLGEWOS_init(int n, double **A, double *s, double *F, double **K, double *b, double **X)
{
  int i, j;
  int rows = n;
  int cols = n;

  double tmpAdiag;
  /* Inizializzazione della Inibition Table e del vettore soluzione */
  for (i = 0; i < rows; i++)
  {
    tmpAdiag = 1 / A[i][i];
    for (j = 0; j < cols; j++)
    {
      if (i == j)
      {
        X[i][j] = tmpAdiag;
        // K[i][j]=1;
      }
      else
      {
        K[i][j] = A[j][i] * tmpAdiag;

        // ATTENTION : transposed
        X[j][i] = 0.0;
      }
    }
    F[i] = b[i];
    s[i] = 0.0;
  }
}

/**
 * @brief Calcolo della soluzione (seconda parte)
 *
 * @param s
 * @param n
 * @param K
 * @param H
 * @param F
 */
void SLGEWOS_risoluzione(double *s, int n, double **K, double *H, double *F, double **X)
{
  int i, j, l;
  int rows = n;
  int cols = n;

  /* Calcolo della soluzione */
  for (l = rows - 1; l > 0; l--)
  {
    for (i = 0; i < l; i++)
    {
      H[i] = 1 / (1 - K[i][l] * K[l][i]);
      for (j = 0; j < cols; j++)
      {
        X[i][j] = H[i] * (X[i][j] - K[i][l] * X[l][j]);
      }
      for (j = 0; j < l; j++)
      {
        if (j != i)
        {
          K[i][j] = H[i] * (K[i][j] - K[i][l] * K[l][j]);
        }
      }
    }
  }

  for (i = rows - 2; i >= 0; i--)
  {
    for (l = i + 1; l < rows; l++)
    {
      F[i] = F[i] - F[l] * K[l][i];
    }
  }

  for (j = 0; j < cols; j++)
  {
    for (l = 0; l < rows; l++)
    {
      s[j] = F[l] * X[l][j] + s[j];
    }
  }
}


/* ----------------------------------------------------------------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------------------------------------------- */

/*
A: ?? la matrice dei coefficienti, di dimensione n x n
b: ?? il vettore di termini costanti, di dimensione n
s: ?? il vettore soluzione, di dimensione n
n: ?? il numero tale per cui il sistema ?? a n equazioni e n incognite
K: ?? la seconda parte della Inibition Table, di dimensione n x n
H: ?? il vettore di Auxiliary Quantities, di dimensione n
F: ?? un vettore di appoggio in cui vengono inseriti tutti i termini di b, di dimensione n
*/
void SLGEWOS_calc(double **A, double *b, double *s, int n, double **K, double *H, double *F)
{
  int i, j, l;
  int rows = n;
  int cols = n;

  /* X: ?? la prima parte della Inibition Table, di dimensione n x n */
  double **X = A;
  double tmpAdiag;

  /* Inizializzazione della Inibition Table e del vettore soluzione */
  for (i = 0; i < rows; i++)
  {
    tmpAdiag = 1 / A[i][i];
    for (j = 0; j < cols; j++)
    {
      if (i == j)
      {
        X[i][j] = tmpAdiag;
        // K[i][j]=1;
      }
      else
      {
        K[i][j] = A[j][i] * tmpAdiag;

        // ATTENTION : transposed
        X[j][i] = 0.0;
      }
    }
    F[i] = b[i];
    s[i] = 0.0;
  }

  /* Calcolo della soluzione */
  for (l = rows - 1; l > 0; l--)
  {
    for (i = 0; i < l; i++)
    {
      H[i] = 1 / (1 - K[i][l] * K[l][i]);
      for (j = 0; j < cols; j++)
      {
        X[i][j] = H[i] * (X[i][j] - K[i][l] * X[l][j]);
      }
      for (j = 0; j < l; j++)
      {
        if (j != i)
        {
          K[i][j] = H[i] * (K[i][j] - K[i][l] * K[l][j]);
        }
      }
    }
  }

  for (i = rows - 2; i >= 0; i--)
  {
    for (l = i + 1; l < rows; l++)
    {
      F[i] = F[i] - F[l] * K[l][i];
    }
  }

  for (j = 0; j < cols; j++)
  {
    for (l = 0; l < rows; l++)
    {
      s[j] = F[l] * X[l][j] + s[j];
    }
  }
}

void SLGEWOS_calc_allocX(double **A, double *b, double *s, int n, double **X, double **K, double *H, double *F)
{
  int i, j, l;
  int rows = n;
  int cols = n;

  for (i = 0; i < rows; i++)
  {
    for (j = 0; j < cols; j++)
    {
      if (i == j)
      {
        X[i][j] = 1 / A[i][j];
        // K[i][j]=1;
      }
      else
      {
        X[i][j] = 0.0;
        K[i][j] = A[j][i] / A[i][i];
      }
    }
    F[i] = b[i];
    s[i] = 0.0;
  }

  for (l = rows - 1; l > 0; l--)
  {
    for (i = 0; i < l; i++)
    {
      H[i] = 1 / (1 - K[i][l] * K[l][i]);
      for (j = 0; j < cols; j++)
      {
        X[i][j] = H[i] * (X[i][j] - K[i][l] * X[l][j]);
      }
      for (j = 0; j < l; j++)
      {
        if (j != i)
        {
          K[i][j] = H[i] * (K[i][j] - K[i][l] * K[l][j]);
        }
      }
    }
  }

  for (i = rows - 2; i >= 0; i--)
  {
    for (l = i + 1; l < rows; l++)
    {
      F[i] = F[i] - F[l] * K[l][i];
    }
  }

  for (j = 0; j < cols; j++)
  {
    for (l = 0; l < rows; l++)
    {
      s[j] = F[l] * X[l][j] + s[j];
    }
  }
}

/*
A: ?? la matrice dei coefficienti, di dimensione n x n
b: ?? il vettore di termini costanti, di dimensione n
s: ?? il vettore soluzione, di dimensione n
n: ?? il numero tale per cui il sistema ?? a n equazioni e n incognite
*/
void SLGEWOS(double **A, double *b, double *s, int n)
{
  // double** X;

  /* K: ?? la seconda parte della Inibition Table, di dimensione n x n */
  double **K;

  /* H: ?? il vettore di Auxiliary Quantities, di dimensione n */
  double *H;

  /* F: ?? un vettore di appoggio in cui vengono inseriti tutti i termini di b, di dimensione n */
  double *F;

  // X=AllocateMatrix2D(n,n,CONTIGUOUS);
  K = AllocateMatrix2D(n, n, CONTIGUOUS);

  H = AllocateVector(n);
  F = AllocateVector(n);

  SLGEWOS_calc(A, b, s, n, K, H, F);
  // SLGEWOS_calc_allocX(A, b, s, n, X, K, H, F);

  // DeallocateMatrix2D(X,n,CONTIGUOUS);
  DeallocateMatrix2D(K, n, CONTIGUOUS);

  DeallocateVector(H);
  DeallocateVector(F);
}
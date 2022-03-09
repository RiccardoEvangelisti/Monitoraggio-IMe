#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../libs/SLGEWOS/matrix.h"
#include "../../libs/SLGEWOS/vector.h"

int main(int argc, char **argv)
{

	int index1;

	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
	{
		printf("\nUsage: SLGEWOS n repetitions verbose[0|1]\n\n");
		exit(-1);
	}

	if (atoi(argv[1]) <= 0)
	{
		printf("\nUsage: SLGEWOS n repetitions verbose[0|1]\n n > 0\n");
		exit(-1);
	}
	if (atoi(argv[2]) <= 0)
	{
		printf("\nUsage: SLGEWOS n repetitions verbose[0|1]\n repetitions > 0\n");
		exit(-1);
	}
	if (atoi(argv[3]) != 0 && atoi(argv[3]) != 1)
	{
		printf("\nUsage: SLGEWOS n repetitions verbose[0|1]\n repetitions > 0\n");
		exit(-1);
	}

	/*A: è la matrice dei coefficienti, di dimensione n x n */
	double **A;

	/*b: è il vettore di termini costanti, di dimensione n */
	double *b;

	/*K: è la seconda parte della Inibition Table, di dimensione n x n */
	double **K;

	/*H: è il vettore di Auxiliary Quantities, di dimensione n */
	double *H;

	/*F: è un vettore di appoggio in cui vengono inseriti tutti i termini di b, di dimensione n */
	double *F;

	/*s: è il vettore soluzione, di dimensione n */
	double *s;

	/*n: è il numero tale per cui il sistema è a n equazioni e n incognite */
	int n = atoi(argv[1]);

	int rows = n;
	int cols = n;

	/*numero di ripetizioni del calcolo */
	int repetitions = atoi(argv[2]);

	int verbose = atoi(argv[3]);

	printf("\nMatrix size: %dx%d", n, n);

	for (index1 = 0; index1 < repetitions; index1++)
	{

		printf("\n\n\nSTART Inhibition Method #%d\n", index1 + 1);

		
		///////////////////////////////////////////////////////////////
		// Allocazione vettori e matrici

		A = AllocateMatrix2D(rows, cols, CONTIGUOUS);
		b = AllocateVector(rows);

		FillMatrix2D(A, rows, cols);
		FillVector(b, rows, 1);

		//X=AllocateMatrix2D(n,n,CONTIGUOUS);
		K = AllocateMatrix2D(n, n, CONTIGUOUS);
		H = AllocateVector(n);
		F = AllocateVector(n);
		s = AllocateVector(n);

		if (verbose == 1)
		{
			printf("\n\n Matrix A:\n");
			PrintMatrix2D(A, rows, cols);
			printf("\n Vector b:\n");
			PrintVector(b, rows);
		}


		///////////////////////////////////////////////////////////////
		// Calcolo

		int i, j, l;
		int rows = n;
		int cols = n;

		/*X: è la prima parte della Inibition Table, di dimensione n x n */
		double **X = A;
		double tmpAdiag;

		/*
		  Prima parte: 
		  Inizializzazione della Inibition Table e del vettore soluzione 
		*/
		for (i = 0; i < rows; i++)
		{
			tmpAdiag = 1 / A[i][i];
			for (j = 0; j < cols; j++)
			{
				if (i == j)
				{
					X[i][j] = tmpAdiag;
					//K[i][j]=1;
				}
				else
				{
					K[i][j] = A[j][i] *tmpAdiag;

					// ATTENTION : transposed
					X[j][i] = 0.0;
				}
			}
			F[i] = b[i];
			s[i] = 0.0;
		}

		/*
		  Seconda parte:
		  Calcolo della soluzione 
		*/
		for (l = rows - 1; l > 0; l--)
		{
			for (i = 0; i < l; i++)
			{
				H[i] = 1 / (1 - K[i][l] *K[l][i]);
				for (j = 0; j < cols; j++)
				{
					X[i][j] = H[i] *(X[i][j] - K[i][l] *X[l][j]);
				}
				for (j = 0; j < l; j++)
				{
					if (j != i)
					{
						K[i][j] = H[i] *(K[i][j] - K[i][l] *K[l][j]);
					}
				}
			}
		}

		for (i = rows - 2; i >= 0; i--)
		{
			for (l = i + 1; l < rows; l++)
			{
				F[i] = F[i] - F[l] *K[l][i];
			}
		}

		for (j = 0; j < cols; j++)
		{
			for (l = 0; l < rows; l++)
			{
				s[j] = F[l] *X[l][j] + s[j];
			}
		}

		if (verbose == 1)
		{
			printf("\nThe IMe solution is:\n");
			PrintVector(s, rows);
		}

		

		///////////////////////////////////////////////////////////////
		// Deallocazione vettori e matrici

		DeallocateMatrix2D(A, n, CONTIGUOUS);
		//DeallocateMatrix2D(X,n,CONTIGUOUS);
		DeallocateMatrix2D(K, n, CONTIGUOUS);
		DeallocateVector(H);
		DeallocateVector(F);
		DeallocateVector(s);
		DeallocateVector(b);
		

		printf("\n\n\nEND Inhibition Method #%d\n", index1 + 1);

		// Pausa di qualche secondo 
		sleep(1);
	}
	return (0);
}
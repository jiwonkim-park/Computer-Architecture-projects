/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	/* 
	By using blocking, we'll enhance the temporal locality
	*/

	int block_size;
	int row_block, col_block;
	int diag, temp;

	if (N == 32) {
		block_size = 8;
		for (col_block = 0; col_block < M; col_block += block_size) {
			for (row_block = 0; row_block < N; row_block += block_size) {
				for (int i = row_block; i < row_block + block_size; i++) {
					for (int j = col_block; j < col_block + block_size; j++) {
						if (i != j) {
							B[j][i] = A[i][j];
						} else {
							temp = A[i][j];
							diag = i;
						}
					}
					if (row_block == col_block) {
						B[diag][diag] = temp;
					}
				}
			}
		}
	} else if (N == 64) {
		block_size = 8;
		/*
		Since just using blocking doesn't decrease the number of misses, I defined 7 more variables and reused variable 'temp' because I should not use variables more than 12.
		*/
		int temp1, temp2, temp3, temp4, temp5, temp6, temp7;
		for (col_block = 0; col_block < M; col_block += block_size) {
			for (row_block = 0; row_block < N; row_block += block_size) {

				for (int i = 0; i < 4; i++) {
					temp = A[col_block + i][row_block + 0];
					temp1 = A[col_block + i][row_block + 1];
					temp2 = A[col_block + i][row_block + 2];
					temp3 = A[col_block + i][row_block + 3];
					temp4 = A[col_block + i][row_block + 4];
					temp5 = A[col_block + i][row_block + 5];
					temp6 = A[col_block + i][row_block + 6];
					temp7 = A[col_block + i][row_block + 7];

					B[row_block + 0][col_block + i + 0] = temp;
					B[row_block + 0][col_block + i + 4] = temp5; //should be moved to B[col_block + i + 4][row_block + i]
					B[row_block + 1][col_block + i + 0] = temp1;
					B[row_block + 1][col_block + i + 4] = temp6;
					B[row_block + 2][col_block + i + 0] = temp2;
					B[row_block + 2][col_block + i + 4] = temp7;
					B[row_block + 3][col_block + i + 0] = temp3;
					B[row_block + 3][col_block + i + 4] = temp4;
				}

				temp = A[col_block + 4][row_block + 4];
				temp1 = A[col_block + 5][row_block + 4];
				temp2 = A[col_block + 6][row_block + 4];
				temp3 = A[col_block + 7][row_block + 4];
				temp4 = A[col_block + 4][row_block + 3];
				temp5 = A[col_block + 5][row_block + 3];
				temp6 = A[col_block + 6][row_block + 3];
				temp7 = A[col_block + 7][row_block + 3];

				B[row_block + 4][col_block + 0] = B[row_block + 3][col_block + 4];
				B[row_block + 4][col_block + 4] = temp;
				B[row_block + 3][col_block + 4] = temp4;
				B[row_block + 4][col_block + 1] = B[row_block + 3][col_block + 5];
				B[row_block + 4][col_block + 5] = temp1;
				B[row_block + 3][col_block + 5] = temp5;
				B[row_block + 4][col_block + 2] = B[row_block + 3][col_block + 6];
				B[row_block + 4][col_block + 6] = temp2;
				B[row_block + 3][col_block + 6] = temp6;
				B[row_block + 4][col_block + 3] = B[row_block + 3][col_block + 7];
				B[row_block + 4][col_block + 7] = temp3;
				B[row_block + 3][col_block + 7] = temp7;

				for (int i = 0; i < 3; i++) {
					temp = A[col_block + 4][row_block + i + 5];
					temp1 = A[col_block + 5][row_block + i + 5];
					temp2 = A[col_block + 6][row_block + i + 5];
					temp3 = A[col_block + 7][row_block + i + 5];
					temp4 = A[col_block + 4][row_block + i];
					temp5 = A[col_block + 5][row_block + i];
					temp6 = A[col_block + 6][row_block + i];
					temp7 = A[col_block + 7][row_block + i];

					B[row_block + i + 5][col_block + 0] = B[row_block + i][col_block + 4];
					B[row_block + i + 5][col_block + 4] = temp;
					B[row_block + i][col_block + 4] = temp4;
					B[row_block + i + 5][col_block + 1] = B[row_block + i][col_block + 5];
					B[row_block + i + 5][col_block + 5] = temp1;
					B[row_block + i][col_block + 5] = temp5;
					B[row_block + i + 5][col_block + 2] = B[row_block + i][col_block + 6];
					B[row_block + i + 5][col_block + 6] = temp2;
					B[row_block + i][col_block + 6] = temp6;
					B[row_block + i + 5][col_block + 3] = B[row_block + i][col_block + 7];
					B[row_block + i + 5][col_block + 7] = temp3;
					B[row_block + i][col_block + 7] = temp7;
				}
			}
		}
	} else {
		block_size = 16;
		for (col_block = 0; col_block < M; col_block += block_size) {
			for (row_block = 0; row_block < N; row_block += block_size) {
				for (int i = row_block; (i < N) && (i < row_block + block_size); i++) {
					for (int j = col_block; (j < M) && (j < col_block + block_size); j++) {
						if (i != j) {
							B[j][i] = A[i][j];
						} else {
							temp = A[i][j];
							diag = i;
						}
					}
					if (row_block == col_block) {
						B[diag][diag] = temp;
					}
				}
			}
		}
	}

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}


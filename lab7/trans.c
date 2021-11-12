/*
  Name: Choi Soomin
  Student ID: 20160169
  Login ID: choisium
*/

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
void trans_32(int M, int N, int A[N][M], int B[M][N]);
void trans_61(int M, int N, int A[N][M], int B[M][N]);
void trans_64(int M, int N, int A[N][M], int B[M][N]);

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
    if (M == 32) {
        trans_32(M, N, A, B);
    } else if (M == 61) {
        trans_61(M, N, A, B);
    } else {
        trans_64(M, N, A, B);
    }

}

void trans_32(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, p, q, K = 8;
    int diag, d, diag_exists;

    for (i = 0; i < N; i += K) {
        for (j = 0; j < M; j += K) {
            for (p = i; p < (i + K < N? i + K: N); p++) {
                diag_exists = 0;
                for (q = j; q < (j + K < M? j + K: M); q++) {
                    if (p == q) {
                        diag_exists = 1;
                        d = p;
                        diag = A[p][q];
                    } else {
                        B[q][p] = A[p][q];
                    }
                }
                if (diag_exists) {
                    B[d][d] = diag;
                }
            }
        }
    }
}


void trans_64(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, p, q, k, r, tmp;
    for (i = 0; i < M; i += 8) {
        for (j = 0; j < N; j += 8) {
            // transpose all 4*4 matrices by dividing upper and lower half
            for (r = 0; r < 2; r++) {
                // move A(i, j) to B(j, i)
                for (p = 0; p < 2; p++) {   // move A's 1, 2 rows to B's 3, 4 rows
                    for (q = 0; q < 8; q++) {
                        B[j+2+p +4*r][i+q] = A[i+p +4*r][j+q];
                    }
                }
                for (p = 0; p < 2; p++) {   // move A's 3, 4 rows to B's 1, 2 rows
                    for (q = 0; q < 8; q++) {
                        B[j+p +4*r][i+q] = A[i+2+p +4*r][j+q];
                    }
                }
                for (p = 0; p < 2; p++) {   // swap B's 1, 2 rows and 3, 4 rows
                    for (q = 0; q < 8; q++) {
                        tmp = B[j+p +4*r][i+q];
                        B[j+p +4*r][i+q] = B[j+2+p +4*r][i+q];
                        B[j+2+p +4*r][i+q] = tmp;
                    }
                }

                // transpose two 4*4 matrices
                for (k = 0; k < 2; k++) {
                    for (p = 0; p < 4; p++) {
                        for (q = p+1; q < 4; q++) {
                            tmp = B[j+p +4*r][i+q +k*4];
                            B[j+p +4*r][i+q +k*4] = B[j+q +4*r][i+p +k*4];
                            B[j+q +4*r][i+p +k*4] = tmp;
                        }
                    }
                }

                if (r == 0) {
                    for (p = 0; p < 2; p++) {   // swap B's 1, 2 rows and 3, 4 rows
                        for (q = 4; q < 8; q++) {
                            tmp = B[j+p +4*r][i+q];
                            B[j+p +4*r][i+q] = B[j+2+p +4*r][i+q];
                            B[j+2+p +4*r][i+q] = tmp;
                        }
                    }
                } else {
                    for (p = 0; p < 2; p++) {   // swap B's 1, 2 rows and 3, 4 rows
                        for (q = 0; q < 4; q++) {
                            tmp = B[j+p +4*r][i+q];
                            B[j+p +4*r][i+q] = B[j+2+p +4*r][i+q];
                            B[j+2+p +4*r][i+q] = tmp;
                        }
                    }
                }
            }

            for (p = 0; p < 2; p++) {
                for (q = 0; q < 4; q++) {
                    tmp = B[j+p + 4][i+q];
                    B[j+p + 4][i+q] = B[j+p + 2][i+q + 4];
                    B[j+p + 2][i+q + 4] = tmp;
                }
            }

            for (p = 0; p < 2; p++) {
                for (q = 0; q < 4; q++) {
                    tmp = B[j+p + 6][i+q];
                    B[j+p + 6][i+q] = B[j+p][i+q + 4];
                    B[j+p][i+q + 4] = tmp;
                }
            }
        }
    }
}

void trans_61(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, p, q, K = 16;
    int diag, d, diag_exists;

    for (i = 0; i < N; i += K) {
        for (j = 0; j < M; j += K) {
            for (p = i; p < (i + K < N? i + K: N); p++) {
                diag_exists = 0;
                for (q = j; q < (j + K < M? j + K: M); q++) {
                    if (p == q) {
                        diag_exists = 1;
                        d = p;
                        diag = A[p][q];
                    } else {
                        B[q][p] = A[p][q];
                    }
                }
                if (diag_exists) {
                    B[d][d] = diag;
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

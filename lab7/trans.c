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
    } else if (M == 64) {
        trans_64(M, N, A, B);
    } else {
        trans_61(M, N, A, B);
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
    int ax, ay, bx, by, next_x, next_y, next_x2, next_y2;
    int p, q;

    for (ay = 0; ay < M; ay += 8) { // access a by row-major
        for (ax = 0; ax < N; ax += 8) {
            // compute b index and next indices
            bx = ay; by = ax;
            
            next_y = by;
            next_x = bx;
            while (next_y == ay || next_y == by) {
                next_y += 8;
                if (next_y >= N) {
                    next_x += 8;
                    next_y -= N;
                }
            }

            next_y2 = next_y;
            next_x2 = next_x;
            while (next_y2 == ay || next_y2 == by || next_y2 == next_y) {
                next_y2 += 8;
                if (next_y2 >= N) {
                    next_x2 += 8;
                    next_y2 -= N;
                }
            }

            if (next_x2 >= M) {
                for (p = 0; p < 8; p++) {
                    for (q = 0; q < 8; q++) {
                        B[bx + q][by + p] = A[ax + p][ay + q];
                    }
                }
            } else {
                // move A to next Bs
                for (p = 0; p < 4; p++) {
                    for (q = 0; q < 8; q++) {
                        B[next_x + p][next_y + q] = A[ax + p][ay + q];
                    }
                }

                for (p = 0; p < 4; p++) {
                    for (q = 0; q < 8; q++) {
                        B[next_x2 + p][next_y2 + q] = A[ax + 4 + p][ay + q];
                    }
                }   

                // move nextBs to currentB
                for (p = 0; p < 4; p++) {
                    for (q = 0; q < 4; q++) {
                        B[bx + q][by + p] = B[next_x + p][next_y + q];
                        B[bx + q][by + 4 + p] = B[next_x2 + p][next_y2 + q];
                    }
                }

                for (p = 0; p < 4; p++) {
                    for (q = 0; q < 4; q++) {
                        B[bx + 4 + q][by + p] = B[next_x + p][next_y + 4 + q];
                        B[bx + 4 + q][by + 4 + p] = B[next_x2 + p][next_y2 + 4 + q];
                    }
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

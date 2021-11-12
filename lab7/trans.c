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
    // 무조건 row-major로 접근하도록 해야 될듯
    // 캐시에 저장되는 방식 - 한 block에 8개, 총 set은 32개
    // 아래는 block의 개수 및 set에 들어가는걸 나타냄.
    //  0  1  2  3  4  5  6  7
    //  8  9 10 11 12 13 14 15
    // 16 17 18 19 20 21 22 23
    // 24 25 26 27 28 29 30 31
    // 이렇게 4줄씩 set에 딱 들어감. 총 64줄이 있음.
    // 따라서 4줄 단위로 처리해야 함
    int i, j, k, p, q, tmp;

    // Move A to B
    for (i = 0; i < N; i += 4) {
        // Move A's first 4 rows to B's first 4 rows.
        for (k = 0; k < 2; k++) {
            // load A's 1, 2 rows to B's 3, 4 rows
            for (j = 0; j < M; j++) {   // 32 miss
                B[i+2+k][j] = A[i+k][j];
            }
            // load A's 3, 4 rows to B's 1, 2 rows
            for (j = 0; j < M; j++) {   // 32 miss
                B[i+k][j] = A[i+2+k][j];
            }
            // swap B's 3, 4 rows with 1, 2 rows.
            for (j = 0; j < M; j++) {   // 16 miss
                tmp = B[i+k][j];
                B[i+k][j] = B[i+2+k][j];
                B[i+2+k][j] = tmp;
            }
        }
    }
    
    // Do transpose in every 8*8 matrices
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            // transpose upper two 4*4 matrices
            for (p = 0; p < 4; p++) {
                for (q = p+1; q < 4; q++) {
                    tmp = B[i+p][j+q];
                    B[i+p][j+q] = B[i+q][j+p];
                    B[i+q][j+p] = tmp;
                }
                for (q = p+1; q < 4; q++) {
                    tmp = B[i+p][j+4+q];
                    B[i+p][j+4+q] = B[i+q][j+4+p];
                    B[i+q][j+4+p] = tmp;
                }
            }
            // transpose lower two 4*4 matrices
            for (p = 0; p < 4; p++) {
                for (q = p+1; q < 4; q++) {
                    tmp = B[i+4+p][j+q];
                    B[i+4+p][j+q] = B[i+4+q][j+p];
                    B[i+4+q][j+p] = tmp;
                }
                for (q = p+1; q < 4; q++) {
                    tmp = B[i+4+p][j+4+q];
                    B[i+4+p][j+4+q] = B[i+4+q][j+4+p];
                    B[i+4+q][j+4+p] = tmp;
                }
            }
            // swap upper right 4*4 matrix and lower left 4*4 matrix
            for (p = 0; p < 2; p++) { // 0, 1 row -> 6, 7 row
                for (q = 0; q < 4; q++) {
                    tmp = B[i+p][j+4+q];
                    B[i+p][j+4+q] = B[i+6+p][j+q];
                    B[i+6+p][j+q] = tmp;
                }
            }
            for (p = 2; p < 4; p++) { // 2, 3 row -> 4, 5 row
                for (q = 0; q < 4; q++) {
                    tmp = B[i+p][j+4+q];
                    B[i+p][j+4+q] = B[i+2+p][j+q];
                    B[i+2+p][j+q] = tmp;
                }
            }
            for (p = 0; p < 2; p++) { // swap 0, 1 row with 2, 3 row
                for (q = 0; q < 4; q++) {
                    tmp = B[i+p][j+4+q];
                    B[i+p][j+4+q] = B[i+2+p][j+4+q];
                    B[i+2+p][j+4+q] = tmp;
                }
            }
            for (p = 4; p < 6; p++) { // swap 4, 5 row with 6, 7 row
                for (q = 0; q < 4; q++) {
                    tmp = B[i+p][j+q];
                    B[i+p][j+q] = B[i+2+p][j+q];
                    B[i+2+p][j+q] = tmp;
                }
            }
        }
    }

    // swap 8*8 matrices
    for (i = 0; i < N; i += 8) {
        for (j = i + 8; j < M; j += 8) {
            for (p = 0; p < 8; p++) {
                for (q = 0; q < 8; q++) {
                    tmp = B[i+p][j+q];
                    B[i+p][j+q] = B[j+p][i+q];
                    B[j+p][i+q] = tmp;
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

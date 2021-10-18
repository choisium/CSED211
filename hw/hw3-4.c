#include <stdio.h>
#include <assert.h>

void psum1(float a[], float p[], long n) {
    long i;
    p[0] = a[0];
    for (i = 1; i < n; i++)
        p[i] = p[i-1] + a[i];   // p[i] = Sigma(0 to i) a[i]
}

void psum2(float a[], float p[], long n) {
    long i;
    p[0] = a[0];
    for (i = 1; i < n-1; i+=2) {  // is this unrolling?
        float mid_val = p[i-1] + a[i];
        p[i] = mid_val;
        p[i+1] = mid_val + a[i+1];
    }
    if (i < n)
        p[i] = p[i-1] + a[i];
}

// don't need to repeatedly retrieve the value of p[i] from memory
// CPE limited by the latency of floating-point addition
void psum1a(float a[], float p[], long n) {
    long i;
    float last_val, val;
    last_val = p[0] = a[0];
    for (i = 1; i < n; i++) {
        val = last_val + a[i];
        p[i] = val;
        last_val = val;
    }
}

// how to make it CPE less than the latency of floating-point addition?
// let's make it bounded by throughput. Use unrolling and reassociation.
// 2-way
void psum2a(float a[], float p[], long n) {
    long i;
    float last_val, val1, val2;
    last_val = p[0] = a[0];
    for (i = 1; i < n-1; i+=2) {
        val1 = last_val + a[i];
        val2 = val1 + a[i+1];
        p[i] = val1;
        p[i+1] = val2;
        last_val = val2; // sum up to a[2k]
    }
    if (i < n)  // when n is even
        p[i] = last_val + a[i];
}

// 4-way
// or partial sum of odd / even -> lastly add together?
void psum4a(float a[], float p[], long n) {
    long i;
    float last_val;
    float val1, val2, val3, val4;
    last_val = p[0] = a[0];  // take care of odd index
    for (i = 1; i < n-3; i+=4) {
        val1 = last_val + a[i];
        val2 = val1 + a[i+1];
        val3 = val2 + a[i+2];
        val4 = val3 + a[i+3];
        p[i] = val1;
        p[i+1] = val2;
        p[i+2] = val3;
        p[i+3] = val4;
        last_val = val4; // sum up to a[4k]
    }
    for (; i < n; i++)  // when n is even
    {
        last_val += a[i];
        p[i] = last_val;
    }       
}


#define LOOP 1000
#define LEN  1000

int main(int argc, char* argv[]) {
    float a[5] = { 1, 2, 3, 4, 5 };
    float p[5];
    psum1a(a, p, 5);
    assert(p[4] == 15);

    float q[5];
    psum2a(a, q, 5);
    assert(q[4] == 15);

    float r[5];
    psum4a(a, r, 5);
    assert(r[4] == 15);

    /* for prof */
    for (int i = 0; i < LOOP; i++) {
        float s[LEN];
        float d[LEN];
        psum1a(s, d, LEN);
        psum2a(s, d, LEN);
        psum4a(s, d, LEN);
    }
    return 0;
}

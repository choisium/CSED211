#include <stdio.h>

int func4(int x, int y, int a, int depth) {
    if (depth > 6) return 0;
    int result = x - y;
    int tmp = (int) ((unsigned) result >> 31);
    tmp = (tmp + result) >> 1;
    tmp = tmp + y;
    if (a < tmp) {
        x = tmp - 1;
        result = func4(x, y, a, depth + 1);
        tmp += result;
    } else if (a > tmp) {
        y = tmp + 1;
        result = func4(x, y, a, depth + 1);
        tmp += result;
    }
    result = tmp;
    return result;
}

// func4의 result가 43이어야 한다.
int main() {
    int res = 0;
    for (int a = 0; a <= 14; a++) {
        res = func4(14, 0, a, 0);
        printf("a=%d, res=%d\n", a, res);
        if (res == 43) break;
    }
    return 0;
}


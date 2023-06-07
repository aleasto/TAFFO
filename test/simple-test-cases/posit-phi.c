///TAFFO_TEST_ARGS -Xvra -propagate-all -Xdta -useposit -lTaffoPosit
#include <stdio.h>

float fun(int b) {
    float __attribute__((annotate("scalar()"))) x = -1;
    if (b > 1) {
        x = b;
    }
    return x;
}

int main(int argc) {
    float __attribute__((annotate("scalar()"))) x = fun(argc);
    printf("%d\n", (int)x);
    return 0;
}

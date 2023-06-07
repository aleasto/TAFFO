///TAFFO_TEST_ARGS -Xvra -propagate-all -Xdta -useposit -lTaffoPosit

#include <stdio.h>

int main() {
    float __attribute__((annotate("scalar(range(-100.0, 100.0))"))) x;

    double tmp;
    scanf("%lf", &tmp);
    x = tmp;

    int i = x;
    printf("Integer: %d\n", i);

    float f = x;
    printf("Single precision: %.17g\n", f);

    double d = x;
    printf("Double precision: %.17g\n", d);

    x = i;
    printf("Integer to Posit: %.17g\n", x);
}

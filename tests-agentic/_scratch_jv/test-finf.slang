//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    // Use large multiplication to produce +inf.
    float big = 3.4e38f;
    float infv = big * 10.0f;
    // CHECK: isinf-pos=1
    printf("isinf-pos=%d\n", int(isinf(infv)));
    float ninfv = -big * 10.0f;
    // CHECK: isinf-neg=1
    printf("isinf-neg=%d\n", int(isinf(ninfv)));
    // NaN from 0/0
    float nanv = 0.0f / 0.0f;
    // CHECK: isnan=1
    printf("isnan=%d\n", int(isnan(nanv)));
}

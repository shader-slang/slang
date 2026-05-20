//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float big = 3.4e38f;
    float infv = big * 10.0f;
    // +inf > any finite
    // CHECK: gt-max=1
    printf("gt-max=%d\n", int(infv > big));
    // -inf < any finite
    float ninfv = -infv;
    // CHECK: lt-min=1
    printf("lt-min=%d\n", int(ninfv < -big));
    // NaN != NaN per IEEE 754
    float nanv = 0.0f / 0.0f;
    // CHECK: nan-self-eq=0
    printf("nan-self-eq=%d\n", int(nanv == nanv));
}

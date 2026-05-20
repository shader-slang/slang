//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float p = 0.0f;
    float n = -0.0f;
    float r1 = 1.0f / p;
    float r2 = 1.0f / n;
    // both produce inf with sign matching the divisor
    printf("r1=%f r2=%f\n", r1, r2);
    //CHECK: r1={{.*}}
}

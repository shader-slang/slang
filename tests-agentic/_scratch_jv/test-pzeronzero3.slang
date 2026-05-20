//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float p = 0.0f;
    float n = -0.0f;
    // CHECK: eq=1
    printf("eq=%d\n", int(p == n));
    // Negative zero divided into a positive yields -inf vs +inf differences.
    float r1 = 1.0f / p;
    float r2 = 1.0f / n;
    // CHECK: same={{[01]}}
    printf("same=%d\n", int(r1 == r2));
}

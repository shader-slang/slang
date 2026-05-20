//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float p = 0.0f;
    float n = -0.0f;
    // CHECK: eq=1
    printf("eq=%d\n", int(p == n));
    uint pb = asuint(p);
    uint nb = asuint(n);
    // CHECK: pbits=0
    printf("pbits=%u\n", pb);
    // CHECK: nbits=2147483648
    printf("nbits=%u\n", nb);
}

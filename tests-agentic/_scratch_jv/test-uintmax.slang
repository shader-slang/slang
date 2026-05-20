//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    uint a = 0xFFFFFFFFu;
    uint b = a + 1u;
    // CHECK: a=4294967295
    printf("a=%u\n", a);
    // CHECK: b=0
    printf("b=%u\n", b);
}

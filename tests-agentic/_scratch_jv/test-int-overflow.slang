//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    uint a = uint(0xFFFFFFFF);
    uint b = a + 1u;
    // CHECK: a=4294967295
    printf("a=%u\n", a);
    // CHECK: b=0
    printf("b=%u\n", b);
}

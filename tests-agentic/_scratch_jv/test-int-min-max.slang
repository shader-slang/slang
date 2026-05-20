//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int a = int(-2147483648);
    int b = int(2147483647);
    // CHECK: a=-2147483648
    printf("a=%d\n", a);
    // CHECK: b=2147483647
    printf("b=%d\n", b);
}

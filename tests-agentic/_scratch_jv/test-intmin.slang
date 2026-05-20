//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int a = -2147483648;
    int b = 2147483647;
    // CHECK: min=-2147483648
    printf("min=%d\n", a);
    // CHECK: max=2147483647
    printf("max=%d\n", b);
}

//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int a[1];
    a[0] = 99;
    // CHECK: a0=99
    printf("a0=%d\n", a[0]);
}

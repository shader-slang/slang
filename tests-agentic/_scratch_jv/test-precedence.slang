//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int a = 2;
    int b = 3;
    int c = 4;
    // * binds tighter than +
    int r = a + b * c;
    // CHECK: r=14
    printf("r=%d\n", r);
    int r2 = (a + b) * c;
    // CHECK: r2=20
    printf("r2=%d\n", r2);
}

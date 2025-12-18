//TEST:INTERPRET(filecheck=CHECK):

enum MyEnum
{
    A,
    B = A,
    C
}

void main()
{
    // CHECK: A = 0
    printf("A = %d\n", MyEnum.A);
    // CHECK: B = 0
    printf("B = %d\n", MyEnum.B);
    // CHECK: C = 1
    printf("C = %d\n", MyEnum.C);
}
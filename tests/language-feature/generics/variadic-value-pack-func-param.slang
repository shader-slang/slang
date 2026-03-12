//TEST:INTERPRET(filecheck=CHECK):

void add(inout int a, int b) { a += b; }

void printEach<let each D : int>()
{
    expand printf("%d\n", each D);
}

void mul(inout int a, int b) { a *= b; }

int product<let each D : int>()
{
    int p = 1;
    expand mul(p, each D);
    return p;
}

void main()
{
    printEach<5, 10, 15>();
    // CHECK: 5
    // CHECK: 10
    // CHECK: 15

    printf("%d\n", product<2, 3, 4>());
    // CHECK: 24
}

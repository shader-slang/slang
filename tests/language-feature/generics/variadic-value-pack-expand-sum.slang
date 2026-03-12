//TEST:INTERPRET(filecheck=CHECK):

void add(inout int a, int b) { a += b; }

int sum<let each D : int>()
{
    int s = 0;
    expand add(s, each D);
    return s;
}

void main()
{
    printf("%d\n", sum<1, 2, 3>());
    // CHECK: 6

    printf("%d\n", sum<10, 20, 30, 40>());
    // CHECK: 100
}

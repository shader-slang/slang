//TEST:INTERPRET(filecheck=CHECK):

void add(inout int a, int b) { a += b; }

int weightedSum<let scale : int, let each D : int>()
{
    int s = 0;
    expand add(s, scale * each D);
    return s;
}

void main()
{
    printf("%d\n", weightedSum<2, 1, 2, 3>());
    // CHECK: 12

    printf("%d\n", weightedSum<10, 5, 6>());
    // CHECK: 110
}

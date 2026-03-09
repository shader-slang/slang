//TEST:INTERPRET(filecheck=CHECK):

void add(inout int a, int b) { a += b; }

// Nested expand: expand ((each D) + 1) * 2
int nestedExpandSum<let each D : int>()
{
    int s = 0;
    expand add(s, ((each D) + 1) * 2);
    return s;
}

void main()
{
    // D = {1, 2, 3}
    // ((1)+1)*2 = 4, ((2)+1)*2 = 6, ((3)+1)*2 = 8
    // sum = 18
    printf("%d\n", nestedExpandSum<1, 2, 3>());
    // CHECK: 18

    // D = {10}
    // ((10)+1)*2 = 22
    printf("%d\n", nestedExpandSum<10>());
    // CHECK: 22
}

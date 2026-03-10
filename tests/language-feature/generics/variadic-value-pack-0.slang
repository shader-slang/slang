//TEST:INTERPRET(filecheck=CHECK):

struct Dims<let each D : int>
{
    int getDimCount() { return countof(D); }
}

void main()
{
    Dims<1, 2, 3> d;
    printf("%d\n", d.getDimCount());
    // CHECK: 3

    Dims<10, 20> d2;
    printf("%d\n", d2.getDimCount());
    // CHECK: 2
}

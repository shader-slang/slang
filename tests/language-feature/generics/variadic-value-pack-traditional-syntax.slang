//TEST:INTERPRET(filecheck=CHECK):

struct Dims<each int D>
{
    int getDimCount() { return countof(D); }
    void print()
    {
        expand printf("%d\n", each D);
    }
}

void main()
{
    Dims<1, 2, 3> d;
    printf("%d\n", d.getDimCount());
    // CHECK: 3

    d.print();
    // CHECK: 1
    // CHECK: 2
    // CHECK: 3
}

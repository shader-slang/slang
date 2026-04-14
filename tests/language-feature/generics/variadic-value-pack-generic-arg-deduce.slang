//TEST:INTERPRET(filecheck=CHECK):

struct Dims<let each D : int>
{
    void print()
    {
        expand printf("%d\n", each D);
    }
}

void printDims<let each D : int>(Dims<D> d)
{
    d.print();
}

void main()
{
    Dims<10, 20, 30> d;
    printDims(d);
    // CHECK: 10
    // CHECK: 20
    // CHECK: 30
}

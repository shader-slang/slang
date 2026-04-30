//TEST:INTERPRET(filecheck=CHECK):

struct Dims<let each D : int>
{
    int getDimCount() { return countof(D); }
    void print()
    {
        expand printf("%d\n", each D);
    }
}

Dims<A, expand (each B)+1> merge<let each A : int, let each B : int>()
{
    return Dims<A, expand (each B)+1>();
}

void printFunc(Dims<1,2,4,5> d)
{
    d.print();
}

void main()
{
    let d = merge<1, 2, 3, 4>();
    printf("%d\n", d.getDimCount());
    d.print();
    // CHECK: 4
    // CHECK: 1
    // CHECK: 2
    // CHECK: 4
    // CHECK: 5

    // Check that the substituted return type of merge matches a concrete type.
    printFunc(merge<1, 2, 3, 4>());
    // CHECK: 1
    // CHECK: 2
    // CHECK: 4
    // CHECK: 5
}

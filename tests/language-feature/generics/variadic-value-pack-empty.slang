//TEST:INTERPRET(filecheck=CHECK):

struct Dims<let each D : int>
{
    int getDimCount() { return countof(D); }
}

void printDimCount<let each D : int>()
{
    Dims<D> d;
    printf("%d\n", d.getDimCount());
}

void main()
{
    printDimCount<>();
    // CHECK: 0

    printDimCount<1>();
    // CHECK: 1

    printDimCount<1, 2, 3>();
    // CHECK: 3
}

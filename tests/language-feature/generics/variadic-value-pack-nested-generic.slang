//TEST:INTERPRET(filecheck=CHECK):

struct Inner<let N : int>
{
    int get() { return N * N; }
}

struct Outer<let each D : int>
{
    void printSquares()
    {
        expand printf("%d\n", Inner<each D>().get());
    }
    
    void add(inout int a, int b) { a += b; }
    
    int sumSquares()
    {
        int s = 0;
        expand add(s, Inner<each D>().get());
        return s;
    }
}

void main()
{
    Outer<1, 2, 3> o;
    o.printSquares();
    // CHECK: 1
    // CHECK: 4
    // CHECK: 9

    printf("%d\n", o.sumSquares());
    // CHECK: 14
}

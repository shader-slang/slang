//TEST:INTERPRET(filecheck=CHECK):

struct Wrapper<let N : int>
{
    int getValue() { return N; }
}

struct WrapAll<let each D : int>
{
    void printValues()
    {
        expand printf("%d\n", Wrapper<each D>().getValue());
    }
}

void main()
{
    WrapAll<10, 20, 30> w;
    w.printValues();
    // CHECK: 10
    // CHECK: 20
    // CHECK: 30
}

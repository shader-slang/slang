//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

interface IFoo
{
    int getVal() { return 0; }
}

struct Impl : IFoo
{
    // Missing override for getVal, which should trigger a diagnostic.
    // CHECK: ([[# @LINE+1]]): error 30853
    int getVal() { return 1; }
}

struct Impl2 : IFoo
{
    // Overriding getVal with a different signature should also trigger a diagnostic.
    // CHECK: ([[# @LINE+1]]): error 30854
    override int getVal(int x) { return x; }
}
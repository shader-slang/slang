// Test that __subscript without return type produces proper error message

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct MyType
{
    int val[12];

    __subscript(int x, int y)
//CHECK:       ^ __subscript declaration must have a return type specified after '->'
    {
        get { return val[x*3 + y]; }
    }
}

// This should compile fine - subscript with return type
struct MyType2
{
    int val[12];
    __subscript(int x, int y) -> int
    {
        get { return val[x*3 + y]; }
    }
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    MyType2 obj;
    int v = obj[1, 2];
} 
// A write through a pointer whose access qualifier is a generic parameter cannot be proven
// to be allowed, so it warns for now; reading through it does not (shader-slang/slang#14462).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0): -target spirv -entry main -stage compute

struct Item
{
    uint count;
}

void bump(inout uint x)
{
    x += 1;
}

RWStructuredBuffer<uint> outputBuffer;

void writeThroughGenericAccessPtr<Access access>(Ptr<Item, access> p)
{
    outputBuffer[0] = p->count;
    p->count = 1;
/*CHECK:
       ^^^^^ writing through a pointer whose access qualifier is not known
       ^^^^^ the access qualifier of the pointer is a generic parameter
*/
    bump(p->count);
//CHECK:    ^^^^^ writing through a pointer whose access qualifier is not known
//CHECK:    ^^^^^ the access qualifier of the pointer is a generic parameter
}

[numthreads(1, 1, 1)]
void main()
{
}

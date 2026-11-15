// A pointer type whose access qualifier is cast from an integer outside the `Access` enum is
// rejected where the type is written, instead of reaching the value-category mapping of its
// pointee and aborting the compiler (shader-slang/slang#14462).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

struct Item
{
    uint count;
}

RWStructuredBuffer<uint> outputBuffer;

void readThroughInvalidAccess(Ptr<Item, (Access)3> p)
//CHECK:                      ^^^ invalid access qualifier for a pointer or reference type
//CHECK:                      ^^^ the access qualifier must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
{
    outputBuffer[0] = p->count;
    outputBuffer[1] = (*p).count;
}

[numthreads(1, 1, 1)]
void main()
{
    let q = Ptr<Item, (Access)3>(0ULL);
//CHECK:    ^^^ invalid access qualifier for a pointer or reference type
//CHECK:    ^^^ the access qualifier must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
    outputBuffer[2] = q->count;
}

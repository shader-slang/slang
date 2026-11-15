// An `Access` argument cast from an integer outside the `Access` enum is rejected where the
// argument is given, instead of reaching a pointer type and aborting the compiler when the
// pointer is dereferenced (shader-slang/slang#14462).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

struct Item
{
    uint count;
}

struct Box<let A : Access>
{
    Ptr<Item, A> p;
}

typealias PtrWith<let A : Access> = Ptr<Item, A>;

void takeAccess<let A : Access>()
{
}

RWStructuredBuffer<uint> outputBuffer;

void readThroughInvalidAccess(Ptr<Item, (Access)3> p)
//CHECK:                                        ^ invalid access qualifier
//CHECK:                                        ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
{
    outputBuffer[0] = p->count;
}

void readThroughInvalidAccessField(Box<(Access)3> b)
//CHECK:                                       ^ invalid access qualifier
//CHECK:                                       ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
{
    outputBuffer[1] = b.p->count;
}

void readThroughInvalidAccessAlias(PtrWith<(Access)3> p)
//CHECK:                                           ^ invalid access qualifier
//CHECK:                                           ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
{
    outputBuffer[2] = p->count;
}

[numthreads(1, 1, 1)]
void main()
{
    let q = Ptr<Item, (Access)3>(0ULL);
//CHECK:                      ^ invalid access qualifier
//CHECK:                      ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
    outputBuffer[3] = q->count;
    takeAccess<(Access)3>();
//CHECK:               ^ invalid access qualifier
//CHECK:               ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
}

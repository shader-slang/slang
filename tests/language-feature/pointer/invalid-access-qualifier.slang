// An `Access` value cast from an integer outside the `Access` enum is rejected instead of
// aborting the compiler when the pointer is dereferenced (shader-slang/slang#14462). A value
// given as an argument or default is rejected where it is written; a value computed by
// substitution is rejected where the pointer is dereferenced.

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

// A default stands in for every omitted argument, so it is checked where it is declared.
struct BoxWithDefault<let A : Access = (Access)3>
//CHECK:                                       ^ invalid access qualifier
//CHECK:                                       ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
{
    Ptr<Item, A> p;
}

// `ComputedBox<3>` gives no argument of type `Access`; substitution computes `(Access)3`.
struct ComputedBox<let N : int>
{
    Ptr<Item, (Access)N> p;
}

// A default that depends on an earlier parameter is computed the same way.
struct DependentDefaultBox<let N : int, let A : Access = (Access)N>
{
    Ptr<Item, A> p;
}

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

void readThroughInvalidDefault(BoxWithDefault b)
{
    outputBuffer[4] = b.p->count;
}

void readThroughComputedAccess(ComputedBox<3> b)
{
    outputBuffer[5] = b.p->count;
//CHECK:                ^ invalid access qualifier
//CHECK:                ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
    outputBuffer[6] = b.p[0].count;
//CHECK:                ^ invalid access qualifier
//CHECK:                ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
    outputBuffer[7] = (*b.p).count;
//CHECK:              ^ invalid access qualifier
//CHECK:              ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
    Item item = *b.p;
//CHECK:        ^ invalid access qualifier
//CHECK:        ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
    outputBuffer[8] = item.count;
}

// The write reports only the invalid access, not that the target is read-only.
void writeThroughComputedAccess(ComputedBox<3> b)
{
    b.p->count = 1;
/*CHECK:
      ^ invalid access qualifier
      ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
*/
}

void readThroughDependentDefault(DependentDefaultBox<3> b)
{
    outputBuffer[9] = b.p->count;
//CHECK:                ^ invalid access qualifier
//CHECK:                ^ an 'Access' value must be 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'
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

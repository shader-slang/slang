// Members reached through `->` on a read-only pointer must not be writable, matching
// the behavior of `(*p).member` (shader-slang/slang#14462).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

struct Item
{
    uint count;
    float value;
}

void bump(inout uint x)
{
    x += 1;
}

RWStructuredBuffer<uint> outputBuffer;

void writeThroughReadPtr(Ptr<Item, Access.Read> p)
{
    p->count = 1;
//CHECK:     ^ left of '=' is not an l-value
//CHECK:     ^ left of '=' is not an l-value.
    p->count += 1;
/*CHECK:
       ^^^^^ argument must be l-value
       ^^^^^ argument passed to parameter '0' must be l-value.
*/
    bump(p->count);
//CHECK:    ^^^^^ argument must be l-value
//CHECK:    ^^^^^ argument passed to parameter '0' must be l-value.
    InterlockedAdd(p->count, 1u);
//CHECK:              ^^^^^ argument must be l-value
//CHECK:              ^^^^^ argument passed to parameter '0' must be l-value.
    (*p).count = 1;
//CHECK:       ^ left of '=' is not an l-value
//CHECK:       ^ left of '=' is not an l-value.
}

void writeThroughImmutablePtr(Ptr<Item, Access.Immutable> p)
{
    p->count = 1;
//CHECK:     ^ left of '=' is not an l-value
//CHECK:     ^ left of '=' is not an l-value.
}

void writeThroughImmutablePtrAlias(ImmutablePtr<Item> p)
{
    p->count = 1;
//CHECK:     ^ left of '=' is not an l-value
//CHECK:     ^ left of '=' is not an l-value.
}

// The cases below must compile without diagnostics.

void readThroughReadPtr(Ptr<Item, Access.Read> p)
{
    outputBuffer[0] = p->count;
    outputBuffer[1] = uint(p->value);
}

void writeThroughReadWritePtr(Ptr<Item, Access.ReadWrite> p)
{
    p->count = 1;
    p->count += 1;
    bump(p->count);
    InterlockedAdd(p->count, 1u);
}

void writeThroughDefaultPtr(Ptr<Item> p)
{
    p->count = 1;
}

[numthreads(1, 1, 1)]
void main()
{
}

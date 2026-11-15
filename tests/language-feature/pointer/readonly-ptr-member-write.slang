// Members reached through `->` on a read-only pointer must not be writable, matching
// the behavior of `(*p).member` (shader-slang/slang#14462).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute -experimental-feature

struct Item
{
    uint count;
    float value;

    [mutating]
    void reset()
    {
        count = 0;
    }
}

void bump(inout uint x)
{
    x += 1;
}

void readOnlyRef(__ref_readonly uint x)
{
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
    p.count = 1;
//CHECK:    ^ left of '=' is not an l-value
//CHECK:    ^ left of '=' is not an l-value.
    p->reset();
//CHECK:    ^ mutating method cannot be called on immutable value
//CHECK:    ^ mutating method 'reset' cannot be called on an immutable value
    p[0].count = 1;
//CHECK:       ^ left of '=' is not an l-value
//CHECK:       ^ left of '=' is not an l-value.
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

// A member of a read-only pointee is a read-only memory location, not just an r-value, so it can
// be passed to a `__ref_readonly` parameter.
void readThroughReadPtr(Ptr<Item, Access.Read> p)
{
    outputBuffer[0] = p->count;
    outputBuffer[1] = uint(p->value);
    readOnlyRef(p->count);
    readOnlyRef(p[0].count);
}

void writeThroughReadWritePtr(Ptr<Item, Access.ReadWrite> p)
{
    p->count = 1;
    p->count += 1;
    bump(p->count);
    InterlockedAdd(p->count, 1u);
    p->reset();
    p[0].count = 1;
}

void writeThroughDefaultPtr(Ptr<Item> p)
{
    p->count = 1;
}

[numthreads(1, 1, 1)]
void main()
{
}

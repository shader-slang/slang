// A write through a pointer whose access qualifier is a generic parameter cannot be proven
// to be allowed, so it warns for now; reading through it does not (shader-slang/slang#14462).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0): -target spirv -entry main -stage compute

struct Item
{
    uint count;
    int signedCount;

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

RWStructuredBuffer<uint> outputBuffer;

void writeThroughGenericAccessPtr<Access access>(Ptr<Item, access> p)
{
    outputBuffer[0] = p->count;
    outputBuffer[1] = p[0].count;
    p->count = 1;
/*CHECK:
       ^^^^^ writing through a pointer whose access qualifier is not known
       ^^^^^ the access qualifier of the pointer is a generic parameter
*/
    p->count += 1;
/*CHECK:
       ^^^^^ writing through a pointer whose access qualifier is not known
       ^^^^^ the access qualifier of the pointer is a generic parameter
*/
    bump(p->count);
//CHECK:    ^^^^^ writing through a pointer whose access qualifier is not known
//CHECK:    ^^^^^ the access qualifier of the pointer is a generic parameter
    InterlockedAdd(p->count, 1u);
//CHECK:              ^^^^^ writing through a pointer whose access qualifier is not known
//CHECK:              ^^^^^ the access qualifier of the pointer is a generic parameter
    p[0].count = 1;
/*CHECK:
         ^^^^^ writing through a pointer whose access qualifier is not known
         ^^^^^ the access qualifier of the pointer is a generic parameter
*/
    p->reset();
/*CHECK:
    ^ writing through a pointer whose access qualifier is not known
    ^ the access qualifier of the pointer is a generic parameter
*/
    // The `int` field is converted for the `inout uint` parameter and written back after the call.
    bump(p->signedCount);
//CHECK:    ^^^^^^^^^^^ writing through a pointer whose access qualifier is not known
//CHECK:    ^^^^^^^^^^^ the access qualifier of the pointer is a generic parameter
}

struct Base
{
    uint baseCount;
}

struct Derived : Base
//CHECK:         ^^^^ support for inheritance is unstable
{
}

// Member lookup reaches an inherited field through a cast of the dereference to `Base`.
void writeInheritedFieldThroughGenericAccessPtr<Access access>(Ptr<Derived, access> p)
{
    p->baseCount = 1;
/*CHECK:
       ^^^^^^^^^ writing through a pointer whose access qualifier is not known
       ^^^^^^^^^ the access qualifier of the pointer is a generic parameter
*/
}

[numthreads(1, 1, 1)]
void main()
{
}

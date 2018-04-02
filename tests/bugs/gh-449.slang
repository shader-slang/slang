// gh-449.slang
//TEST:SIMPLE:

// Issue when dealing with binary operations that
// mix scalars with vectors that have a different
// element type.

struct S { int dummy; };

void foo(S s);

void main()
{
    // This works fine right now. The `uint` gets converted to a
    // `float`, and then we do the addition.
    float2 a = float2(1, 2);
    uint   b = 3;
    foo(a + b);

    // This used to get confused, with the `f` getting converted
    // to a `uint` before the addition.
    uint2 u = uint2(1, 2);
    float f = 3.0;
    foo(u + f);
}

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry main -stage fragment

// Two never-written locals meet at the end of an `if`, and the function also
// calls a core-module function that is inlined early (`bool((E)e)`). The
// optimizations that run after that early inlining must keep the merge's two
// undefined values apart, so each variable still gets its own "may be
// uninitialized on some paths" warning instead of a single definite warning.

enum E
{
    A,
    B
}

cbuffer C
{
    bool b;
    int e;
};

float4 f()
{
    float4 v;
    if (b)
    {
        float4 w;
        v = w;
    }
    if (bool((E)e))
        return 1;
    return v;
/*CHK:
    ^^^^^^ possible use of uninitialized variable
    ^^^^^^ variable 'v' may be uninitialized on some paths; it is only conditionally assigned
    ^^^^^^ possible use of uninitialized variable
    ^^^^^^ variable 'w' may be uninitialized on some paths; it is only conditionally assigned
*/
}

[shader("fragment")]
float4 main(float2 uv: TEXCOORD) : SV_Target
{
    return f();
}

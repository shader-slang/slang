//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute

// WGSL defines the bitwise operators `|`, `&` and `^` for integer scalars and vectors only
// (https://www.w3.org/TR/WGSL/#bit-expr). `|` and `&` are also defined for `bool` and `vecN<bool>`
// as logical operators (https://www.w3.org/TR/WGSL/#logical-expr), but `^` is not, so `bool ^ bool`
// must be emitted as the equivalent `!=`, which WGSL defines for `bool` and `vecN<bool>`
// (https://www.w3.org/TR/WGSL/#comparison-expr). `^` on integers must stay `^`.
//
// The inputs are read from a buffer so that the operations are not folded away before emission.
// The `TINT` check compiles the emitted WGSL to SPIR-V with Tint, which rejects `bool ^ bool`.

RWStructuredBuffer<uint> o;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    bool a = o[0] > 1u;
    bool b = o[1] > 1u;
    bool d = o[2] > 1u;

    // Scalar `bool ^ bool`.
    //WGSL: if((a_0 != (b_0)))
    o[5] = (a ^ b) ? 1u : 0u;

    // Vector `bool ^ bool`.
    //WGSL: if(((vec2<bool>(a_0, b_0) != (vec2<bool>(b_0, a_0)))).x)
    bool2 v = bool2(a, b) ^ bool2(b, a);
    o[6] = v.x ? 1u : 0u;

    // Compound assignment.
    //WGSL: if((a_0 != (d_0)))
    bool c = a;
    c ^= d;
    o[7] = c ? 1u : 0u;

    // Integer `^` is unchanged.
    //WGSL: = (o_0[i32(3)] ^ (o_0[i32(4)]));
    o[8] = o[3] ^ o[4];
}

// The WGSL is valid: Tint translates it to SPIR-V.
// TINT: OpEntryPoint

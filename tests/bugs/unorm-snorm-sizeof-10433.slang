// `sizeof`/`alignof` on a value whose type carries a `unorm`/`snorm` modifier
// must report the base type's size. The modifier only selects a texture image
// format at emit and never changes storage, so `sizeof(unorm float4)` is
// `sizeof(float4)`.
//
// The modifier reaches ordinary values without the user ever spelling it:
// loading from a `RWTexture2D<unorm float4>` yields a `unorm float4`, so
// `sizeof(value)` used to be rejected with "invalid sizeof argument" (E30099)
// on a value that has a perfectly well-defined size.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

typealias U4 = unorm float4;

// `sizeof` has to fold during semantic checking, not only by the time IR layout runs:
// generic argument unification compares the folded values, so an unfolded
// `sizeof(U4)` makes `G<sizeof(U4)>` a distinct type from `G<16>` and the call below
// fails with E30019. Emitted-code checks alone do not catch that -- the IR layout path
// supplies the right constant afterwards either way.
struct G<let N : int> { int get() { return N; } }
void takesG16(G<16> g) {}

RWTexture2D<unorm float4> utex;
RWTexture2D<snorm float2> stex;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // `var` keeps the modifier on the value's type; writing the modifier out
    // by hand would not reproduce the original report.
    var u4 = utex[uint2(0, 0)];
    var s2 = stex[uint2(0, 0)];

    outputBuffer[0] = sizeof(u4);
    outputBuffer[1] = alignof(u4);
    outputBuffer[2] = sizeof(s2);
    outputBuffer[3] = alignof(s2);

    // The assertion here is that this compiles: if `sizeof(U4)` does not fold during
    // checking, the argument is `G<sizeof(unorm vector<float,4>)>` and this call is
    // rejected with E30019.
    G<sizeof(U4)> g;
    takesG16(g);

    // Every value folds at compile time, so the emitted code contains the
    // constants rather than any size computation.
    //CHECK: outputBuffer{{.*}}[int(0)] = int(16);
    //CHECK: outputBuffer{{.*}}[int(1)] = int(4);
    //CHECK: outputBuffer{{.*}}[int(2)] = int(8);
    //CHECK: outputBuffer{{.*}}[int(3)] = int(4);
}

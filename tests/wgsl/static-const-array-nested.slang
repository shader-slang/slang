//TEST:SIMPLE(filecheck=WGSL): -stage vertex -entry vertexMain -target wgsl
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -stage vertex -entry vertexMain -target wgsl-spirv-asm

// Regression test for issue #6747 — nested aggregates.
//
// A *nested* `static const` array (here a 2D array) lowers to nested `MakeArray` insts, each of
// which is a module-scope array constant. The inner constituents are folded inline
// (shouldFoldInstIntoUseSites), so the outermost runtime-indexed array is a single module-scope
// `const` with a self-contained inline initializer.

static const int grid[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};

// Exactly one module-scope `const`, whose initializer inlines the inner arrays as constructor
// expressions (NOT as references to separately-declared values), and no private variable.
// WGSL-NOT: var<private>
// WGSL: {{^}}const grid{{.*}} : array<array<i32, i32(3)>, i32(2)> = array<array<i32, i32(3)>, i32(2)>( array<i32, i32(3)>( i32(1), i32(2), i32(3) ), array<i32, i32(3)>( i32(4), i32(5), i32(6) ) );

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;
    // Runtime index of the outer array — valid on a WGSL `const` array value.
    // WGSL: grid{{.*}}[vertexID
    // SPV: OpEntryPoint
    output.position = float4(float(grid[vertexID][1]), 0.0f, 0.0f, 1.0f);
    return output;
}

//TEST:SIMPLE(filecheck=WGSL): -stage vertex -entry vertexMain -target wgsl
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -stage vertex -entry vertexMain -target wgsl-spirv-asm

// Regression test for issue #6747 — nested aggregates.
//
// A *nested* `static const` array (here a 2D array) lowers to nested `MakeArray` insts, each of
// which is a module-scope array constant. Naively emitting each as its own module-scope decl and
// converting all of them to `var<private>` would make the outer `var<private>` initializer
// reference the inner `var<private>`s — which is illegal WGSL (a `var<private>` initializer must
// be a const-expression and cannot read another `var`). The fix folds the inner constituents
// inline (shouldFoldInstIntoUseSites) so only the outermost runtime-indexed array becomes a
// `var<private>`, with a self-contained inline initializer.

static const int grid[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};

// Exactly one module-scope private variable, whose initializer inlines the inner arrays as
// constructor expressions (NOT as references to separately-declared variables).
// WGSL: var<private> grid{{.*}} : array<array<i32, i32(3)>, i32(2)> = array<array<i32, i32(3)>, i32(2)>( array<i32, i32(3)>( i32(1), i32(2), i32(3) ), array<i32, i32(3)>( i32(4), i32(5), i32(6) ) );

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;
    // Runtime index of the outer array — the construct that requires `grid` to be `var<private>`.
    // WGSL: grid{{.*}}[vertexID
    // SPV: OpEntryPoint
    output.position = float4(float(grid[vertexID][1]), 0.0f, 0.0f, 1.0f);
    return output;
}

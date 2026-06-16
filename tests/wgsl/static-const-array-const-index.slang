//TEST:SIMPLE(filecheck=WGSL): -stage vertex -entry vertexMain -target wgsl

// Regression guard for issue #6747.
//
// Companion to static-const-array-indexing.slang. That test verifies a runtime-indexed
// `static const` array is emitted as `var<private>`. This test verifies the opposite case
// is not broken by that change: an element of a `static const` array, read at a *constant*
// index, must constant-fold (the peephole pass folds `GetElement(MakeArray, constIndex)` to
// the element) and therefore must NOT leave behind a module-scope `const` whose initializer
// indexes a runtime variable, which would be illegal WGSL (a `const` initializer must be a
// const-expression and cannot read a `var`). The array, being only constant-indexed, folds
// away entirely.

static const float weights[3] = {0.25f, 0.5f, 0.25f};

// `weights[1]` is a constant index, so it folds to the literal `0.5` and `weights` is
// dead-code-eliminated; it must not survive as a variable that `midWeight` indexes.
static const float midWeight = weights[1];

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;
    output.position = float4(float(vertexID) * midWeight, 0.0f, 0.0f, 1.0f);
    return output;
}

// The constant-indexed array must fold away; no identifier derived from it survives.
// (If it instead became a `var<private>` referenced by `midWeight`, "weights" would appear.)
// WGSL-NOT: weights
// ...and the dependent constant must survive as the folded element value (weights[1] == 0.5).
// WGSL: 0.5

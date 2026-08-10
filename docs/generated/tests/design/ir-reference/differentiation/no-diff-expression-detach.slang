//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#detachderivative
//META: doc_section_digest=6a606134e98de20cf82c1e5baedde0a38a9f404df32219b204c2dad2b81869b7
//META: purpose=no_diff applied to an expression wraps the materialized load in detachDerivative, so a local-array element read is detached the same way a resource read is.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second documented spelling and the specific materialization
// behaviour it relies on. `no_diff(tbl[i])` indexes a mutable local array, so
// the element read lowers to an address computation followed by a `load`;
// CHECK binds that load and then requires `detachDerivative` to take the loaded
// value as its operand. Wrapping the load rather than the address is what makes
// `no_diff` on local-array indexing behave the same as on a resource read.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform int idx;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float f(float x, no_diff int i)
{
    float tbl[4];
    tbl[0] = 1.0;
    tbl[1] = 2.0;
    tbl[2] = 3.0;
    tbl[3] = x;
    return x * no_diff(tbl[i]);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, 1.0), idx).d;
}

// CHECK: func %f
// CHECK: let [[LD:%[A-Za-z0-9_]+]] : Float = load(
// CHECK-NEXT: Float = detachDerivative([[LD]])

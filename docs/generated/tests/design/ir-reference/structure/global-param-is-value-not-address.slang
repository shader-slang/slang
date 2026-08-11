//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=100b91a908217e68795cbc9dac7393e0325718d4c3df831fe3616ee38716a45a
//META: purpose=Unlike global_var, a global_param is the value itself rather than its address, so its result type is the declared type and not a Ptr to it.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Contrasts the two module-scope opcodes in a single module so the difference
// is not an artefact of two separate compiles. `gIn` and `gState` have the same
// declared type `int`; the CHECK-DAGs require `gIn` to be typed `Int` under
// `global_param` while `gState` is typed `Ptr(Int)` under `global_var`. A
// regression that gave `global_param` a pointer result would fail the first.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int gIn;
uniform RWStructuredBuffer<int> buf;

static int gState = 0;

[numthreads(1, 1, 1)]
void main()
{
    gState = gState + gIn;
    buf[0] = gState;
}

// CHECK-DAG: let  %gIn	: Int	= global_param
// CHECK-DAG: global_var %gState	: Ptr(Int)

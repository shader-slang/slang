//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#other
//META: doc_section_digest=0d0762e4e59612e4c842da9adb0ce25510dfd2380e8ab427638f4c6b2bd64f9a
//META: purpose=A member function declared on a struct lowers to a func with a method decoration attached.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `method` is the nullary marker distinguishing a
// member-function lowering from a free function. The CHECK pins the bare
// decoration followed by the qualified `nameHint("S.run")`, which is what
// identifies the decorated function as the struct member rather than any
// other function in the module.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct S
{
    float run(float x) { return x + 1.0; }
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    S s;
    buf[0] = s.run(float(tid.x));
}

// CHECK: [method]
// CHECK: [nameHint("S.run")]

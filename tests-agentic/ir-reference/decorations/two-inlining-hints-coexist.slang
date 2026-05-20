//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#inlining-and-optimization
//META: doc_section_digest=99f72cda245d99360168c24acf24a82fd577e139a911dfa073addcf5493b365a
//META: purpose=Two helper functions with opposing inlining hints ([ForceInline] vs [noinline]) each surface their own decoration, independent of the other.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists ForceInline and noInline as two distinct opcodes
// in the same category. Confirm they coexist on different
// functions in the same module without interference.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[ForceInline]
int hot(int x) { return x + 1; }

[noinline]
int cold(int x) { return x * 2; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = hot(int(tid.x)) + cold(int(tid.x));
}

// CHECK-DAG: [ForceInline]
// CHECK-DAG: [noInline]

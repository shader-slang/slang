//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#inlining-and-optimization
//META: doc_section_digest=99f72cda245d99360168c24acf24a82fd577e139a911dfa073addcf5493b365a
//META: purpose=A function decorated with [__NoSideEffect] carries a noSideEffect decoration with no operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `noSideEffect` ("Marks a callee as side-effect
// free") with AST origin "[__NoSideEffect] attribute".

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[__NoSideEffect]
int helperPure(int x) { return x ^ 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helperPure(int(tid.x));
}

// CHECK: [noSideEffect]
// CHECK: [nameHint("helperPure")]
// CHECK: func %helperPure

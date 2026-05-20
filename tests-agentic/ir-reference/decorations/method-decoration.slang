//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#other
//META: doc_section_digest=99f72cda245d99360168c24acf24a82fd577e139a911dfa073addcf5493b365a
//META: purpose=A member function declared on a struct lowers to a func with a method decoration attached.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc records `method` with no operands and AST origin
// "Member-function declaration". The dotted name `S.run` mangles
// into the dump as `Sx5Frun` (underscore → x5F), so the test
// anchors on the [method] decoration line.

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

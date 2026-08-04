//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#specialization-and-existentials
//META: doc_section_digest=561db21a4ad8f3742180c521eb399a01338495f47b8085f196d3a2228c3211cf
//META: purpose=Calling a generic function with a concrete type argument lowers to a `specialize` IR opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `specialize` row ("Applies generic arguments;
// hoistable"). Calling the generic `pickFirst<int>` produces a
// `specialize(%generic, ...)` in the IR dump that supplies the concrete
// type argument to the generic.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pickFirst(a, a + 1);
}

// IR-LABEL: func %main
// IR: specialize({{.*}})

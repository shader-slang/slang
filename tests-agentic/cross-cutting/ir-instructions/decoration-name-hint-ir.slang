//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=1b5a75ff3886dcb59a3adffbb216b97498738abd9bb7db936eee68e3ef99f8e8
//META: purpose=Each user-named declaration in source carries a `nameHint` decoration in the IR so the linker / debug output can render it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A user-named function and a user-named local variable both carry
// `nameHint("...")` decorations in the IR. This is the doc's
// "Carries an identifier name for debug / link output" claim.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int v) { return v + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int myLocal = helper(a);
    buf[0] = myLocal;
}

// IR: [nameHint("helper")]
// IR: [nameHint("main")]
// IR: [nameHint("myLocal")]

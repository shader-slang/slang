//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=fa1a8caf9cb2dd874328b2b826017621c7f1df8e509df2e908e25b520003a421
//META: purpose=Expansion: Array composes with the narrow Int8 element type, surfacing Array(Int8, N : Int).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Array row admits any IRType element. An `int8_t[4]`
// global_param surfaces `Array(Int8, 4 : Int)` in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t v[4];
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(v[0]) + int(v[3]);
}

// IR: let {{.*}}%v{{.*}}: Array(Int8, 4 : Int){{.*}}= global_param

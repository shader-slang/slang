//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=Expansion: Vec composes with UInt8 to form Vec(UInt8, 4 : Int) as a global_param.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Vec row pairs any scalar with an element count.
// `vector<uint8_t, 4>` exercises the narrow unsigned scalar in a
// vector and surfaces `Vec(UInt8, 4 : Int)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform vector<uint8_t, 4> v;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(v.x) + uint(v.w);
}

// IR: let {{.*}}%v{{.*}}: Vec(UInt8, 4 : Int){{.*}}= global_param

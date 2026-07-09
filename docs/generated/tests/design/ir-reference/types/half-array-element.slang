//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=a39cfe25465518ad38d31423ca65973ffe6e67b11946248f9af71a03f7cee6df
//META: purpose=Expansion: Array of Half elements surfaces Array(Half, N : Int) in the IR dump.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Array row admits any element type. A `half[4]`
// global_param surfaces `Array(Half, 4 : Int)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half v[4];
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float(v[0]) + float(v[3]);
}

// IR: let {{.*}}%v{{.*}}: Array(Half, 4 : Int){{.*}}= global_param

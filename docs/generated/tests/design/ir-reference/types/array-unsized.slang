//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#array-vs-unsizedarray
//META: doc_section_digest=8d861464d52d5973316dc6cf7fb88f9d4e6d9a20f1c42ebaf22ceb34ed960ce4
//META: purpose=An unsized array (runtime extent) is typed UnsizedArray(elementType) in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An unsized array (runtime extent) is typed UnsizedArray(elementType)
// in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float tail[];
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = tail[0];
}

// IR: let {{.*}}%tail{{.*}}: UnsizedArray(Float){{.*}}= global_param

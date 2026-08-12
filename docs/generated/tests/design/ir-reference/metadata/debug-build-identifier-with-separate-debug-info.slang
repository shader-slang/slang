//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=50809951cc8677fde31d92b4c0fe9b213a4fc77720df38216d197ee7561bc6d1
//META: purpose=DebugBuildIdentifier records the build identifier of the compilation together with a flags operand when separate debug information is requested.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the module-level build-identifier record. It is only created when
// the compile asks for separate debug information, so that option is passed
// alongside `-g`. The identifier itself is a hash of the source and options
// and therefore matched with a wildcard; the flags operand is pinned because
// it is a stable literal.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g -separate-debug-info

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: DebugBuildIdentifier("{{[0-9a-f]+}}", {{[0-9]+}} : UInt)

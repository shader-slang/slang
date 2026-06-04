//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=21e167102f1daeca19e02f308c112de6dba84ede8a917f058d59ae80138afd50
//META: purpose=Module-scope shader-parameter declarations surface as `global_param` lines in the IR preamble, with the resource type (`RWStructuredBuffer(...)`, `StructuredBuffer(...)`, `ByteAddressBuffer`, `RWByteAddressBuffer`) carried in the result type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Shader IO row says `global_param` is the IR opcode for a
// module-scope shader parameter (`uniform`, push-constant, descriptor
// binding). One `global_param` is emitted per such declaration.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> rwbuf;
uniform StructuredBuffer<int> sbuf;
uniform ByteAddressBuffer babuf;
uniform RWByteAddressBuffer rwbabuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    rwbuf[idx] = sbuf.Load(idx) + (int)babuf.Load(idx * 4);
    rwbabuf.Store(idx * 4, (uint)idx);
}

// IR: %rwbuf{{.*}}: RWStructuredBuffer(Int{{.*}}= global_param
// IR: %sbuf{{.*}}: StructuredBuffer(Int{{.*}}= global_param
// IR: %babuf{{.*}}: ByteAddressBuffer{{.*}}= global_param
// IR: %rwbabuf{{.*}}: RWByteAddressBuffer{{.*}}= global_param

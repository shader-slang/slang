//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=212a9e3fd4ca324be91b034872f2b7a9afdb1fe8ac5acfc9f068664d69c3e326
//META: purpose=Module-scope shader-parameter declarations surface as `global_param` lines in the IR preamble, with the resource type (`RWStructuredBuffer(...)`, `StructuredBuffer(...)`, `ByteAddressBuffer`, `RWByteAddressBuffer`) carried in the result type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that one module-scope shader parameter yields one
// `global_param`, and that the parameter's declared buffer type is
// carried on the result type. Four buffer kinds are declared in one
// shader so the CHECKs can pin each type spelling against its own
// parameter name.

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

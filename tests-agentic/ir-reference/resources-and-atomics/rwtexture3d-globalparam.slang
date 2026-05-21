//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=21e167102f1daeca19e02f308c112de6dba84ede8a917f058d59ae80138afd50
//META: purpose=A module-scope `RWTexture3D<T>` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShape3DType, 0, 0, 0, 1, ...)`, combining 3D shape with the RW-access flag.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RWTexture3D<T>` combines 3D shape with the RW flag.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWTexture3D<float4> rwtex;
uniform RWStructuredBuffer<int> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    rwtex[tid] = float4(0, 0, 0, 0);
    outbuf[0] = 1;
}

// IR: %rwtex{{.*}}: TextureType({{.*}}TextureShape3DType, 0 : Int, 0 : Int, 0 : Int, 1 : Int{{.*}}= global_param

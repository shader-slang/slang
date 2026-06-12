//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#downstream-dxc-fxc
//META: doc_section_digest=924b157419520ebcec2e0e4024679821ed4db2d92a3d90fcd3a21f934950d4d7
//META: purpose=DXC is invoked as the downstream compiler when -target dxil is requested; the artifact contains DXIL bytecode whose textual disassembly contains the DXIL entry-point definition.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `### Downstream DXC / fxc` section: `CodeGenTarget::DXIL`
// rides the same Slang IR path but diverges in `createArtifactFromIR`
// to dispatch DXC. CI's nightly runner has DXC; the produced DXIL
// disassembly contains the canonical `define void @main` (DXIL uses
// LLVM-textual entry definitions). This commit ships the test
// directive; CI executes it.

//TEST:SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_0 -entry main

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tid.x;
}

// DXIL: define void @
// DXIL: main

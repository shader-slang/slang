//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=af2eb87d563403d52d71a7d33122c279b70c033339e51393dbe00f9f5f506a59
//META: purpose=A function selected as a pipeline entry point carries an `entryPoint` decoration in the IR and lowers to a target entry-point declaration.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `EntryPointDecoration`/`IREntryPointDecoration` row
// ("Marks a function as a pipeline entry point"). The `-entry main`
// function carries an `[entryPoint(...)]` decoration in the IR dump; on
// SPIR-V that decoration lowers to `OpEntryPoint GLCompute %main "main"`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 1;
}

// IR: [entryPoint({{.*}}"main"{{.*}})]

// SPV: OpEntryPoint GLCompute %main "main"

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#switch-and-targetswitch
//META: doc_section_digest=bb310095020a0added87bde60cb729ab81078e469d274a935ab7dd9ece947e47
//META: purpose=A switch whose condition is a bool is legalized on targets requiring an integer switch — the scrutinee gains a bool-to-int conversion and each case key is rewritten to the matching integer (true to 1, false to 0) — while targets that accept a bool switch keep `case true:` / `case false:`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bool-switch legalization and, just as importantly, that it is
// selected per target rather than applied everywhere.
//
// The scrutinee is a `uniform bool` so the switch cannot be constant-folded
// away before legalization runs, and the arm results are sunk into an
// RWStructuredBuffer so DCE cannot delete the switch. Each target then gets
// its own CHECK prefix asserting which side of the split it lands on:
//
//   * GLSL / SPIR-V / WGSL require an integer switch, so the CHECKs pin the
//     inserted conversion (`int(...)`, `OpSelect`, `select(i32(0), i32(1), ...)`)
//     together with the rewritten integer case keys. Pinning both `1` and `0`
//     — in that source order, matching the `case true:` / `case false:` order
//     below — is what proves the documented `true` to 1, `false` to 0 mapping
//     rather than some other bool-to-int encoding.
//   * HLSL / Metal / CUDA / C++ accept a bool switch, so `case true:` and
//     `case false:` surviving verbatim proves the pass did NOT rewrite the
//     keys there.
//
// SPIR-V ids are matched with permissive captures because the harness may
// disassemble with friendly names; the OpSwitch case keys are literals in the
// instruction stream, not ids, so those are pinned exactly.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

uniform bool g;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    switch (g)
    {
    case true:
        r = 1;
        break;
    case false:
        r = 2;
        break;
    }
    outputBuffer[0] = r;
}

// Targets requiring an integer switch: the condition is converted and the
// bool case keys become the integers 1 and 0.

// GLSL: switch(int(
// GLSL: case 1:
// GLSL: case 0:

// SPV: OpSelect %{{[A-Za-z0-9_]+}}
// SPV: OpSwitch %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} 1 %{{[A-Za-z0-9_]+}} 0 %{{[A-Za-z0-9_]+}}

// WGSL: switch(select(i32(0), i32(1),
// WGSL: case i32(1):
// WGSL: case i32(0):

// Targets that accept a bool switch: the keys are left as booleans.

// HLSL: case true:
// HLSL: case false:

// METAL: case true:
// METAL: case false:

// CUDA: case true:
// CUDA: case false:

// CPP: case true:
// CPP: case false:

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#moving-and-deleting-control-flow-instructions
//META: doc_section_digest=1a35f3e6e880b02476bb7fbe3db3c3bfa0fe959a20d8b9005e5bd64d9b003102
//META: purpose=Printf survives dead-code elimination at -O3 because it reports as possibly-side-effecting, while a genuinely unused arithmetic value next to it is removed.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the practical consequence of Printf having no case in the
// side-effect classifier and therefore landing in the conservative arm:
// nothing consumes the print, so a purely value-based DCE would delete it.
// The shader is compiled at -O3 with an unused arithmetic expression sitting
// right next to the call; each target's CHECK asserts the print is still in
// the emitted text while the CHECK-NOT asserts the unused multiply is gone,
// so the test distinguishes "DCE did not run" from "DCE ran and kept the
// print".
//
// The claim is target-dependent: each back-end lowers the print to its own
// facility (`printf` in HLSL, `debugPrintfEXT` in GLSL, a DebugPrintf
// `OpExtInst` in SPIR-V, `os_log_default.log` in Metal), so every text-emit
// shader target that can express a print gets its own directive. WGSL has no
// print facility at all -- it rejects the entry point with E36107 -- so it is
// recorded in the bundle README as an `unsupported-on-target` Untested claim
// rather than given a weakened directive here.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -O3 -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -O3 -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -O3 -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -O3 -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int unusedValue = n * 3 + 7;
    printf("kept %d\n", n);
    buf[0] = n;
}

// HLSL-LABEL: void main()
// HLSL-NOT: {{[*]}} int(3)
// HLSL: printf("kept %d\n"

// GLSL-LABEL: void main()
// GLSL-NOT: {{[*]}} 3 {{[+]}} 7
// GLSL: debugPrintfEXT("kept %d\n"

// SPIRV-LABEL: %main = OpFunction
// SPIRV-NOT: OpIMul
// SPIRV: OpExtInst %void

// METAL-LABEL: main_0(
// METAL-NOT: {{[*]}} int(3)
// METAL: os_log_default.log("kept %d\n"

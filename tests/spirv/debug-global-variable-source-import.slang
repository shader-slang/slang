//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly
//TEST:SIMPLE(filecheck=DUP):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly

// Test that DebugGlobalVariable and DebugCompilationUnit reference the correct source file
// (this main file) when using import syntax instead of #include.

import shader_utils_module;

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
}

[shader("fragment")]
float4 main(VertexOutput input) : SV_Target
{
    float2 uv = input.texCoord;
    float distanceFromCenter = calculateDistanceFromCenterModule(uv);
    float radialFade = createRadialFadeModule(uv, 0.7);
    float4 color = float4(
        uv.x * (1.0 - distanceFromCenter * 0.5),
        uv.y * radialFade,
        uv.x * uv.y,
        1.0
    );
    return color;
}

// Verify that DebugSource for the main file is defined
// CHECK-DAG: [[MAIN_FILE:%[0-9]+]] = OpString "{{.*}}debug-global-variable-source-import.slang"
// CHECK-DAG: [[SOURCE_MAIN:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugSource [[MAIN_FILE]] %{{[0-9]+}}

// Verify that DebugCompilationUnit references the main file source, not the imported module,
// and capture its result ID for scope verification below.
// CHECK-DAG: [[CU_MAIN:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugCompilationUnit %uint_{{[0-9]+}} %uint_{{[0-9]+}} [[SOURCE_MAIN]] %uint_{{[0-9]+}}

// Verify that DebugGlobalVariable references the main file source AND has the main file's
// DebugCompilationUnit as its parent scope (not the imported module's compilation unit).
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugGlobalVariable {{%[0-9]+}} {{%[0-9]+}} [[SOURCE_MAIN]] %uint_0 %uint_0 [[CU_MAIN]]

// Regression guard for shader-slang/slang#11982: an imported module used to produce a
// duplicate, orphaned DebugSource record (the two IR-lowering producers spelled its path
// differently, defeating hoistable dedup). There are exactly two physical source files here
// (this entry-point file and the imported module), so there must be exactly two DebugSource
// records. This runs as its own FileCheck pass (DUP) so a lone count assertion scans the whole
// module without ordering hazards against the CHECK-DAG block above. The pattern matches on the
// opcode/mnemonic only (no operands) so a record with a different operand shape cannot escape the
// count; it still does not match the embedded -g2 source text of these directive lines, so it is
// immune to the embedded-source self-match trap.
// DUP-COUNT-2: OpExtInst %void %{{[0-9]+}} DebugSource
// DUP-NOT: OpExtInst %void %{{[0-9]+}} DebugSource

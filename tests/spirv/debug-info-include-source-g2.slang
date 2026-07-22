//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -debug-info-include-source -emit-spirv-directly -preserve-embedded-source
//TEST:SIMPLE(filecheck=CHECK):-target spirv -g3 -debug-info-include-source -emit-spirv-directly -preserve-embedded-source

// At `-g2` (Standard) and `-g3` (Maximal) the source is already embedded via the NonSemantic
// `DebugSource` path, so `-debug-info-include-source` is a no-op at both levels: the NonSemantic
// extension is still used for the source and the core `OpSource` stays a bare language/version
// instruction (no File/Source operand). SPIR-V emit treats Standard and Maximal identically, so
// the same checks below apply to both directives above.

RWStructuredBuffer<float> outputBuffer;

// SOURCE_MARKER_G2_d4e5f6
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 1.0f;
}

// Standard debug info still uses the NonSemantic extension (source rides the DebugSource path).
// Match the real instruction form (`%<id> = OpExtInstImport "..."`); the `= OpExtInstImport`
// prefix avoids matching an embedded copy of this comment inside the DebugSource string.
// CHECK-DAG: = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"

// The source is still embedded via the NonSemantic DebugSource path: the OpString feeding
// DebugSource carries the source text, so this shader's unique marker appears in the module. This
// guards against the option accidentally disabling g2 source embedding. Matching the marker with
// its `// ` source-comment prefix (and nothing before it) targets the real embedded source line,
// not the embedded copy of this CHECK directive (which renders as `// CHECK-DAG: ...`).
// CHECK-DAG: {{^}}// SOURCE_MARKER_G2_d4e5f6

// The DebugSource instruction is present (source rides it at g2, not the core OpSource).
// CHECK-DAG: {{%[0-9]+}} = OpExtInst %void {{%[0-9]+}} DebugSource

// The core OpSource remains bare (language + version only); the option does not add File/Source.
// CHECK-DAG: OpSource {{Slang|Unknown}} 1{{$}}

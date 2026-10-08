//TEST:SIMPLE(filecheck=CHECK):-target spirv -g1 -debug-info-include-source -emit-spirv-directly -preserve-embedded-source

// At -g1 (Minimal), -debug-info-include-source embeds the shader source via the core SPIR-V
// OpSource File+Source operands, without pulling in the NonSemantic debug extension.

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 1.0f;
}

// These leading CHECK-NOTs guard the output from the start up to the first positive CHECK below,
// and the trailing CHECK-NOTs guard from the last positive CHECK to EOF (a plain trailing-only
// CHECK-NOT would miss an instruction that appears *before* the positive match). The only interval
// not covered lies between the two positive CHECKs, which is inside the single multi-line OpSource
// string literal — no SPIR-V instruction can appear there — so absence holds across the module.
//
// (1) When source is embedded, the file-carrying OpSource replaces the bare language/version one
// (they are mutually exclusive), so a source extractor does not see a spurious file-less record.
// `{{^}}...{{$}}` matches only a real bare instruction at column 0, not the `// `-prefixed embedded
// copy of this directive, and `{{$}}` (end anchor) distinguishes it from the file-carrying form.
// (2) The source must NOT ride the NonSemantic debug extension at g1; the `= OpExtInstImport`
// prefix matches the real instruction, not an embedded copy of this comment.
// CHECK-NOT: {{^}}OpSource {{Slang|Unknown}} 1{{$}}
// CHECK-NOT: = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"

// The core OpSource carries the source language, version, a File <id> operand, and the embedded
// source text as a literal string. The disassembler renders the multi-line source with real
// newlines, so the opening quote and the first source line share one physical line while the
// string continues below; match only that opening (no closing quote). The instruction-start form
// `OpSource Slang 1 %<id> "` cannot be satisfied by an embedded copy of a CHECK directive, so it
// does not self-match; `%{{[0-9]+}}` is the File <id>.
// CHECK: OpSource {{Slang|Unknown}} 1 %{{[0-9]+}} "//TEST:SIMPLE

// The full source body is embedded: this shader's own body line appears verbatim on its own line
// inside the OpSource string literal. Matching it with its leading source indentation (nothing but
// spaces before it) targets the real embedded line, not the embedded copy of this CHECK directive.
// CHECK: {{^}}    outputBuffer[0] = 1.0f;

// Re-assert both absences from the last positive match to EOF.
// CHECK-NOT: {{^}}OpSource {{Slang|Unknown}} 1{{$}}
// CHECK-NOT: = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"

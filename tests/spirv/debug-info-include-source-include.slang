//TEST:SIMPLE(filecheck=CHECK):-target spirv -g1 -debug-info-include-source -emit-spirv-directly -preserve-embedded-source

// With an #include, the module has multiple IRDebugSource insts. At -g1 the option embeds every
// source file via its own core OpSource (File + Source), matching the -g2/-g3 NonSemantic
// DebugSource path (one record per file, included files too). Both this main file's marker and
// the included file's marker must therefore appear in the embedded source.

#include "debug-info-include-source-inc-helper.slang"

RWStructuredBuffer<float> outputBuffer;

// MAIN_MARKER
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = debugSourceIncludeHelper(1.0f);
}

// Each source file gets its own OpSource with a File <id> and embedded text; two are expected
// (this file + the included helper). `%{{[0-9]+}}` is the File id.
// CHECK-DAG: OpSource {{Slang|Unknown}} 1 %{{[0-9]+}} "//TEST:SIMPLE
// CHECK-DAG: OpSource {{Slang|Unknown}} 1 %{{[0-9]+}} "// INCLUDED_HELPER_MARKER

// The main file's marker also appears on its own line within its embedded source (matched with the
// `// ` source-comment prefix so an embedded copy of a CHECK directive cannot self-match). The
// included file's marker is its first line, already covered by the OpSource check above.
// CHECK-DAG: {{^}}// MAIN_MARKER

// No NonSemantic extension for the source at g1.
// CHECK-NOT: = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"

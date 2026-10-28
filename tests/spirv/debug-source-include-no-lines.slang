//TEST:SIMPLE(filecheck=MAINHDR):-target spirv -entry main -stage compute -g2 -emit-spirv-directly
//TEST:SIMPLE(filecheck=MODHDR):-target spirv -entry main -stage compute -g2 -emit-spirv-directly
//TEST:SIMPLE(filecheck=G1HDR):-target spirv -entry main -stage compute -g1 -emit-spirv-directly

// Regression test for the file list of shader-slang/slang#7203 under the linker rule of
// shader-slang/slang#13982. This file and the module it imports each `#include` a header that
// defines only a macro, so neither header has a line-table entry and no debug record of the
// linked code refers to it. Both modules contribute code to `main`, so both headers must still
// get a DebugSource (and, at -g1, an OpString): the linker reaches them only through the
// compilation units of their modules, which name every `#include`d file of the translation unit.

#include "debug-source-include-no-lines-main-header.slang"
import debug_source_include_no_lines_module;

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = includeNoLinesModuleValue(float(tid.x)) * MAIN_HEADER_SCALE_7203;
}

// MAINHDR: [[FILE:%[0-9]+]] = OpString "{{.*}}debug-source-include-no-lines-main-header.slang"
// MAINHDR: OpExtInst %void %{{[0-9]+}} DebugSource [[FILE]]

// MODHDR: [[FILE:%[0-9]+]] = OpString "{{.*}}debug-source-include-no-lines-module-header.slang"
// MODHDR: OpExtInst %void %{{[0-9]+}} DebugSource [[FILE]]

// G1HDR-DAG: OpString "{{.*}}debug-source-include-no-lines-main-header.slang"
// G1HDR-DAG: OpString "{{.*}}debug-source-include-no-lines-module-header.slang"

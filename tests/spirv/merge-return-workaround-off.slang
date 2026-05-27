// This OFF-only test is excluded from the default full category.
// Run with: slang-test -category spirv-opt-off tests/spirv/merge-return-workaround-off.slang
//TEST_CATEGORY(spirv-opt-off):
//TEST:SIMPLE(filecheck=CHECK_OFF_O1): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=CHECK_OFF_O3): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O3

// CHECK_OFF_O1: OpEntryPoint GLCompute
// CHECK_OFF_O1-DAG-COUNT-2: = OpFunction {{.*}}
// CHECK_OFF_O1-DAG-COUNT-1: OpFunctionCall
// CHECK_OFF_O1-DAG-COUNT-3: OpReturnValue

// CHECK_OFF_O3: OpEntryPoint GLCompute
// CHECK_OFF_O3-DAG-COUNT-2: = OpFunction {{.*}}
// CHECK_OFF_O3-DAG-COUNT-1: OpFunctionCall
// CHECK_OFF_O3-DAG-COUNT-3: OpReturnValue

#include "merge-return-workaround.slang"

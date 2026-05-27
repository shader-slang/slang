// This OFF-only test is excluded from the default full category.
// Run with: slang-test -category spirv-opt-off tests/spirv/merge-return-workaround-off.slang
//TEST_CATEGORY(spirv-opt-off):
//TEST:SIMPLE(filecheck=CHECK_OFF_O1): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=CHECK_OFF_O3): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O3
//TEST:SIMPLE(filecheck=CHECK_OFF_O1): -target spirv-assembly -emit-spirv-directly -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=CHECK_OFF_O3): -target spirv-assembly -emit-spirv-directly -entry computeMain -stage compute -O3

// CHECK_OFF_O1: OpEntryPoint GLCompute
// CHECK_OFF_O1-DAG: = OpFunction {{.*}}
// CHECK_OFF_O1-DAG: = OpFunction {{.*}}
// CHECK_OFF_O1-DAG: OpFunctionCall
// CHECK_OFF_O1-DAG: OpReturnValue
// CHECK_OFF_O1-DAG: OpReturnValue
// CHECK_OFF_O1-DAG: OpReturnValue

// CHECK_OFF_O3: OpEntryPoint GLCompute
// CHECK_OFF_O3-DAG: = OpFunction {{.*}}
// CHECK_OFF_O3-DAG: = OpFunction {{.*}}
// CHECK_OFF_O3-DAG: OpFunctionCall
// CHECK_OFF_O3-DAG: OpReturnValue
// CHECK_OFF_O3-DAG: OpReturnValue
// CHECK_OFF_O3-DAG: OpReturnValue

#include "merge-return-workaround.slang"

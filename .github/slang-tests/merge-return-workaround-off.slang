//TEST:SIMPLE(filecheck=CHECK_OFF_O1): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=CHECK_OFF_O3): -target spirv-assembly -emit-spirv-via-glsl -entry computeMain -stage compute -O3

// CHECK_OFF_O1: OpEntryPoint GLCompute
// CHECK_OFF_O1: = OpFunction
// CHECK_OFF_O1: OpFunctionCall
// CHECK_OFF_O1: OpReturn
// CHECK_OFF_O1: = OpFunction
// CHECK_OFF_O1-COUNT-3: OpReturnValue

// CHECK_OFF_O3: OpEntryPoint GLCompute
// CHECK_OFF_O3: = OpFunction
// CHECK_OFF_O3: OpFunctionCall
// CHECK_OFF_O3: OpReturn
// CHECK_OFF_O3: = OpFunction
// CHECK_OFF_O3-COUNT-3: OpReturnValue

#include "../../tests/spirv/merge-return-workaround.slang"

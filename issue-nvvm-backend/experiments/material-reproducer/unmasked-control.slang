// Research fixture; compile and execute the same computeMain entry.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CONTROL):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CONTROL):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CONTROL):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[1 2 3], stride=4):name=inputs
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputs
// CONTROL: 12
// CONTROL-NEXT: 1
// CONTROL-NEXT: 1
// CONTROL-NEXT: 1
// CONTROL-NEXT: 6
// CONTROL-NEXT: 1
// CONTROL-NEXT: 1
// CONTROL-NEXT: 1

#define SLANG_REPRO_UNMASKED_INDICES
#define SLANG_REPRO_CONTROL
#include "graph.slangh"

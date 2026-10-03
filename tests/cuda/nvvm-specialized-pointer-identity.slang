//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
import "nvvm-specialized-pointer-module";

// Pointer types have no diagnostic name hint. Their distinct pointees must still produce
// distinct generic symbols, including when nested inside an imported nominal wrapper.
//TEST_INPUT:ubuffer(data=[7], stride=4):name=integers
uniform int* integers;
//TEST_INPUT:ubuffer(data=[3.0], stride=4):name=floats
uniform float* floats;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<int> outputs;
// CHECK: 7
// CHECK-NEXT: 3
// CHECK-NEXT: 7
// CHECK-NEXT: 3
[numthreads(1, 1, 1)]
void computeMain()
{
    Wrapper<int*> integerWrapper = { integers };
    Wrapper<float*> floatWrapper = { floats };
    outputs[0] = *preserve(integers);
    outputs[1] = int(*preserve(floats));
    outputs[2] = *integerWrapper.getValue();
    outputs[3] = int(*floatWrapper.getValue());
}

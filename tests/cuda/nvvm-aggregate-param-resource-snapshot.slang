//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
// Keep a selected resource-bearing snapshot in its existing aggregate helper representation.
// Its ParameterBlock field is not independently supported as an NVVM helper value parameter.
struct Material
{
    uint value;
};
struct Scene
{
    uint marker;
    ParameterBlock<Material> material;
    [noinline]
    uint readValue()
    {
        return material.value;
    }
};
//TEST_INPUT: set firstScene = new Scene {41, new Material {7}}
ParameterBlock<Scene> firstScene;
//TEST_INPUT: set secondScene = new Scene {43, new Material {11}}
ParameterBlock<Scene> secondScene;
//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=selectors
StructuredBuffer<uint> selectors;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;
// CHECK: 7
// CHECK-NEXT: 11
[numthreads(1, 1, 1)]
void computeMain()
{
    Scene first = firstScene;
    Scene second = secondScene;
    for (uint i = 0; i < 2; ++i)
    {
        Scene selected = selectors[i] == 0 ? first : second;
        outputs[i] = selected.readValue();
    }
}

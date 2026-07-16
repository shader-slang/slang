//TEST:SIMPLE(filecheck=CHECK_SPV_QUAD): -stage compute -entry computeMain -target spirv -DQUAD
//TEST:SIMPLE(filecheck=CHECK_SPV_LINEAR): -stage compute -entry computeMain -target spirv -DLINEAR
//TEST:SIMPLE(filecheck=CHECK_GLSL_QUAD): -stage compute -entry computeMain -target glsl -DQUAD
//TEST:SIMPLE(filecheck=CHECK_VIA_GLSL_QUAD): -stage compute -entry computeMain -target spirv -emit-spirv-via-glsl -DQUAD
//TEST:SIMPLE(filecheck=CHECK_VIA_GLSL_LINEAR): -stage compute -entry computeMain -target spirv -emit-spirv-via-glsl -DLINEAR
//TEST:SIMPLE(diag=CHECK_ERR): -stage compute -entry computeMain -target spirv -DQUAD_ODD_X

// A workgroup size given by specialization constants (SPIR-V `LocalSizeId`,
// GLSL `local_size_*_id`) is allowed with compute derivative groups; Vulkan
// validates the resolved size at pipeline creation time.

// CHECK_SPV_QUAD-DAG: OpExecutionModeId %computeMain LocalSizeId
// CHECK_SPV_QUAD-DAG: OpExecutionMode %computeMain DerivativeGroupQuads{{NV|KHR}}

// CHECK_SPV_LINEAR-DAG: OpExecutionModeId %computeMain LocalSizeId
// CHECK_SPV_LINEAR-DAG: OpExecutionMode %computeMain DerivativeGroupLinear{{NV|KHR}}

// CHECK_GLSL_QUAD-DAG: layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z = 1) in;
// CHECK_GLSL_QUAD-DAG: layout(derivative_group_quadsNV) in;

// Verify that glslang also accepts spec-constant local size combined with a
// derivative-group layout qualifier.
// CHECK_VIA_GLSL_QUAD-DAG: OpExecutionMode %main DerivativeGroupQuads{{NV|KHR}}
// CHECK_VIA_GLSL_QUAD-DAG: SpecId 0

// CHECK_VIA_GLSL_LINEAR-DAG: OpExecutionMode %main DerivativeGroupLinear{{NV|KHR}}
// CHECK_VIA_GLSL_LINEAR-DAG: SpecId 0

[vk::constant_id(0)]
const int threadCountX = 8;

[vk::constant_id(1)]
const int threadCountY = 8;

RWStructuredBuffer<float> outputBuffer;

#if defined(QUAD)
[DerivativeGroupQuad]
[numthreads(threadCountX, threadCountY, 1)]
#elif defined(LINEAR)
[DerivativeGroupLinear]
[numthreads(threadCountX, threadCountY, 1)]
#elif defined(QUAD_ODD_X)
// A literal axis that violates the quad constraint is still diagnosed even
// when another axis is a specialization constant.
[DerivativeGroupQuad]
[numthreads(3, threadCountY, 1)]
// CHECK_ERR: ([[#@LINE-1]]): error 31210: 
// CHECK_ERR:   [numthreads(3, threadCountY, 1)]
// CHECK_ERR:               ^
#endif
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
/*CHECK_ERR:
     ^^^^^^^^^^^ derivative group quad thread count error
     ^^^^^^^^^^^ compute derivative group quad requires thread dispatch count of X and Y to each be at a multiple of 2
*/
{
    outputBuffer[0] = ddx_fine(float(dispatchThreadID.x));
}

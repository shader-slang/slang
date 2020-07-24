#include "C:/Users/dgeisler/Documents/slang/prelude/slang-cpp-prelude.h"


//namespace { // anonymous 

#ifdef SLANG_PRELUDE_NAMESPACE
using namespace SLANG_PRELUDE_NAMESPACE;
#endif


#line 21 "../../examples/heterogeneous-hello-world/shader.slang"
struct gfx_Window_0
{
};


#line 22
struct gfx_Renderer_0
{
};


#line 23
struct gfx_BufferResource_0
{
};


struct gfx_ShaderProgram_0
{
};


#line 26
struct gfx_DescriptorSetLayout_0
{
};


#line 24
struct gfx_PipelineLayout_0
{
};


#line 27
struct gfx_DescriptorSet_0
{
};


#line 25
struct gfx_PipelineState_0
{
};


#line 34
gfx_Window_0* createWindow_0(int32_t _0, int32_t _1);


#line 35
gfx_Renderer_0* createRenderer_0(int32_t _0, int32_t _1, gfx_Window_0* _2);



gfx_BufferResource_0* createStructuredBuffer_0(gfx_Renderer_0* _0, FixedArray<float, 4> _1);


#line 33
gfx_ShaderProgram_0* loadShaderProgram_0(gfx_Renderer_0* _0);


#line 40
gfx_DescriptorSetLayout_0* buildDescriptorSetLayout_0(gfx_Renderer_0* _0);


#line 41
gfx_PipelineLayout_0* buildPipeline_0(gfx_Renderer_0* _0, gfx_DescriptorSetLayout_0* _1);


#line 42
gfx_DescriptorSet_0* buildDescriptorSet_0(gfx_Renderer_0* _0, gfx_DescriptorSetLayout_0* _1, gfx_BufferResource_0* _2);



gfx_PipelineState_0* buildPipelineState_0(gfx_ShaderProgram_0* _0, gfx_Renderer_0* _1, gfx_PipelineLayout_0* _2);



void printInitialValues_0(FixedArray<float, 4> _0, int32_t _1);


#line 51
void dispatchComputation_0(gfx_Renderer_0* _0, gfx_PipelineState_0* _1, gfx_PipelineLayout_0* _2, gfx_DescriptorSet_0* _3);




void print_output_0(gfx_Renderer_0* _0, gfx_BufferResource_0* _1, int32_t _2);




bool executeComputation_0()
{



    FixedArray<float, 4> initialArray_0 = { 3.00000000000000000000f, -20.00000000000000000000f, -6.00000000000000000000f, 8.00000000000000000000f };


    gfx_Window_0* _S1 = createWindow_0(int(1024), int(768));
    gfx_Renderer_0* _S2 = createRenderer_0(int(1024), int(768), _S1);
    gfx_BufferResource_0* _S3 = createStructuredBuffer_0(_S2, initialArray_0);
    gfx_ShaderProgram_0* _S4 = loadShaderProgram_0(_S2);
    gfx_DescriptorSetLayout_0* _S5 = buildDescriptorSetLayout_0(_S2);
    gfx_PipelineLayout_0* _S6 = buildPipeline_0(_S2, _S5);
    gfx_DescriptorSet_0* _S7 = buildDescriptorSet_0(_S2, _S5, _S3);
    gfx_PipelineState_0* _S8 = buildPipelineState_0(_S4, _S2, _S6);
    printInitialValues_0(initialArray_0, int(4));
    dispatchComputation_0(_S2, _S8, _S6, _S7);
    print_output_0(_S2, _S3, int(4));


    return true;
}

//} // anonymous


#include "../../prelude/slang-cpp-prelude.h"


#ifdef SLANG_PRELUDE_NAMESPACE
using namespace SLANG_PRELUDE_NAMESPACE;
#endif

Vector<uint32_t, 3> operator*(Vector<uint32_t, 3> a, Vector<uint32_t, 3> b)
{
    Vector<uint32_t, 3> r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    return r;
}

Vector<uint32_t, 3> operator+(Vector<uint32_t, 3> a, Vector<uint32_t, 3> b)
{
    Vector<uint32_t, 3> r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}

Vector<uint32_t, 3> make_VecU3(uint32_t a, uint32_t b, uint32_t c)
{
    return Vector<uint32_t, 3>{ a, b, c};
}

size_t __computeMainSize = 668;
unsigned char __computeMain[] = {68, 88, 66, 67, 87, 111, 81, 164, 2, 29, 72, 42, 151, 28, 13, 217, 55, 37, 7, 95, 1, 0, 0, 0, 156, 2, 0, 0, 5, 0, 0, 0, 52, 0, 0, 0, 8, 1, 0, 0, 24, 1, 0, 0, 40, 1, 0, 0, 32, 2, 0, 0, 82, 68, 69, 70, 204, 0, 0, 0, 1, 0, 0, 0, 88, 0, 0, 0, 1, 0, 0, 0, 28, 0, 0, 0, 0, 4, 83, 67, 0, 9, 16, 0, 164, 0, 0, 0, 60, 0, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 101, 110, 116, 114, 121, 80, 111, 105, 110, 116, 80, 97, 114, 97, 109, 115, 95, 105, 111, 66, 117, 102, 102, 101, 114, 95, 48, 0, 60, 0, 0, 0, 1, 0, 0, 0, 112, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 136, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 148, 0, 0, 0, 0, 0, 0, 0, 36, 69, 108, 101, 109, 101, 110, 116, 0, 171, 171, 171, 0, 0, 3, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 77, 105, 99, 114, 111, 115, 111, 102, 116, 32, 40, 82, 41, 32, 72, 76, 83, 76, 32, 83, 104, 97, 100, 101, 114, 32, 67, 111, 109, 112, 105, 108, 101, 114, 32, 49, 48, 46, 49, 0, 73, 83, 71, 78, 8, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 79, 83, 71, 78, 8, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 83, 72, 69, 88, 240, 0, 0, 0, 64, 0, 5, 0, 60, 0, 0, 0, 106, 8, 0, 1, 158, 0, 0, 4, 0, 224, 17, 0, 0, 0, 0, 0, 4, 0, 0, 0, 95, 0, 0, 2, 18, 0, 2, 0, 104, 0, 0, 2, 1, 0, 0, 0, 155, 0, 0, 4, 4, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 167, 0, 0, 8, 18, 0, 16, 0, 0, 0, 0, 0, 10, 0, 2, 0, 1, 64, 0, 0, 0, 0, 0, 0, 6, 224, 17, 0, 0, 0, 0, 0, 49, 0, 0, 7, 34, 0, 16, 0, 0, 0, 0, 0, 10, 0, 16, 0, 0, 0, 0, 0, 1, 64, 0, 0, 0, 0, 0, 63, 0, 0, 0, 7, 66, 0, 16, 0, 0, 0, 0, 0, 10, 0, 16, 0, 0, 0, 0, 0, 10, 0, 16, 0, 0, 0, 0, 0, 75, 0, 0, 5, 18, 0, 16, 0, 0, 0, 0, 0, 10, 0, 16, 0, 0, 0, 0, 0, 55, 0, 0, 9, 18, 0, 16, 0, 0, 0, 0, 0, 26, 0, 16, 0, 0, 0, 0, 0, 42, 0, 16, 0, 0, 0, 0, 0, 10, 0, 16, 0, 0, 0, 0, 0, 168, 0, 0, 8, 18, 224, 17, 0, 0, 0, 0, 0, 10, 0, 2, 0, 1, 64, 0, 0, 0, 0, 0, 0, 10, 0, 16, 0, 0, 0, 0, 0, 62, 0, 0, 1, 83, 84, 65, 84, 116, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
void computeMain_wrapper(gfx_Renderer_0* renderer, Vector<uint32_t, 3> gridDims, 
	RWStructuredBuffer<float> buffer)
{
	gfx_ShaderProgram_0* shaderProgram = loadShaderProgram_0(renderer, __computeMain, __computeMainSize);
	gfx_DescriptorSetLayout_0* setLayout = buildDescriptorSetLayout_0(renderer);
	gfx_PipelineLayout_0* pipelineLayout = buildPipeline_0(renderer, setLayout);
	gfx_DescriptorSet_0* descriptorSet = buildDescriptorSet_0(renderer, setLayout, unconvertBuffer_0(buffer));
	gfx_PipelineState_0* pipelineState = buildPipelineState_0(shaderProgram, renderer, pipelineLayout);
	dispatchComputation_0(renderer, pipelineState, pipelineLayout, descriptorSet, gridDims.x, gridDims.y, gridDims.z);
}

#line 7 "../../examples/heterogeneous-hello-world/shader.slang"
struct EntryPointParams_0
{
    RWStructuredBuffer<float> ioBuffer_0;
};

struct KernelContext_0
{
};


#line 21
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


#line 7
void _computeMain(void* _S1, void* entryPointParams_0, void* _S2)
{
    ComputeThreadVaryingInput* _S3 = ((ComputeThreadVaryingInput*)(_S1));
    KernelContext_0 kernelContext_0;

#line 9
    uint32_t tid_0 = (*(&_S3->groupID) * make_VecU3(4U, 1U, 1U) + *(&_S3->groupThreadID)).x;

    float* _S4 = &(*(&((EntryPointParams_0*)(entryPointParams_0))->ioBuffer_0))[tid_0];

#line 11
    float i_0 = *_S4;
    bool _S5 = i_0 < 0.50000000000000000000f;

#line 12
    float _S6 = i_0 + i_0;

#line 12
    float _S7 = (F32_sqrt((i_0)));

#line 12
    float o_0 = _S5 ? _S6 : _S7;

    float* _S8 = &(*(&((EntryPointParams_0*)(entryPointParams_0))->ioBuffer_0))[tid_0];

#line 14
    *_S8 = o_0;

#line 7
    return;
}


#line 34
gfx_Window_0* createWindow_0(int32_t _0, int32_t _1);


#line 35
gfx_Renderer_0* createRenderer_0(int32_t _0, int32_t _1, gfx_Window_0* _2);



gfx_BufferResource_0* createStructuredBuffer_0(gfx_Renderer_0* _0, FixedArray<float, 4> _1);


#line 4
RWStructuredBuffer<float> convertBuffer_0(gfx_BufferResource_0* _0);


#line 40
void printInitialValues_0(FixedArray<float, 4> _0, int32_t _1);


#line 41
void print_output_0(gfx_Renderer_0* _0, gfx_BufferResource_0* _1, int32_t _2);




bool executeComputation_0()
{



    FixedArray<float, 4> initialArray_0 = { 3.00000000000000000000f, -20.00000000000000000000f, -6.00000000000000000000f, 8.00000000000000000000f };


    gfx_Window_0* _S9 = createWindow_0(int(1024), int(768));
    gfx_Renderer_0* _S10 = createRenderer_0(int(1024), int(768), _S9);
    gfx_BufferResource_0* _S11 = createStructuredBuffer_0(_S10, initialArray_0);
    Vector<uint32_t, 3> _S12 = make_VecU3(uint32_t(int(4)), uint32_t(int(1)), uint32_t(int(1)));
    RWStructuredBuffer<float> _S13 = convertBuffer_0(_S11);

#line 57
    computeMain_wrapper(_S10, _S12, _S13);

    printInitialValues_0(initialArray_0, int(4));
    print_output_0(_S10, _S11, int(4));


    return true;
}

// [numthreads(4, 1, 1)]
SLANG_PRELUDE_EXPORT
void computeMain_Thread(ComputeThreadVaryingInput* varyingInput, void* entryPointParams, void* globalParams)
{
    _computeMain(varyingInput, entryPointParams, globalParams);
}
// [numthreads(4, 1, 1)]
SLANG_PRELUDE_EXPORT
void computeMain_Group(ComputeVaryingInput* varyingInput, void* entryPointParams, void* globalParams)
{
    ComputeThreadVaryingInput threadInput = {};
    threadInput.groupID = varyingInput->startGroupID;
    for (uint32_t x = 0; x < 4; ++x)
    {
        threadInput.groupThreadID.x = x;
        _computeMain(&threadInput, entryPointParams, globalParams);
    }
}
// [numthreads(4, 1, 1)]
SLANG_PRELUDE_EXPORT
void computeMain(ComputeVaryingInput* varyingInput, void* entryPointParams, void* globalParams)
{
    ComputeVaryingInput vi = *varyingInput;
    ComputeVaryingInput groupVaryingInput = {};
    for (uint32_t z = vi.startGroupID.z; z < vi.endGroupID.z; ++z)
    {
        groupVaryingInput.startGroupID.z = z;
        for (uint32_t y = vi.startGroupID.y; y < vi.endGroupID.y; ++y)
        {
            groupVaryingInput.startGroupID.y = y;
            for (uint32_t x = vi.startGroupID.x; x < vi.endGroupID.x; ++x)
            {
                groupVaryingInput.startGroupID.x = x;
                computeMain_Group(&groupVaryingInput, entryPointParams, globalParams);
            }
        }
    }
}

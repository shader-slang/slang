#include "C:/Code/slang/prelude/slang-cpp-prelude.h"


#ifdef SLANG_PRELUDE_NAMESPACE
using namespace SLANG_PRELUDE_NAMESPACE;
#endif

Vector<uint32_t, 3> operator+(Vector<uint32_t, 3> a, Vector<uint32_t, 3> b)
{
    Vector<uint32_t, 3> r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}

Vector<uint32_t, 3> operator*(Vector<uint32_t, 3> a, Vector<uint32_t, 3> b)
{
    Vector<uint32_t, 3> r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    return r;
}

Vector<uint32_t, 3> make_VecU3(uint32_t a, uint32_t b, uint32_t c)
{
    return Vector<uint32_t, 3>{ a, b, c};
}

void computeMain_wrapper(gfx_Device_0* device, Vector<uint32_t, 3> gridDims, 
	RWStructuredBuffer<float> buffer)
{
	gfx_ShaderProgram_0* shaderProgram = loadShaderProgram_0(device, "computeMain");
	gfx_TransientResourceHeap_0* transientHeap = buildTransientHeap_0(device);
	gfx_PipelineState_0* pipelineState = buildPipelineState_0(device, shaderProgram);
	gfx_ResourceView_0* bufferView = createBufferView_0(device, unconvertBuffer_0(buffer));
	dispatchComputation_0(device, transientHeap, pipelineState, bufferView, gridDims.x, gridDims.y, gridDims.z);
}

#line 8 "C:/Code/slang/examples/heterogeneous-hello-world/shader.slang"
struct EntryPointParams_0
{
    RWStructuredBuffer<float> ioBuffer_0;
};


#line 21
struct gfx_Device_0
{
};


#line 22
struct gfx_BufferResource_0
{
};


#line 23
struct gfx_ResourceView_0
{
};


#line 8
void _computeMain(void* _S1, void* entryPointParams_0, void* _S2)
{

#line 8
    ComputeThreadVaryingInput* _S3 = (slang_bit_cast<ComputeThreadVaryingInput*>(_S1));

    uint32_t tid_0 = (*(&_S3->groupID) * make_VecU3(4U, 1U, 1U) + *(&_S3->groupThreadID)).x;

    float* _S4 = &(*(&(slang_bit_cast<EntryPointParams_0*>(entryPointParams_0))->ioBuffer_0))[tid_0];

#line 12
    float i_0 = *_S4;
    bool _S5 = i_0 < 0.50000000000000000000f;

#line 13
    float _S6 = i_0 + i_0;

#line 13
    float _S7 = (F32_sqrt((i_0)));

#line 13
    float o_0 = _S5 ? _S6 : _S7;

    float* _S8 = &(*(&(slang_bit_cast<EntryPointParams_0*>(entryPointParams_0))->ioBuffer_0))[tid_0];

#line 15
    *_S8 = o_0;
    return;
}


#line 31
gfx_Device_0* createDevice_0();

gfx_BufferResource_0* createStructuredBuffer_0(gfx_Device_0* _0, FixedArray<float, 4> _1);


gfx_ResourceView_0* createBufferView_0(gfx_Device_0* _0, gfx_BufferResource_0* _1);


#line 4
RWStructuredBuffer<float> convertBuffer_0(gfx_BufferResource_0* _0);


#line 44
void printInitialValues_0(FixedArray<float, 4> _0, int32_t _1);


#line 50
bool printOutputValues_0(gfx_Device_0* _0, gfx_BufferResource_0* _1, int32_t _2);




bool executeComputation_0()
{

    FixedArray<float, 4> initialArray_0 = { 3.00000000000000000000f, -20.00000000000000000000f, -6.00000000000000000000f, 8.00000000000000000000f };


    gfx_Device_0* _S9 = createDevice_0();
    gfx_BufferResource_0* _S10 = createStructuredBuffer_0(_S9, initialArray_0);
    gfx_ResourceView_0* _S11 = createBufferView_0(_S9, _S10);
    Vector<uint32_t, 3> _S12 = make_VecU3(uint32_t(int(4)), uint32_t(int(1)), uint32_t(int(1)));
    RWStructuredBuffer<float> _S13 = convertBuffer_0(_S10);

#line 64
    computeMain_wrapper(_S9, _S12, _S13);

    printInitialValues_0(initialArray_0, int(4));
    bool _S14 = printOutputValues_0(_S9, _S10, int(4));


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

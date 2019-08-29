#ifndef SLANG_PRELUDE_CPP_TYPES_H
#define SLANG_PRELUDE_CPP_TYPES_H

#include "../slang.h"

#ifdef SLANG_PRELUDE_NAMESPACE
namespace SLANG_PRELUDE_NAMESPACE {
#endif

template <typename T, size_t SIZE>
struct FixedArray
{
    const T& operator[](size_t index) const { assert(index < SIZE); return m_data[index]; }
    T& operator[](size_t index) { assert(index < SIZE); return m_data[index]; }
    
    T m_data[SIZE];
};


// Hmm... I guess a constant buffer should be unwrapped to be just a struct passed in
/* template <typename T>
struct ConstantBuffer
{
}; */

template <typename T, int COUNT>
struct Vector;

template <typename T>
struct Vector<T, 1>
{
    T x;
};

template <typename T>
struct Vector<T, 2>
{
    T x, y;
};

template <typename T>
struct Vector<T, 3>
{
    T x, y, z;
};

template <typename T>
struct Vector<T, 4>
{
    T x, y, z, w;
};


typedef Vector<float, 2> float2;
typedef Vector<float, 3> float3;
typedef Vector<float, 4> float4;

typedef Vector<int32_t, 2> int2;
typedef Vector<int32_t, 3> int3;
typedef Vector<int32_t, 4> int4;

typedef Vector<uint32_t, 2> uint2;
typedef Vector<uint32_t, 3> uint3;
typedef Vector<uint32_t, 4> uint4;

template <typename T, int ROWS, int COLS>
struct Matrix
{
    Vector<T, COLS> rows[ROWS];
};

// ----------------------------- ResourceType -----------------------------------------

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-structuredbuffer-getdimensions
// Missing  Load(_In_  int  Location, _Out_ uint Status);

template <typename T>
struct RWStructuredBuffer
{
    T& operator[](size_t index) const { assert(index < count); return data[index]; }
    const T& Load(size_t index) const { assert(index < count); return data[index]; }  
    void GetDimensions(uint32_t& outNumStructs, uint32_t& outStride) { outNumStructs = uint32_t(count); outStride = uint32_t(sizeof(T)); }
  
    T* data;
    size_t count;
};

template <typename T>
struct StructuredBuffer
{
    const T& operator[](size_t index) const { assert(index < count); return data[index]; }
    const T& Load(size_t index) const { assert(index < count); return data[index]; }
    void GetDimensions(uint32_t& outNumStructs, uint32_t& outStride) { outNumStructs = uint32_t(count); outStride = uint32_t(sizeof(T)); }
    
    T* data;
    size_t count;
};

// Missing  Load(_In_  int  Location, _Out_ uint Status);
struct ByteAddressBuffer
{
    void GetDimensions(uint32_t& outDim) const { outDim = uint32_t(sizeInBytes); }
    uint32_t Load(size_t index) const 
    { 
        assert(index + 4 <= sizeInBytes && (index & 3) == 0); 
        return data[index >> 2]; 
    }
    uint2 Load2(size_t index) const 
    { 
        assert(index + 8 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint2{data[dataIdx], data[dataIdx + 1]}; 
    }
    uint3 Load3(size_t index) const 
    { 
        assert(index + 12 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]}; 
    }
    uint4 Load4(size_t index) const 
    { 
        assert(index + 16 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]}; 
    }
    
    const uint32_t* data;
    size_t sizeInBytes;  //< Must be multiple of 4
};

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-rwbyteaddressbuffer
// Missing support for Atomic operations 
// Missing support for Load with status
struct RWByteAddressBuffer
{
    void GetDimensions(uint32_t& outDim) const { outDim = uint32_t(sizeInBytes); }
    
    uint32_t Load(size_t index) const 
    { 
        assert(index + 4 <= sizeInBytes && (index & 3) == 0); 
        return data[index >> 2]; 
    }
    uint2 Load2(size_t index) const 
    { 
        assert(index + 8 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint2{data[dataIdx], data[dataIdx + 1]}; 
    }
    uint3 Load3(size_t index) const 
    { 
        assert(index + 12 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]}; 
    }
    uint4 Load4(size_t index) const 
    { 
        assert(index + 16 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]}; 
    }
    
    void Store(size_t index, uint32_t v) const 
    { 
        assert(index + 4 <= sizeInBytes && (index & 3) == 0); 
        data[index >> 2] = v; 
    }
    void Store2(size_t index, uint2 v) const 
    { 
        assert(index + 8 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
    }
    void Store3(size_t index, uint3 v) const 
    { 
        assert(index + 12 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
    }
    void Store4(size_t index, uint4 v) const 
    { 
        assert(index + 16 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
        data[dataIdx + 3] = v.w;
    }
    
    uint32_t* data;
    size_t sizeInBytes; //< Must be multiple of 4 
};

struct ISamplerState;
struct ISamplerComparisonState;

struct SamplerState
{
    ISamplerState* state;
};

struct SamplerComparisonState
{
    ISamplerComparisonState* state;
};

// Texture

struct ITexture2D
{
    virtual void Load(const int3& v, void* out) = 0;
    virtual void Sample(SamplerState samplerState, const float2& loc, void* out) = 0;
    virtual void SampleLevel(SamplerState samplerState, const float2& loc, float level, void* out) = 0;
};

template <typename T>
struct Texture2D
{
    T Load(const int3& v) const { T out; texture->Load(v, &out); return out; }
    T Sample(SamplerState samplerState, const float2& v) const { T out; texture->Sample(samplerState, v, &out); return out; }
    T SampleLevel(SamplerState samplerState, const float2& v, float level) { T out; texture->SampleLevel(samplerState, v, level, &out); return out; }
    
    ITexture2D* texture;              
};

/* Varying input for Compute */
struct ComputeVaryingInput
{
    uint3 groupID;
    uint3 groupThreadID;
};

/* Type that defines the uniform entry point params. The actual content of this type is dependent on the entry point parameters, and can be
found via reflection or defined such that it matches the shader appropriately. */
struct UniformEntryPointParams;

#ifdef SLANG_PRELUDE_NAMESPACE
}
#endif

#endif



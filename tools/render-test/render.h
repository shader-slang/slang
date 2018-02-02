// render.h
#pragma once

#include "options.h"
#include "window.h"
#include "shader-input-layout.h"

namespace renderer_test {

typedef struct Buffer           Buffer;
typedef struct InputLayout      InputLayout;
typedef struct ShaderProgram    ShaderProgram;
typedef struct BindingState     BindingState;
struct ShaderCompileRequest
{
    struct SourceInfo
    {
        char const* path;

        // The data may either be source text (in which
        // case it can be assumed to be nul-terminated with
        // `dataEnd` pointing at the terminator), or
        // raw binary data (in which case `dataEnd` points
        // at the end of the buffer).
        char const* dataBegin;
        char const* dataEnd;
    };

    struct EntryPoint
    {
        char const* name = nullptr;
        char const* profile = nullptr;

        SourceInfo  source;
    };

    SourceInfo source;
    EntryPoint vertexShader;
    EntryPoint fragmentShader;
	EntryPoint computeShader;
    Slang::List<Slang::String> entryPointTypeArguments;
};

class ShaderCompiler
{
public:
    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) = 0;
};

enum class Format
{
    Unknown,
    RGB_Float32,
    RG_Float32,
};

enum class BufferFlavor
{
    Constant,
    Vertex
};

struct BufferDesc
{
    UInt            size        = 0;
    BufferFlavor    flavor      = BufferFlavor::Constant;
    void const*     initData    = nullptr;
};

struct InputElementDesc
{
    char const* semanticName;
    UInt        semanticIndex;
    Format      format;
    UInt        offset;
};

enum class MapFlavor
{
	HostRead,
	HostWrite,
    WriteDiscard,
};

enum class PrimitiveTopology
{
    TriangleList,
};

class Renderer
{
public:
    virtual void initialize(void* inWindowHandle) = 0;

    virtual void setClearColor(float const* color) = 0;
    virtual void clearFrame() = 0;

    virtual void presentFrame() = 0;

    virtual void captureScreenShot(char const* outputPath) = 0;
    virtual void serializeOutput(BindingState * state, char const* outputPath) = 0;
    virtual Buffer* createBuffer(BufferDesc const& desc) = 0;

    virtual InputLayout* createInputLayout(InputElementDesc const* inputElements, UInt inputElementCount) = 0;
    virtual BindingState* createBindingState(const ShaderInputLayout & shaderInput) = 0;
    virtual ShaderCompiler* getShaderCompiler() = 0;

    virtual void* map(Buffer* buffer, MapFlavor flavor) = 0;
    virtual void unmap(Buffer* buffer) = 0;

    virtual void setInputLayout(InputLayout* inputLayout) = 0;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;
    virtual void setBindingState(BindingState * state) = 0;
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* strides, UInt const* offsets) = 0;

    inline void setVertexBuffer(UInt slot, Buffer* buffer, UInt stride, UInt offset = 0)
    {
        setVertexBuffers(slot, 1, &buffer, &stride, &offset);
    }

    virtual void setShaderProgram(ShaderProgram* program) = 0;

    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets) = 0;
    inline void setConstantBuffer(UInt slot, Buffer* buffer, UInt offset = 0)
    {
        setConstantBuffers(slot, 1, &buffer, &offset);
    }
    virtual void draw(UInt vertexCount, UInt startVertex = 0) = 0;
	virtual void dispatchCompute(int x, int y, int z) = 0;
};

} // renderer_test

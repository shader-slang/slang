// render.h
#pragma once

#include "options.h"
#include "window.h"

namespace renderer_test {

typedef struct Buffer           Buffer;
typedef struct InputLayout      InputLayout;
typedef struct ShaderProgram    ShaderProgram;

struct ShaderCompileRequest
{
    struct SourceInfo
    {
        char const* path;
        char const* text;
    };

    struct EntryPoint
    {
        char const* name;
        char const* profile;

        SourceInfo  source;
    };

    SourceInfo source;
    EntryPoint vertexShader;
    EntryPoint fragmentShader;
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
};

enum class BufferFlavor
{
    Constant,
    Vertex,
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

    virtual Buffer* createBuffer(BufferDesc const& desc) = 0;

    virtual InputLayout* createInputLayout(InputElementDesc const* inputElements, UInt inputElementCount) = 0;

    virtual ShaderCompiler* getShaderCompiler() = 0;

    virtual void* map(Buffer* buffer, MapFlavor flavor) = 0;
    virtual void unmap(Buffer* buffer) = 0;

    virtual void setInputLayout(InputLayout* inputLayout) = 0;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;

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
};

} // renderer_test

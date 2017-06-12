// render-gl.cpp
#include "render-gl.h"

#include "options.h"
#include "render.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "external/stb/stb_image_write.h"

// TODO(tfoley): eventually we should be able to run these
// tests on non-Windows targets to confirm that cross-compilation
// at least *works* on those platforms...
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#ifdef _MSC_VER
#include <stddef.h>
#if (_MSC_VER < 1900)
#define snprintf sprintf_s
#endif
#endif

#pragma comment(lib, "opengl32")

#include <GL/GL.h>
#include "external/glext.h"

// We define an "X-macro" for mapping over loadable OpenGL
// extension entry point that we will use, so that we can
// easily write generic code to iterate over them.
#define MAP_GL_EXTENSION_FUNCS(F)                       \
    F(glCreateProgram,      PFNGLCREATEPROGRAMPROC)     \
    F(glCreateShader,       PFNGLCREATESHADERPROC)      \
    F(glShaderSource,       PFNGLSHADERSOURCEPROC)      \
    F(glCompileShader,      PFNGLCOMPILESHADERPROC)     \
    F(glGetShaderiv,        PFNGLGETSHADERIVPROC)       \
    F(glDeleteShader,       PFNGLDELETESHADERPROC)      \
    F(glAttachShader,       PFNGLATTACHSHADERPROC)      \
    F(glLinkProgram,        PFNGLLINKPROGRAMPROC)       \
    F(glGetProgramiv,       PFNGLGETPROGRAMIVPROC)      \
    F(glGetProgramInfoLog,  PFNGLGETPROGRAMINFOLOGPROC) \
    F(glDeleteProgram,      PFNGLDELETEPROGRAMPROC)     \
    F(glGetShaderInfoLog,   PFNGLGETSHADERINFOLOGPROC)  \
    F(glGenBuffers,         PFNGLGENBUFFERSPROC)        \
    F(glBindBuffer,         PFNGLBINDBUFFERPROC)        \
    F(glBufferData,         PFNGLBUFFERDATAPROC)        \
    F(glMapBuffer,          PFNGLMAPBUFFERPROC)         \
    F(glUnmapBuffer,        PFNGLUNMAPBUFFERPROC)       \
    F(glUseProgram,         PFNGLUSEPROGRAMPROC)        \
    F(glBindBufferBase,     PFNGLBINDBUFFERBASEPROC)    \
    F(glVertexAttribPointer,        PFNGLVERTEXATTRIBPOINTERPROC)       \
    F(glEnableVertexAttribArray,    PFNGLENABLEVERTEXATTRIBARRAYPROC)   \
    F(glDisableVertexAttribArray,   PFNGLDISABLEVERTEXATTRIBARRAYPROC)  \
    F(glDebugMessageCallback,       PFNGLDEBUGMESSAGECALLBACKPROC)      \
    /* end */

namespace renderer_test {

class GLRenderer : public Renderer, public ShaderCompiler
{
public:
    HDC     deviceContext;
    HGLRC   glContext;

    // Declre a function pointer for each OpenGL
    // extension function we need to load

#define DECLARE_GL_EXTENSION_FUNC(NAME, TYPE) TYPE NAME;
    MAP_GL_EXTENSION_FUNCS(DECLARE_GL_EXTENSION_FUNC)
#undef DECLARE_GL_EXTENSION_FUNC

    // Renderer interface

    virtual void initialize(void* inWindowHandle) override
    {
        auto windowHandle = (HWND) inWindowHandle;

        deviceContext = GetDC(windowHandle);

        PIXELFORMATDESCRIPTOR pixelFormatDesc = { sizeof(PIXELFORMATDESCRIPTOR) };
        pixelFormatDesc.nVersion = 1;
        pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
        pixelFormatDesc.cColorBits = 32;
        pixelFormatDesc.cDepthBits = 24;
        pixelFormatDesc.cStencilBits = 8;
        pixelFormatDesc.iLayerType = PFD_MAIN_PLANE;

        int pixelFormatIndex = ChoosePixelFormat(deviceContext, &pixelFormatDesc);
        SetPixelFormat(deviceContext, pixelFormatIndex, &pixelFormatDesc);

        glContext = wglCreateContext(deviceContext);
        wglMakeCurrent(deviceContext, glContext);

        auto renderer = glGetString(GL_RENDERER);
        auto extensions = glGetString(GL_EXTENSIONS);

        // Load ech of our etension functions by name

    #define LOAD_GL_EXTENSION_FUNC(NAME, TYPE) NAME = (TYPE) wglGetProcAddress(#NAME);
        MAP_GL_EXTENSION_FUNCS(LOAD_GL_EXTENSION_FUNC)
    #undef LOAD_GL_EXTENSION_FUNC


        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glViewport(0, 0, gWindowWidth, gWindowHeight);

        if (glDebugMessageCallback)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(staticDebugCallback, this);
        }
    }

    void debugCallback(
        GLenum          source,
        GLenum          type,
        GLuint          id,
        GLenum          severity,
        GLsizei         length,
        GLchar const*   message)
    {
        OutputDebugStringA("GL: ");
        OutputDebugStringA(message);
        OutputDebugStringA("\n");

        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:
            assert(!"unexpected");
            break;

        default:
            break;
        }
    }

    static void APIENTRY staticDebugCallback(
        GLenum          source,
        GLenum          type,
        GLuint          id,
        GLenum          severity,
        GLsizei         length,
        GLchar const*   message,
        void const*     userParam)
    {
        ((GLRenderer*) userParam)->debugCallback(
            source, type, id, severity, length, message);
    }

    float clearColor[4] = { 0, 0, 0, 0 };
    virtual void setClearColor(float const* color) override
    {
        glClearColor(color[0], color[1], color[2], color[3]);
    }

    virtual void clearFrame() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    virtual void presentFrame() override
    {
        glFlush();
        SwapBuffers(deviceContext);
    }

    virtual void captureScreenShot(char const* outputPath) override
    {
        int width = gWindowWidth;
        int height = gWindowHeight;

        int components = 4;
        int rowStride = width*components;

        GLubyte* buffer = (GLubyte*)malloc(components * width * height);

        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

        // OpenGL's "upside down" convention bites us here, so we need
        // to flip the data in the buffer by swapping rows
        int halfHeight = height / 2;
        for (int hh = 0; hh < halfHeight; ++hh)
        {
            // Get a pointer to the row data, and a pointer
            // to the row on the "other end" of the image
            GLubyte* rowA = buffer + rowStride*hh;
            GLubyte* rowB = buffer + rowStride*(height - (hh + 1));

            for (int ii = 0; ii < rowStride; ++ii)
            {
                auto a = rowA[ii];
                auto b = rowB[ii];

                rowA[ii] = b;
                rowB[ii] = a;
            }
        }

        //

        int stbResult = stbi_write_png(
            outputPath,
            width,
            height,
            components,
            buffer,
            rowStride);
        if( !stbResult )
        {
            assert(!"unexpected");
        }

        delete(buffer);
    }

    virtual ShaderCompiler* getShaderCompiler() override
    {
        return this;
    }

    virtual Buffer* createBuffer(BufferDesc const& desc) override
    {
        // TODO: should derive target from desc...
        GLenum target = GL_UNIFORM_BUFFER;

        // TODO: should derive from desc...
        GLenum usage = GL_DYNAMIC_DRAW;

        GLuint bufferID = 0;
        glGenBuffers(1, &bufferID);
        glBindBuffer(target, bufferID);

        glBufferData(target, desc.size, desc.initData, usage);

        return (Buffer*)(uintptr_t)bufferID;
    }

    struct VertexAttributeFormat
    {
        GLint       componentCount;
        GLenum      componentType;
        GLboolean   normalized;
    };

    struct VertexAttributeDesc
    {
        VertexAttributeFormat   format;
        GLuint                  streamIndex;
        GLsizei                 offset;
    };

    enum
    {
        kMaxVertexStreams = 16,
    };

    struct InputLayoutImpl
    {
        VertexAttributeDesc attributes[kMaxVertexStreams];
        UInt attributeCount = 0;
    };

    static VertexAttributeFormat getVertexAttributeFormat(
        Format format)
    {
        switch (format)
        {
        default: assert(!"unexpected"); return VertexAttributeFormat();

    #define CASE(NAME, COUNT, TYPE, NORMALIZED) \
        case Format::NAME: do { VertexAttributeFormat result = {COUNT, TYPE, NORMALIZED}; return result; } while (0)

        CASE(RGB_Float32, 3, GL_FLOAT, GL_FALSE);

    #undef CASE

        }
    }

    virtual InputLayout* createInputLayout(InputElementDesc const* inputElements, UInt inputElementCount) override
    {
        InputLayoutImpl* inputLayout = new InputLayoutImpl();

        inputLayout->attributeCount = inputElementCount;
        for (UInt ii = 0; ii < inputElementCount; ++ii)
        {
            auto& inputAttr = inputElements[ii];
            auto& glAttr = inputLayout->attributes[ii];

            glAttr.streamIndex = 0;
            glAttr.format = getVertexAttributeFormat(inputAttr.format);
            glAttr.offset = inputAttr.offset;
        }

        return (InputLayout*)inputLayout;
    }

    virtual void* map(Buffer* buffer, MapFlavor flavor) override
    {
        GLenum target = GL_UNIFORM_BUFFER;

        GLuint access = 0;
        switch (flavor)
        {
        case MapFlavor::WriteDiscard:
            access = GL_WRITE_ONLY;
            break;
        }

        auto bufferID = (GLuint)(uintptr_t)buffer;
        glBindBuffer(target, bufferID);

        return glMapBuffer(target, access);
    }

    virtual void unmap(Buffer* buffer) override
    {
        GLenum target = GL_UNIFORM_BUFFER;

        auto bufferID = (GLuint)(uintptr_t)buffer;
        glUnmapBuffer(target);
    }

    InputLayoutImpl* boundInputLayout = nullptr;

    virtual void setInputLayout(InputLayout* inputLayout) override
    {
        boundInputLayout = (InputLayoutImpl*) inputLayout;
    }

    GLenum boundPrimitiveTopology = GL_TRIANGLES;

    virtual void setPrimitiveTopology(PrimitiveTopology topology) override
    {
        GLenum glTopology = 0;
        switch (topology)
        {
    #define CASE(NAME, VALUE) case PrimitiveTopology::NAME: glTopology = VALUE; break

        CASE(TriangleList, GL_TRIANGLES);

    #undef CASE
        }
        boundPrimitiveTopology = glTopology;
    }

    GLuint  boundVertexStreamBuffers[kMaxVertexStreams];
    UInt    boundVertexStreamStrides[kMaxVertexStreams];
    UInt    boundVertexStreamOffsets[kMaxVertexStreams];

    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* strides, UInt const* offsets) override
    {
        for (UInt ii = 0; ii < slotCount; ++ii)
        {
            UInt slot = startSlot + ii;

            Buffer* buffer = buffers[ii];
            GLuint bufferID = (GLuint)(uintptr_t)buffer;

            boundVertexStreamBuffers[slot] = bufferID;
            boundVertexStreamStrides[slot] = strides[ii];
            boundVertexStreamOffsets[slot] = offsets[ii];
        }
    }

    virtual void setShaderProgram(ShaderProgram* program) override
    {
        GLuint programID = (GLuint)(uintptr_t)program;
        glUseProgram(programID);
    }

    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets) override
    {
        for (UInt ii = 0; ii < slotCount; ++ii)
        {
            UInt slot = startSlot + ii;

            Buffer* buffer = buffers[ii];
            GLuint bufferID = (GLuint)(uintptr_t)buffer;

            assert(!offsets || !offsets[ii]);

            glBindBufferBase(GL_UNIFORM_BUFFER, slot, bufferID);
        }
    }

    void flushStateForDraw()
    {
        auto layout = this->boundInputLayout;
        auto attrCount = layout->attributeCount;
        for (UInt ii = 0; ii < attrCount; ++ii)
        {
            auto& attr = layout->attributes[ii];

            auto streamIndex = attr.streamIndex;

            glBindBuffer(GL_ARRAY_BUFFER, boundVertexStreamBuffers[streamIndex]);

            glVertexAttribPointer(
                ii,
                attr.format.componentCount,
                attr.format.componentType,
                attr.format.normalized,
                boundVertexStreamStrides[streamIndex],
                (GLvoid*)(attr.offset + boundVertexStreamOffsets[streamIndex]));

            glEnableVertexAttribArray(ii);
        }
        for (UInt ii = attrCount; ii < kMaxVertexStreams; ++ii)
        {
            glDisableVertexAttribArray(ii);
        }
    }

    virtual void draw(UInt vertexCount, UInt startVertex = 0) override
    {
        flushStateForDraw();

        glDrawArrays(boundPrimitiveTopology, startVertex, vertexCount);
    }

    // ShaderCompiler interface

    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override
    {
        auto programID = glCreateProgram();

        auto vertexShaderID   = loadShader(GL_VERTEX_SHADER,   request.vertexShader  .source.text);
        auto fragmentShaderID = loadShader(GL_FRAGMENT_SHADER, request.fragmentShader.source.text);

        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);

        glLinkProgram(programID);

        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);

        GLint success = GL_FALSE;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if( !success )
        {
            int maxSize = 0;
            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxSize);

            auto infoBuffer = (char*) malloc(maxSize);

            int infoSize = 0;
            glGetProgramInfoLog(programID, maxSize, &infoSize, infoBuffer);
            if( infoSize > 0 )
            {
                fprintf(stderr, "%s", infoBuffer);
                OutputDebugStringA(infoBuffer);
            }

            glDeleteProgram(programID);
            return 0;
        }

        return (ShaderProgram*) (uintptr_t) programID;
    }

    GLuint loadShader(GLenum stage, char const* source)
    {
        auto shaderID = glCreateShader(stage);

        char const* stagePrelude = "\n";
        switch (stage)
        {
#define CASE(NAME) case GL_##NAME##_SHADER: stagePrelude = "#define __GLSL_" #NAME "__ 1\n"; break

        CASE(VERTEX);
        CASE(TESS_CONTROL);
        CASE(TESS_EVALUATION);
        CASE(GEOMETRY);
        CASE(FRAGMENT);
        CASE(COMPUTE);

#undef CASE
        }

        char const* prelude =
            "#define __GLSL__ 1\n"
            ;

        char const* sourceStrings[] =
        {
            stagePrelude,
            prelude,
            source,
        };

        glShaderSource(
            shaderID,
            sizeof(sourceStrings) / sizeof(sourceStrings[0]),
            &sourceStrings[0],
            nullptr);
        glCompileShader(shaderID);

        GLint success = GL_FALSE;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if( !success )
        {
            int maxSize = 0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxSize);

            auto infoBuffer = (char*) malloc(maxSize);

            int infoSize = 0;
            glGetShaderInfoLog(shaderID, maxSize, &infoSize, infoBuffer);
            if( infoSize > 0 )
            {
                fprintf(stderr, "%s", infoBuffer);
                OutputDebugStringA(infoBuffer);
            }

            glDeleteShader(shaderID);
            return 0;
        }

        return shaderID;
    }
};



Renderer* createGLRenderer()
{
    return new GLRenderer();
}

} // renderer_test

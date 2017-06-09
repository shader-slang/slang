// render-gl.cpp
#include "render-gl.h"

#include "options.h"
#include "render.h"

#include <stdio.h>
#include <stdlib.h>

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
    /* emtty */

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

    }

    virtual ShaderCompiler* getShaderCompiler() override
    {
        return this;
    }

    virtual Buffer* createBuffer(BufferDesc const& desc) override
    {
        return nullptr;
    }

    virtual InputLayout* createInputLayout(InputElementDesc const* inputElements, UInt inputElementCount) override
    {
        return nullptr;
    }

    virtual void* map(Buffer* buffer, MapFlavor flavor) override
    {
        return nullptr;
    }

    virtual void unmap(Buffer* buffer) override
    {
    }

    virtual void setInputLayout(InputLayout* inputLayout) override
    {
    }

    virtual void setPrimitiveTopology(PrimitiveTopology topology) override
    {
    }

    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* strides, UInt const* offsets) override
    {
    }

    virtual void setShaderProgram(ShaderProgram* program) override
    {
    }

    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets) override
    {
    }


    virtual void draw(UInt vertexCount, UInt startVertex = 0) override
    {
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

        glShaderSource(shaderID, 1, &source, nullptr);
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

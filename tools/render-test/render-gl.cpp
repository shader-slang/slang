// render-gl.cpp
#include "render-gl.h"

#include "options.h"
#include "render.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "core/basic.h"
#include "core/secure-crt.h"
#include "external/stb/stb_image_write.h"

#include "surface.h"

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
	F(glDeleteBuffers,		PFNGLDELETEBUFFERSPROC)		\
	F(glMapBuffer,          PFNGLMAPBUFFERPROC)         \
    F(glUnmapBuffer,        PFNGLUNMAPBUFFERPROC)       \
    F(glUseProgram,         PFNGLUSEPROGRAMPROC)        \
    F(glBindBufferBase,     PFNGLBINDBUFFERBASEPROC)    \
    F(glVertexAttribPointer,        PFNGLVERTEXATTRIBPOINTERPROC)       \
    F(glEnableVertexAttribArray,    PFNGLENABLEVERTEXATTRIBARRAYPROC)   \
    F(glDisableVertexAttribArray,   PFNGLDISABLEVERTEXATTRIBARRAYPROC)  \
    F(glDebugMessageCallback,       PFNGLDEBUGMESSAGECALLBACKPROC)      \
    F(glDispatchCompute,            PFNGLDISPATCHCOMPUTEPROC) \
    F(glActiveTexture,              PFNGLACTIVETEXTUREPROC) \
    F(glCreateSamplers,             PFNGLCREATESAMPLERSPROC) \
	F(glDeleteSamplers,				PFNGLDELETESAMPLERSPROC) \
    F(glBindSampler,                PFNGLBINDSAMPLERPROC) \
    F(glTexImage3D,                 PFNGLTEXIMAGE3DPROC) \
    F(glSamplerParameteri,          PFNGLSAMPLERPARAMETERIPROC) \
    /* end */

using namespace Slang;

namespace renderer_test {

class GLRenderer : public Renderer, public ShaderCompiler
{
public:
    
    // Renderer    implementation
    virtual SlangResult initialize(const Desc& desc, void* inWindowHandle) override;
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    virtual TextureResource* createTextureResource(Resource::Type type, Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData) override;
    virtual BufferResource* createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData) override;
    virtual SlangResult captureScreenSurface(Surface& surfaceOut) override;
    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) override;
    virtual BindingState* createBindingState(const BindingState::Desc& bindingStateDesc) override;
    virtual ShaderCompiler* getShaderCompiler() override;
    virtual void* map(BufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(BufferResource* buffer) override;
    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual void setBindingState(BindingState* state);
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets) override;
    virtual void setShaderProgram(ShaderProgram* inProgram) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;
    virtual void submitGpuWork() override {}
    virtual void waitForGpu() override {}
    virtual RendererType getRendererType() const override { return RendererType::OpenGl; }

    // ShaderCompiler implementation
    virtual ShaderProgram* compileProgram(const ShaderCompileRequest& request) override;

    protected:
    enum
    {
        kMaxVertexStreams = 16,
    };

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

    class InputLayoutImpl: public InputLayout
    {
		public:
        VertexAttributeDesc m_attributes[kMaxVertexStreams];
        UInt m_attributeCount = 0;
    };

	class BufferResourceImpl: public BufferResource
	{
		public:
        typedef BufferResource Parent;

        BufferResourceImpl(Usage initialUsage, const Desc& desc, GLRenderer* renderer, GLuint id, GLenum target):
            Parent(desc),
			m_renderer(renderer),
			m_handle(id),
            m_initialUsage(initialUsage),
            m_target(target)
		{}
		~BufferResourceImpl()
		{
			if (m_renderer)
			{
				m_renderer->glDeleteBuffers(1, &m_handle);
			}
		}

        Usage m_initialUsage;
		GLRenderer* m_renderer;
		GLuint m_handle;
        GLenum m_target;
	};

    class TextureResourceImpl: public TextureResource
    {
        public:
        typedef TextureResource Parent;

        TextureResourceImpl(Type type, Usage initialUsage, const Desc& desc, GLRenderer* renderer):
            Parent(type, desc),
            m_initialUsage(initialUsage),
            m_renderer(renderer)
        {
            m_target = 0;
            m_handle = 0;
        }

        ~TextureResourceImpl()
        {
            if (m_handle)
            {
                glDeleteTextures(1, &m_handle);
            }
         }   

        Usage m_initialUsage;
        GLRenderer* m_renderer;
        GLenum m_target;
        GLuint m_handle;
    };

    struct BindingDetail
    {
        GLuint m_samplerHandle = 0;
        int m_firstBinding;                      //< Holds binding index if not sampler (which has multiple, and can be read from the BindingState::Desc)
    };

    class BindingStateImpl: public BindingState
    {
		public:
        typedef BindingState Parent;

            /// Ctor
		BindingStateImpl(const Desc& desc, GLRenderer* renderer):
            Parent(desc),
			m_renderer(renderer)
		{
		}

		~BindingStateImpl()
		{
			if (m_renderer)
			{
				m_renderer->destroyBindingEntries(getDesc(), m_bindingDetails.Buffer());
			}
		}

		GLRenderer* m_renderer;
        List<BindingDetail> m_bindingDetails;
    };

	class ShaderProgramImpl : public ShaderProgram
	{
	public:
		ShaderProgramImpl(GLRenderer* renderer, GLuint id):
			m_renderer(renderer),
			m_id(id)
		{
		}
		~ShaderProgramImpl()
		{
			if (m_renderer)
			{
				m_renderer->glDeleteProgram(m_id);
			}
		}
	
		GLuint m_id;
		GLRenderer* m_renderer;
	};

    enum class GlPixelFormat
    {
        Unknown,
        RGBA_Unorm_UInt8,
        CountOf,
    };

    struct GlPixelFormatInfo
    {
        GLint internalFormat;           // such as GL_RGBA8
        GLenum format;                  // such as GL_RGBA
        GLenum formatType;              // such as GL_UNSIGNED_BYTE
    };

	void destroyBindingEntries(const BindingState::Desc& desc, const BindingDetail* details);

    void bindBufferImpl(int target, UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* offsets);
    void flushStateForDraw();
    GLuint loadShader(GLenum stage, char const* source);
    void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message);
    
        /// Returns GlPixelFormat::Unknown if not an equivalent
    static GlPixelFormat _getGlPixelFormat(Format format);

    static void APIENTRY staticDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
    static VertexAttributeFormat getVertexAttributeFormat(Format format);

    static void compileTimeAsserts();

    HDC     m_hdc;
    HGLRC   m_glContext;
    float   m_clearColor[4] = { 0, 0, 0, 0 };
    
	RefPtr<ShaderProgramImpl> m_boundShaderProgram;
    RefPtr<InputLayoutImpl> m_boundInputLayout;

    GLenum m_boundPrimitiveTopology = GL_TRIANGLES;
    GLuint  m_boundVertexStreamBuffers[kMaxVertexStreams];
    UInt    m_boundVertexStreamStrides[kMaxVertexStreams];
    UInt    m_boundVertexStreamOffsets[kMaxVertexStreams];

    Desc m_desc;

    // Declare a function pointer for each OpenGL
    // extension function we need to load
#define DECLARE_GL_EXTENSION_FUNC(NAME, TYPE) TYPE NAME;
    MAP_GL_EXTENSION_FUNCS(DECLARE_GL_EXTENSION_FUNC)
#undef DECLARE_GL_EXTENSION_FUNC

    static const GlPixelFormatInfo s_pixelFormatInfos[];            /// Maps GlPixelFormat to a format info 
};

/* static */GLRenderer::GlPixelFormat GLRenderer::_getGlPixelFormat(Format format)
{
    switch (format)
    {
        case Format::RGBA_Unorm_UInt8:      return GlPixelFormat::RGBA_Unorm_UInt8;
        default:                            return GlPixelFormat::Unknown;
    }
}

/* static */ const GLRenderer::GlPixelFormatInfo GLRenderer::s_pixelFormatInfos[] = 
{
    // internalType, format, formatType
    { 0,                0,          0},                         // GlPixelFormat::Unknown
    { GL_RGBA8,         GL_RGBA,    GL_UNSIGNED_BYTE },         // GlPixelFormat::RGBA_Unorm_UInt8
};

/* static */void GLRenderer::compileTimeAsserts()
{
    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(s_pixelFormatInfos) == int(GlPixelFormat::CountOf));
}

Renderer* createGLRenderer()
{
    return new GLRenderer();
}

void GLRenderer::debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message)
{
    ::OutputDebugStringA("GL: ");
    ::OutputDebugStringA(message);
    ::OutputDebugStringA("\n");

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            break;
        default:
            break;
    }
}

/* static */void APIENTRY GLRenderer::staticDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    ((GLRenderer*)userParam)->debugCallback(source, type, id, severity, length, message);
}

/* static */GLRenderer::VertexAttributeFormat GLRenderer::getVertexAttributeFormat(Format format)
{
    switch (format)
    {
        default: assert(!"unexpected"); return VertexAttributeFormat();

#define CASE(NAME, COUNT, TYPE, NORMALIZED) \
        case Format::NAME: do { VertexAttributeFormat result = {COUNT, TYPE, NORMALIZED}; return result; } while (0)

        CASE(RGBA_Float32, 4, GL_FLOAT, GL_FALSE);
        CASE(RGB_Float32, 3, GL_FLOAT, GL_FALSE);
        CASE(RG_Float32, 2, GL_FLOAT, GL_FALSE);
        CASE(R_Float32, 1, GL_FLOAT, GL_FALSE);
#undef CASE
    }
}

void GLRenderer::bindBufferImpl(int target, UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* offsets)
{
    for (UInt ii = 0; ii < slotCount; ++ii)
    {
        UInt slot = startSlot + ii;

        BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(buffers[ii]);
        GLuint bufferID = buffer ? buffer->m_handle : 0;
		
        assert(!offsets || !offsets[ii]);

        glBindBufferBase(target, (GLuint)slot, bufferID);
    }
}

void GLRenderer::flushStateForDraw()
{
    auto layout = m_boundInputLayout.Ptr();
    auto attrCount = layout->m_attributeCount;
    for (UInt ii = 0; ii < attrCount; ++ii)
    {
        auto& attr = layout->m_attributes[ii];

        auto streamIndex = attr.streamIndex;

        glBindBuffer(GL_ARRAY_BUFFER, m_boundVertexStreamBuffers[streamIndex]);

        glVertexAttribPointer(
            (GLuint)ii,
            attr.format.componentCount,
            attr.format.componentType,
            attr.format.normalized,
            (GLsizei)m_boundVertexStreamStrides[streamIndex],
            (GLvoid*)(attr.offset + m_boundVertexStreamOffsets[streamIndex]));

        glEnableVertexAttribArray((GLuint)ii);
    }
    for (UInt ii = attrCount; ii < kMaxVertexStreams; ++ii)
    {
        glDisableVertexAttribArray((GLuint)ii);
    }
}

GLuint GLRenderer::loadShader(GLenum stage, const char* source)
{
    // GLSL is monumentally stupid. It officially requires the `#version` directive
    // to be the first thing in the file, which wouldn't be so bad but the API
    // doesn't provide a way to pass a `#define` into your shader other than by
    // prepending it to the whole thing.
    //
    // We are going to solve this problem by doing some surgery on the source
    // that was passed in.

    const char* sourceBegin = source;
    const char* sourceEnd = source + strlen(source);

    // Look for a version directive in the user-provided source.
    const char* versionBegin = strstr(source, "#version");
    const char* versionEnd = nullptr;
    if (versionBegin)
    {
        // If we found a directive, then scan for the end-of-line
        // after it, and use that to specify the slice.
        versionEnd = strchr(versionBegin, '\n');
        if (!versionEnd)
        {
            versionEnd = sourceEnd;
        }
        else
        {
            versionEnd = versionEnd + 1;
        }
    }
    else
    {
        // If we didn't find a directive, then treat it as being
        // a zero-byte slice at the start of the string
        versionBegin = sourceBegin;
        versionEnd = sourceBegin;
    }

    enum { kMaxSourceStringCount = 16 };
    const GLchar* sourceStrings[kMaxSourceStringCount];
    GLint sourceStringLengths[kMaxSourceStringCount];

    int sourceStringCount = 0;

    const char* stagePrelude = "\n";
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

    const char* prelude =
        "#define __GLSL__ 1\n"
        ;

#define ADD_SOURCE_STRING_SPAN(BEGIN, END)                              \
        sourceStrings[sourceStringCount] = BEGIN;                       \
        sourceStringLengths[sourceStringCount++] = GLint(END - BEGIN)   \
        /* end */

#define ADD_SOURCE_STRING(BEGIN)                                        \
        sourceStrings[sourceStringCount] = BEGIN;                       \
        sourceStringLengths[sourceStringCount++] = GLint(strlen(BEGIN)) \
        /* end */

    ADD_SOURCE_STRING_SPAN(versionBegin, versionEnd);
    ADD_SOURCE_STRING(stagePrelude);
    ADD_SOURCE_STRING(prelude);
    ADD_SOURCE_STRING_SPAN(sourceBegin, versionBegin);
    ADD_SOURCE_STRING_SPAN(versionEnd, sourceEnd);

    auto shaderID = glCreateShader(stage);
    glShaderSource(
        shaderID,
        sourceStringCount,
        &sourceStrings[0],
        &sourceStringLengths[0]);
    glCompileShader(shaderID);

    GLint success = GL_FALSE;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        int maxSize = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxSize);

        auto infoBuffer = (char*)malloc(maxSize);

        int infoSize = 0;
        glGetShaderInfoLog(shaderID, maxSize, &infoSize, infoBuffer);
        if (infoSize > 0)
        {
            fprintf(stderr, "%s", infoBuffer);
            ::OutputDebugStringA(infoBuffer);
        }

        glDeleteShader(shaderID);
        return 0;
    }

    return shaderID;
}

void GLRenderer::destroyBindingEntries(const BindingState::Desc& desc, const BindingDetail* details)
{
    const auto& bindings = desc.m_bindings;
    const int numBindings = int(bindings.Count());
	for (int i = 0; i < numBindings; ++i)
	{
        const auto& binding = bindings[i];
        const auto& detail = details[i];

        if (binding.bindingType == BindingType::Sampler && detail.m_samplerHandle != 0)
        {
            glDeleteSamplers(1, &detail.m_samplerHandle);
        }
	}
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult GLRenderer::initialize(const Desc& desc, void* inWindowHandle)
{
    auto windowHandle = (HWND)inWindowHandle;
    m_desc = desc;

    m_hdc = ::GetDC(windowHandle);

    PIXELFORMATDESCRIPTOR pixelFormatDesc = { sizeof(PIXELFORMATDESCRIPTOR) };
    pixelFormatDesc.nVersion = 1;
    pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDesc.cColorBits = 32;
    pixelFormatDesc.cDepthBits = 24;
    pixelFormatDesc.cStencilBits = 8;
    pixelFormatDesc.iLayerType = PFD_MAIN_PLANE;

    int pixelFormatIndex = ChoosePixelFormat(m_hdc, &pixelFormatDesc);
    SetPixelFormat(m_hdc, pixelFormatIndex, &pixelFormatDesc);

    m_glContext = wglCreateContext(m_hdc);
    wglMakeCurrent(m_hdc, m_glContext);

    auto renderer = glGetString(GL_RENDERER);
    auto extensions = glGetString(GL_EXTENSIONS);

    // Load each of our extension functions by name

#define LOAD_GL_EXTENSION_FUNC(NAME, TYPE) NAME = (TYPE) wglGetProcAddress(#NAME);
    MAP_GL_EXTENSION_FUNCS(LOAD_GL_EXTENSION_FUNC)
#undef LOAD_GL_EXTENSION_FUNC

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, desc.width, desc.height);

    if (glDebugMessageCallback)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(staticDebugCallback, this);
    }

    return SLANG_OK;
}

void GLRenderer::setClearColor(const float color[4])
{
    glClearColor(color[0], color[1], color[2], color[3]);
}

void GLRenderer::clearFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GLRenderer::presentFrame()
{
    glFlush();
    ::SwapBuffers(m_hdc);
}

SlangResult GLRenderer::captureScreenSurface(Surface& surfaceOut)
{
    SLANG_RETURN_ON_FAIL(surfaceOut.allocate(m_desc.width, m_desc.height, Format::RGBA_Unorm_UInt8, 1, SurfaceAllocator::getMallocAllocator()));
    glReadPixels(0, 0, m_desc.width, m_desc.height, GL_RGBA, GL_UNSIGNED_BYTE, surfaceOut.m_data);
    surfaceOut.flipInplaceVertically();
    return SLANG_OK;
}

ShaderCompiler* GLRenderer::getShaderCompiler()
{
    return this;
}

TextureResource* GLRenderer::createTextureResource(Resource::Type type, Resource::Usage initialUsage, const TextureResource::Desc& descIn, const TextureResource::Data* initData)
{
    TextureResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(type, initialUsage);

    GlPixelFormat pixelFormat = _getGlPixelFormat(srcDesc.format);
    if (pixelFormat == GlPixelFormat::Unknown)
    {
        return nullptr;
    }

    const GlPixelFormatInfo& info = s_pixelFormatInfos[int(pixelFormat)];

    const GLint internalFormat = info.internalFormat;
    const GLenum format = info.format;
    const GLenum formatType = info.formatType;
    
    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(type, initialUsage, srcDesc, this));

    GLenum target = 0;
    GLuint handle = 0;
    glGenTextures(1, &handle);

    const int effectiveArraySize = srcDesc.calcEffectiveArraySize(type);

    assert(initData);
    assert(initData->numSubResources == srcDesc.numMipLevels * srcDesc.size.depth * effectiveArraySize);

    // Set on texture so will be freed if failure
    texture->m_handle = handle;
    const void*const*const data = initData->subResources;

    switch (type)
    {
        case Resource::Type::Texture1D:
        {
            if (srcDesc.arraySize > 0)
            {
                target = GL_TEXTURE_1D_ARRAY;
                glBindTexture(target, handle);

                int slice = 0;
                for (int i = 0; i < effectiveArraySize; i++)
                {
                    for (int j = 0; j < srcDesc.numMipLevels; j++)
                    {
                        glTexImage2D(target, j, internalFormat, srcDesc.size.width, i, 0, format, formatType, data[slice++]);
                    }
                }
            }
            else
            {
                target = GL_TEXTURE_1D;
                glBindTexture(target, handle);
                for (int i = 0; i < srcDesc.numMipLevels; i++)
                {
                    glTexImage1D(target, i, internalFormat, srcDesc.size.width, 0, format, formatType, data[i]);
                }
            }
            break;
        }
        case Resource::Type::TextureCube:
        case Resource::Type::Texture2D:
        {
            if (srcDesc.arraySize > 0)
            {
                if (type == Resource::Type::TextureCube)
                {
                    target = GL_TEXTURE_CUBE_MAP_ARRAY;
                }
                else
                {
                    target = GL_TEXTURE_2D_ARRAY;
                }

                glBindTexture(target, handle);
                
                int slice = 0;
                for (int i = 0; i < effectiveArraySize; i++)
                {
                    for (int j = 0; j < srcDesc.numMipLevels; j++)
                    {
                        glTexImage3D(target, j, internalFormat, srcDesc.size.width, srcDesc.size.height, slice, 0, format, formatType, data[slice++]);
                    }
                }
            }
            else
            {
                if (type == Resource::Type::TextureCube)
                {
                    target = GL_TEXTURE_CUBE_MAP;
                    glBindTexture(target, handle);
                    
                    int slice = 0;
                    for (int j = 0; j < 6; j++)
                    {
                        for (int i = 0; i < srcDesc.numMipLevels; i++)
                        {
                            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, internalFormat, srcDesc.size.width, srcDesc.size.height, 0, format, formatType, data[slice++]);
                        }
                    }
                }
                else
                {
                    target = GL_TEXTURE_2D;
                    glBindTexture(target, handle);
                    for (int i = 0; i < srcDesc.numMipLevels; i++)
                    {
                        glTexImage2D(target, i, internalFormat, srcDesc.size.width, srcDesc.size.height, 0, format, formatType, data[i]);
                    }
                }
            }
            break;
        }
        case Resource::Type::Texture3D:
        {
            target = GL_TEXTURE_3D;
            glBindTexture(target, handle);
            for (int i = 0; i < srcDesc.numMipLevels; i++)
            {
                glTexImage3D(target, i, internalFormat, srcDesc.size.width, srcDesc.size.height, srcDesc.size.depth, 0, format, formatType, data[i]);
            }
            break;
        }
        default: return nullptr;
    }

    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);
    
    // Assume regular sampling (might be superseded - if a combined sampler wanted)
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);

    texture->m_target = target;

    return texture.detach();
}

static GLenum _calcUsage(Resource::Usage usage)
{
    typedef Resource::Usage Usage;
    switch (usage)
    {
        case Usage::ConstantBuffer:     return GL_DYNAMIC_DRAW;
        default:                        return GL_STATIC_READ;
    }
}

static GLenum _calcTarget(Resource::Usage usage)
{
    typedef Resource::Usage Usage;
    switch (usage)
    {
        case Usage::ConstantBuffer:     return GL_UNIFORM_BUFFER;
        default:                        return GL_SHADER_STORAGE_BUFFER;
    }
}

BufferResource* GLRenderer::createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData) 
{
    BufferResource::Desc desc(descIn);

    if (desc.bindFlags == 0)
    {
        desc.bindFlags = Resource::s_requiredBinding[int(initialUsage)];
    }

    const GLenum target = _calcTarget(initialUsage);
    // TODO: should derive from desc...
    const GLenum usage = _calcUsage(initialUsage);
    
    GLuint bufferID = 0;
    glGenBuffers(1, &bufferID);
    glBindBuffer(target, bufferID);

    glBufferData(target, descIn.sizeInBytes, initData, usage);

	return new BufferResourceImpl(initialUsage, desc, this, bufferID, target);
}

InputLayout* GLRenderer::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount)
{
    InputLayoutImpl* inputLayout = new InputLayoutImpl;

    inputLayout->m_attributeCount = inputElementCount;
    for (UInt ii = 0; ii < inputElementCount; ++ii)
    {
        auto& inputAttr = inputElements[ii];
        auto& glAttr = inputLayout->m_attributes[ii];

        glAttr.streamIndex = 0;
        glAttr.format = getVertexAttributeFormat(inputAttr.format);
        glAttr.offset = (GLsizei)inputAttr.offset;
    }

    return (InputLayout*)inputLayout;
}

void* GLRenderer::map(BufferResource* bufferIn, MapFlavor flavor)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);

    //GLenum target = GL_UNIFORM_BUFFER;

    GLuint access = 0;
    switch (flavor)
    {
        case MapFlavor::WriteDiscard:
        case MapFlavor::HostWrite:
            access = GL_WRITE_ONLY;
            break;
        case MapFlavor::HostRead:
            access = GL_READ_ONLY;
            break;
    }

    glBindBuffer(buffer->m_target, buffer->m_handle);

    return glMapBuffer(buffer->m_target, access);
}

void GLRenderer::unmap(BufferResource* bufferIn)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);
    glUnmapBuffer(buffer->m_target);
}

void GLRenderer::setInputLayout(InputLayout* inputLayout)
{
    m_boundInputLayout = static_cast<InputLayoutImpl*>(inputLayout);
}

void GLRenderer::setPrimitiveTopology(PrimitiveTopology topology)
{
    GLenum glTopology = 0;
    switch (topology)
    {
#define CASE(NAME, VALUE) case PrimitiveTopology::NAME: glTopology = VALUE; break

        CASE(TriangleList, GL_TRIANGLES);

#undef CASE
    }
    m_boundPrimitiveTopology = glTopology;
}

void GLRenderer::setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets)
{
    for (UInt ii = 0; ii < slotCount; ++ii)
    {
        UInt slot = startSlot + ii;

        BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(buffers[ii]);
        GLuint bufferID = buffer ? buffer->m_handle : 0;
		
        m_boundVertexStreamBuffers[slot] = bufferID;
        m_boundVertexStreamStrides[slot] = strides[ii];
        m_boundVertexStreamOffsets[slot] = offsets[ii];
    }
}

void GLRenderer::setShaderProgram(ShaderProgram* programIn)
{
	ShaderProgramImpl* program = static_cast<ShaderProgramImpl*>(programIn);
	m_boundShaderProgram = program;
    GLuint programID = program ? program->m_id : 0;
	glUseProgram(programID);
}

void GLRenderer::draw(UInt vertexCount, UInt startVertex = 0) 
{
    flushStateForDraw();

    glDrawArrays(m_boundPrimitiveTopology, (GLint)startVertex, (GLsizei)vertexCount);
}

void GLRenderer::dispatchCompute(int x, int y, int z)
{
    glDispatchCompute(x, y, z);
}

BindingState* GLRenderer::createBindingState(const BindingState::Desc& bindingStateDesc)
{
    RefPtr<BindingStateImpl> bindingState(new BindingStateImpl(bindingStateDesc, this));

    const auto& srcBindings = bindingStateDesc.m_bindings;
    const int numBindings = int(srcBindings.Count());

    auto& dstDetails = bindingState->m_bindingDetails;
    dstDetails.SetSize(numBindings);

    for (int i = 0; i < numBindings; ++i)
    {
        auto& dstDetail = dstDetails[i];
        const auto& srcBinding = srcBindings[i];

        // Copy over the bindings 
        dstDetail.m_firstBinding = bindingStateDesc.getFirst(BindingState::ShaderStyle::Glsl, srcBinding.shaderBindSet);
        
        switch (srcBinding.bindingType)
        {
            case BindingType::Texture:
            case BindingType::Buffer:
            {
                break;
            }
            case BindingType::CombinedTextureSampler:
            {
                assert(srcBinding.resource && srcBinding.resource->isTexture());
                TextureResourceImpl* texture = static_cast<TextureResourceImpl*>(srcBinding.resource.Ptr());
                const BindingState::SamplerDesc& samplerDesc = bindingStateDesc.m_samplerDescs[srcBinding.descIndex];

                if (samplerDesc.isCompareSampler)
                {
                    auto target = texture->m_target;

                    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                    glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                }
                break;
            }
            case BindingType::Sampler:
            {
                const BindingState::SamplerDesc& samplerDesc = bindingStateDesc.m_samplerDescs[srcBinding.descIndex];

                GLuint handle;

                glCreateSamplers(1, &handle);
                glSamplerParameteri(handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glSamplerParameteri(handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glSamplerParameteri(handle, GL_TEXTURE_WRAP_R, GL_REPEAT);

                if (samplerDesc.isCompareSampler)
                {
                    glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glSamplerParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glSamplerParameteri(handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                    glSamplerParameteri(handle, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                }
                else
                {
                    glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glSamplerParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glSamplerParameteri(handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
                }

                dstDetail.m_samplerHandle = handle;
                break;
            }
        }
    }

    return bindingState.detach();
}

void GLRenderer::setBindingState(BindingState* stateIn)
{
    BindingStateImpl* state = static_cast<BindingStateImpl*>(stateIn);

    const auto& bindingDesc = state->getDesc();

    const auto& details = state->m_bindingDetails;
    const auto& bindings = bindingDesc.m_bindings;
    const int numBindings = int(bindings.Count());

    for (int i = 0; i < numBindings; ++i)
    {
        const auto& binding = bindings[i];
        const auto& detail = details[i];

        switch (binding.bindingType)
        {
            case BindingType::Buffer:
            {
                BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(binding.resource.Ptr());
                glBindBufferBase(buffer->m_target, detail.m_firstBinding, buffer->m_handle);
                break;
            }
            case BindingType::Sampler:
            {
                auto bindings = bindingDesc.asSlice(BindingState::ShaderStyle::Glsl, binding.shaderBindSet);
                for (auto b : bindings)
                {
                    glBindSampler(b, detail.m_samplerHandle);
                }
                break;
            }
            case BindingType::Texture:
            case BindingType::CombinedTextureSampler:
            {
                BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(binding.resource.Ptr());

                glActiveTexture(GL_TEXTURE0 + detail.m_firstBinding);
                glBindTexture(buffer->m_target, buffer->m_handle);
                break;
            }
        }
    }
}

// ShaderCompiler interface

ShaderProgram* GLRenderer::compileProgram(const ShaderCompileRequest& request) 
{
    auto programID = glCreateProgram();
    if (request.computeShader.name)
    {
        auto computeShaderID = loadShader(GL_COMPUTE_SHADER, request.computeShader.source.dataBegin);
        glAttachShader(programID, computeShaderID);
        glLinkProgram(programID);
        glDeleteShader(computeShaderID);
    }
    else
    {
        auto vertexShaderID = loadShader(GL_VERTEX_SHADER, request.vertexShader.source.dataBegin);
        auto fragmentShaderID = loadShader(GL_FRAGMENT_SHADER, request.fragmentShader.source.dataBegin);

        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);


        glLinkProgram(programID);

        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
    }
    GLint success = GL_FALSE;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        int maxSize = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxSize);

        auto infoBuffer = (char*)::malloc(maxSize);

        int infoSize = 0;
        glGetProgramInfoLog(programID, maxSize, &infoSize, infoBuffer);
        if (infoSize > 0)
        {
            fprintf(stderr, "%s", infoBuffer);
            OutputDebugStringA(infoBuffer);
        }

        ::free(infoBuffer);

        glDeleteProgram(programID);
        return nullptr;
    }

    return new ShaderProgramImpl(this, programID);
}


} // renderer_test

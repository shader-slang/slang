// render-gl.cpp
#include "render-gl.h"

#include "../nvapi/nvapi-util.h"

//WORKING:#include "options.h"
#include "../render.h"
#include "../render-graphics-common.h"

#include <stdio.h>
#include <stdlib.h>
#include "core/slang-basic.h"
#include "core/slang-secure-crt.h"
#include "external/stb/stb_image_write.h"

#include "../surface.h"

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

namespace gfx {

class GLRenderer : public GraphicsAPIRenderer
{
public:

    // Renderer    implementation
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc, void* inWindowHandle) override;
    virtual SLANG_NO_THROW const List<String>& SLANG_MCALL getFeatures() override
    {
        return m_features;
    }
    virtual SLANG_NO_THROW void SLANG_MCALL setClearColor(const float color[4]) override;
    virtual SLANG_NO_THROW void SLANG_MCALL clearFrame() override;
    virtual SLANG_NO_THROW void SLANG_MCALL presentFrame() override;
    virtual SLANG_NO_THROW TextureResource::Desc SLANG_MCALL getSwapChainTextureDesc() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        Resource::Usage initialUsage,
        const TextureResource::Desc& desc,
        const TextureResource::Data* initData,
        TextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        Resource::Usage initialUsage,
        const BufferResource::Desc& desc,
        const void* initData,
        BufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(SamplerState::Desc const& desc, SamplerState** outSampler) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        TextureResource* texture, ResourceView::Desc const& desc, ResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        BufferResource* buffer, ResourceView::Desc const& desc, ResourceView** outView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        InputLayout** outLayout) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const DescriptorSetLayout::Desc& desc, DescriptorSetLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createPipelineLayout(const PipelineLayout::Desc& desc, PipelineLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createDescriptorSet(DescriptorSetLayout* layout, DescriptorSet** outDescriptorSet) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const ShaderProgram::Desc& desc, ShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, PipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, PipelineState** outState) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
        captureScreenSurface(Surface& surfaceOut) override;

    virtual SLANG_NO_THROW void* SLANG_MCALL map(BufferResource* buffer, MapFlavor flavor) override;
    virtual SLANG_NO_THROW void SLANG_MCALL unmap(BufferResource* buffer) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
        PipelineType pipelineType,
        PipelineLayout* layout,
        UInt index,
        DescriptorSet* descriptorSet) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        BufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(BufferResource* buffer, Format indexFormat, UInt offset) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setDepthStencilTarget(ResourceView* depthStencilView) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(UInt count, Viewport const* viewports) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(UInt count, ScissorRect const* rects) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setPipelineState(PipelineType pipelineType, PipelineState* state) override;
    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex) override;
    virtual void SLANG_MCALL
        drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;
    virtual SLANG_NO_THROW void SLANG_MCALL submitGpuWork() override {}
    virtual SLANG_NO_THROW void SLANG_MCALL waitForGpu() override {}
    virtual SLANG_NO_THROW RendererType SLANG_MCALL getRendererType() const override
    {
        return RendererType::OpenGl;
    }

    GLRenderer();
    ~GLRenderer();

    protected:
    enum
    {
        kMaxVertexStreams = 16,
        kMaxDescriptorSetCount = 8,
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

        BufferResourceImpl(Usage initialUsage, const Desc& desc, WeakSink<GLRenderer>* renderer, GLuint id, GLenum target):
            Parent(desc),
			m_renderer(renderer),
			m_handle(id),
            m_initialUsage(initialUsage),
            m_target(target)
		{}
		~BufferResourceImpl()
		{
			if (auto renderer = m_renderer->get())
			{
				renderer->glDeleteBuffers(1, &m_handle);
			}
		}

        Usage m_initialUsage;
		RefPtr<WeakSink<GLRenderer> > m_renderer;
		GLuint m_handle;
        GLenum m_target;
	};

    class TextureResourceImpl: public TextureResource
    {
        public:
        typedef TextureResource Parent;

        TextureResourceImpl(Usage initialUsage, const Desc& desc, WeakSink<GLRenderer>* renderer):
            Parent(desc),
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
        RefPtr<WeakSink<GLRenderer> > m_renderer;
        GLenum m_target;
        GLuint m_handle;
    };

    class SamplerStateImpl : public SamplerState
    {
    public:
        GLuint m_samplerID;
    };

    class ResourceViewImpl : public ResourceView
    {
    };

    class TextureViewImpl : public ResourceViewImpl
    {
    public:
        RefPtr<TextureResourceImpl> m_resource;
        GLuint                      m_textureID;
    };

    class BufferViewImpl : public ResourceViewImpl
    {
    public:
        RefPtr<BufferResourceImpl>  m_resource;
        GLuint                      m_bufferID;
    };

    enum class GLDescriptorSlotType
    {
        ConstantBuffer,
        CombinedTextureSampler,

        CountOf,
    };

    class DescriptorSetLayoutImpl : public DescriptorSetLayout
    {
    public:
        struct RangeInfo
        {
            GLDescriptorSlotType    type;
            UInt                    arrayIndex;
        };
        List<RangeInfo> m_ranges;
        Int             m_counts[int(GLDescriptorSlotType::CountOf)];
    };

    class PipelineLayoutImpl : public PipelineLayout
    {
    public:
        struct DescriptorSetInfo
        {
            RefPtr<DescriptorSetLayoutImpl> layout;
            UInt                            baseArrayIndex[int(GLDescriptorSlotType::CountOf)];
        };

        List<DescriptorSetInfo> m_sets;
    };

    class DescriptorSetImpl : public DescriptorSet
    {
    public:
        virtual void setConstantBuffer(UInt range, UInt index, BufferResource* buffer) override;
        virtual void setResource(UInt range, UInt index, ResourceView* view) override;
        virtual void setSampler(UInt range, UInt index, SamplerState* sampler) override;
        virtual void setCombinedTextureSampler(
            UInt range,
            UInt index,
            ResourceView*   textureView,
            SamplerState*   sampler) override;
        virtual void setRootConstants(
            UInt range,
            UInt offset,
            UInt size,
            void const* data) override;

        RefPtr<DescriptorSetLayoutImpl>     m_layout;
        List<RefPtr<BufferResourceImpl>>    m_constantBuffers;
        List<RefPtr<TextureViewImpl>>       m_textures;
        List<RefPtr<SamplerStateImpl>>      m_samplers;
    };

	class ShaderProgramImpl : public ShaderProgram
	{
	public:
		ShaderProgramImpl(WeakSink<GLRenderer>* renderer, GLuint id):
			m_renderer(renderer),
			m_id(id)
		{
		}
		~ShaderProgramImpl()
		{
			if (auto renderer = m_renderer->get())
			{
				renderer->glDeleteProgram(m_id);
			}
		}

		GLuint m_id;
		RefPtr<WeakSink<GLRenderer> > m_renderer;
	};

    class PipelineStateImpl : public PipelineState
    {
    public:
        RefPtr<ShaderProgramImpl>   m_program;
        RefPtr<PipelineLayoutImpl>  m_pipelineLayout;
        RefPtr<InputLayoutImpl>     m_inputLayout;
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

//	void destroyBindingEntries(const BindingState::Desc& desc, const BindingDetail* details);

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

    RefPtr<PipelineStateImpl> m_currentPipelineState;
    RefPtr<WeakSink<GLRenderer> > m_weakRenderer;

    RefPtr<DescriptorSetImpl>   m_boundDescriptorSets[kMaxDescriptorSetCount];

    GLenum m_boundPrimitiveTopology = GL_TRIANGLES;
    GLuint  m_boundVertexStreamBuffers[kMaxVertexStreams];
    UInt    m_boundVertexStreamStrides[kMaxVertexStreams];
    UInt    m_boundVertexStreamOffsets[kMaxVertexStreams];

    Desc m_desc;

    List<String> m_features;

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

SlangResult createGLRenderer(IRenderer** outRenderer)
{
    *outRenderer = new GLRenderer();
    (*outRenderer)->addRef();
    return SLANG_OK;
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


GLRenderer::GLRenderer()
{
    m_weakRenderer = new WeakSink<GLRenderer>(this);
}

GLRenderer::~GLRenderer()
{
    // We can destroy things whilst in this state
    m_currentPipelineState.setNull();

    // By resetting the weak pointer, other objects accessing through WeakSink<GLRenderer> will no longer
    // be able to access this object which is entering a 'being destroyed' to 'destroyed' state
    if (m_weakRenderer)
    {
        SLANG_ASSERT(m_weakRenderer->get() == this);
        m_weakRenderer->detach();
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
    auto inputLayout = m_currentPipelineState->m_inputLayout.Ptr();
    auto attrCount = Index(inputLayout->m_attributeCount);
    for (Index ii = 0; ii < attrCount; ++ii)
    {
        auto& attr = inputLayout->m_attributes[ii];

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
    for (Index ii = attrCount; ii < kMaxVertexStreams; ++ii)
    {
        glDisableVertexAttribArray((GLuint)ii);
    }

    // Next bind the descriptor sets as required by the layout
    auto pipelineLayout = m_currentPipelineState->m_pipelineLayout;
    auto descriptorSetCount = pipelineLayout->m_sets.getCount();
    for(Index ii = 0; ii < descriptorSetCount; ++ii)
    {
        auto descriptorSet = m_boundDescriptorSets[ii];
        auto descriptorSetInfo = pipelineLayout->m_sets[ii];
        auto descriptorSetLayout = descriptorSetInfo.layout;

        // TODO: need to validate that `descriptorSet->m_layout` matches
        // `descriptorSetLayout`.

        {
            // First we will bind any uniform buffers that were specified.

            auto slotTypeIndex = int(GLDescriptorSlotType::ConstantBuffer);
            auto count = descriptorSetLayout->m_counts[slotTypeIndex];
            auto baseIndex = descriptorSetInfo.baseArrayIndex[slotTypeIndex];

            for(Int ii = 0; ii < count; ++ii)
            {
                auto bufferImpl = descriptorSet->m_constantBuffers[ii];
                glBindBufferBase(GL_UNIFORM_BUFFER, GLuint(ii), bufferImpl->m_handle);
            }
        }


        {
            // Next we will bind any combined texture/sampler slots.

            auto slotTypeIndex = int(GLDescriptorSlotType::CombinedTextureSampler);
            auto count = descriptorSetLayout->m_counts[slotTypeIndex];
            auto baseIndex = descriptorSetInfo.baseArrayIndex[slotTypeIndex];

            // TODO: We should be able to use a single call to glBindTextures here,
            // rather than a loop. This would also eliminate the need to retain
            // the appropriate target (e.g., `GL_TEXTURE_2D` for binding).

            for(Int ii = 0; ii < count; ++ii)
            {
                auto textureViewImpl = descriptorSet->m_textures[ii];
                auto samplerImpl = descriptorSet->m_samplers[ii];

                glActiveTexture(GLuint(GL_TEXTURE0 + ii));
                glBindTexture(GL_TEXTURE_2D, textureViewImpl->m_textureID);

                glBindSampler(GLuint(baseIndex + ii), samplerImpl->m_samplerID);
            }
        }
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

#if 0
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
#endif

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

    if (renderer && desc.adapter.getLength() > 0)
    {
        String lowerAdapter = desc.adapter.toLower();
        String lowerRenderer = String((const char*)renderer).toLower();

        // The adapter is not available
        if (lowerRenderer.indexOf(lowerAdapter) == Index(-1))
        {
            return SLANG_E_NOT_AVAILABLE;
        }
    }

    if (m_desc.nvapiExtnSlot >= 0)
    {
        if (SLANG_FAILED(NVAPIUtil::initialize()))
        {
            return SLANG_E_NOT_AVAILABLE;
        }
    }

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

TextureResource::Desc GLRenderer::getSwapChainTextureDesc()
{
    TextureResource::Desc desc;
    desc.init2D(Resource::Type::Texture2D, Format::Unknown, m_desc.width, m_desc.height, 1);
    return desc;
}

SlangResult GLRenderer::captureScreenSurface(Surface& surfaceOut)
{
    SLANG_RETURN_ON_FAIL(surfaceOut.allocate(m_desc.width, m_desc.height, Format::RGBA_Unorm_UInt8, 1, SurfaceAllocator::getMallocAllocator()));
    glReadPixels(0, 0, m_desc.width, m_desc.height, GL_RGBA, GL_UNSIGNED_BYTE, surfaceOut.m_data);
    surfaceOut.flipInplaceVertically();
    return SLANG_OK;
}

Result GLRenderer::createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& descIn, const TextureResource::Data* initData, TextureResource** outResource)
{
    TextureResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    GlPixelFormat pixelFormat = _getGlPixelFormat(srcDesc.format);
    if (pixelFormat == GlPixelFormat::Unknown)
    {
        return SLANG_FAIL;
    }

    const GlPixelFormatInfo& info = s_pixelFormatInfos[int(pixelFormat)];

    const GLint internalFormat = info.internalFormat;
    const GLenum format = info.format;
    const GLenum formatType = info.formatType;

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(initialUsage, srcDesc, m_weakRenderer));

    GLenum target = 0;
    GLuint handle = 0;
    glGenTextures(1, &handle);

    const int effectiveArraySize = srcDesc.calcEffectiveArraySize();

    assert(initData);
    assert(initData->numSubResources == srcDesc.numMipLevels * srcDesc.size.depth * effectiveArraySize);

    // Set on texture so will be freed if failure
    texture->m_handle = handle;
    const void*const*const data = initData->subResources;

    switch (srcDesc.type)
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
                if (srcDesc.type == Resource::Type::TextureCube)
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
                if (srcDesc.type == Resource::Type::TextureCube)
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
        default:
            return SLANG_FAIL;
    }

    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);

    // Assume regular sampling (might be superseded - if a combined sampler wanted)
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);

    texture->m_target = target;

    *outResource = texture.detach();
    return SLANG_OK;
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

Result GLRenderer::createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData, BufferResource** outResource)
{
    BufferResource::Desc desc(descIn);
    desc.setDefaults(initialUsage);

    const GLenum target = _calcTarget(initialUsage);
    // TODO: should derive from desc...
    const GLenum usage = _calcUsage(initialUsage);

    GLuint bufferID = 0;
    glGenBuffers(1, &bufferID);
    glBindBuffer(target, bufferID);

    glBufferData(target, descIn.sizeInBytes, initData, usage);

    RefPtr<BufferResourceImpl> resourceImpl = new BufferResourceImpl(initialUsage, desc, m_weakRenderer, bufferID, target);
    *outResource = resourceImpl.detach();
    return SLANG_OK;
}

Result GLRenderer::createSamplerState(SamplerState::Desc const& desc, SamplerState** outSampler)
{
    GLuint samplerID;
    glCreateSamplers(1, &samplerID);

    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_samplerID = samplerID;
    *outSampler = samplerImpl.detach();
    return SLANG_OK;
}

Result GLRenderer::createTextureView(TextureResource* texture, ResourceView::Desc const& desc, ResourceView** outView)
{
    auto resourceImpl = (TextureResourceImpl*) texture;

    // TODO: actually do something?

    RefPtr<TextureViewImpl> viewImpl = new TextureViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_textureID = resourceImpl->m_handle;
    *outView = viewImpl;
    return SLANG_OK;
}

Result GLRenderer::createBufferView(BufferResource* buffer, ResourceView::Desc const& desc, ResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;

    // TODO: actually do something?

    RefPtr<BufferViewImpl> viewImpl = new BufferViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_bufferID = resourceImpl->m_handle;
    *outView = viewImpl.detach();
    return SLANG_OK;
}

Result GLRenderer::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount, InputLayout** outLayout)
{
    RefPtr<InputLayoutImpl> inputLayout = new InputLayoutImpl;

    inputLayout->m_attributeCount = inputElementCount;
    for (UInt ii = 0; ii < inputElementCount; ++ii)
    {
        auto& inputAttr = inputElements[ii];
        auto& glAttr = inputLayout->m_attributes[ii];

        glAttr.streamIndex = 0;
        glAttr.format = getVertexAttributeFormat(inputAttr.format);
        glAttr.offset = (GLsizei)inputAttr.offset;
    }

    *outLayout = inputLayout.detach();
    return SLANG_OK;
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

void GLRenderer::setIndexBuffer(BufferResource* buffer, Format indexFormat, UInt offset)
{
}

void GLRenderer::setDepthStencilTarget(ResourceView* depthStencilView)
{
}

void GLRenderer::setViewports(UInt count, Viewport const* viewports)
{
    assert(count == 1);
    auto viewport = viewports[0];
    glViewport(
        (GLint)     viewport.originX,
        (GLint)     viewport.originY,
        (GLsizei)   viewport.extentX,
        (GLsizei)   viewport.extentY);
    glDepthRange(viewport.minZ, viewport.maxZ);
}

void GLRenderer::setScissorRects(UInt count, ScissorRect const* rects)
{
    assert(count <= 1);
    if( count )
    {
        // TODO: this isn't goign to be quite right because of the
        // flipped coordinate system in GL.
        //
        // The best way around this is probably to *always* render
        // things internally into textures with "flipped" conventions,
        // and then only deal with the flipping as part of a final
        // "present" step that copies to the primary back-buffer.
        //
        auto rect = rects[0];
        glScissor(
            GLint(rect.minX),
            GLint(rect.minY),
            GLsizei(rect.maxX - rect.minX),
            GLsizei(rect.maxY - rect.minY));

        glEnable(GL_SCISSOR_TEST);
    }
    else
    {
        glDisable(GL_SCISSOR_TEST);
    }
}

void GLRenderer::setPipelineState(PipelineType pipelineType, PipelineState* state)
{
    auto pipelineStateImpl = (PipelineStateImpl*) state;

    m_currentPipelineState = pipelineStateImpl;

    auto program = pipelineStateImpl->m_program;
    GLuint programID = program ? program->m_id : 0;
    glUseProgram(programID);
}

void GLRenderer::draw(UInt vertexCount, UInt startVertex = 0)
{
    flushStateForDraw();

    glDrawArrays(m_boundPrimitiveTopology, (GLint)startVertex, (GLsizei)vertexCount);
}

void GLRenderer::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
    assert(!"unimplemented");
}

void GLRenderer::dispatchCompute(int x, int y, int z)
{
    glDispatchCompute(x, y, z);
}

void GLRenderer::DescriptorSetImpl::setConstantBuffer(UInt range, UInt index, BufferResource* buffer)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;

    auto layout = m_layout;
    auto rangeInfo = layout->m_ranges[range];
    auto arrayIndex = rangeInfo.arrayIndex + index;

    m_constantBuffers[arrayIndex] = resourceImpl;
}

void GLRenderer::DescriptorSetImpl::setResource(UInt range, UInt index, ResourceView* view)
{
    auto viewImpl = (ResourceViewImpl*) view;

    auto layout = m_layout;
    auto rangeInfo = layout->m_ranges[range];
    auto arrayIndex = rangeInfo.arrayIndex + index;

    assert(!"unimplemented");
}

void GLRenderer::DescriptorSetImpl::setSampler(UInt range, UInt index, SamplerState* sampler)
{
    assert(!"unsupported");
}

void GLRenderer::DescriptorSetImpl::setCombinedTextureSampler(
    UInt range,
    UInt index,
    ResourceView*   textureView,
    SamplerState*   sampler)
{
    auto viewImpl = (TextureViewImpl*) textureView;
    auto samplerImpl = (SamplerStateImpl*) sampler;

    auto layout = m_layout;
    auto rangeInfo = layout->m_ranges[range];
    auto arrayIndex = rangeInfo.arrayIndex + index;

    m_textures[arrayIndex] = viewImpl;
    m_samplers[arrayIndex] = samplerImpl;
}

void GLRenderer::DescriptorSetImpl::setRootConstants(
    UInt range,
    UInt offset,
    UInt size,
    void const* data)
{
    SLANG_UNEXPECTED("unimplemented: setRootConstants for GlRenderer");
}

void GLRenderer::setDescriptorSet(PipelineType pipelineType, PipelineLayout* layout, UInt index, DescriptorSet* descriptorSet)
{
    auto descriptorSetImpl = (DescriptorSetImpl*)descriptorSet;

    // TODO: can we just bind things immediately here, rather than shadowing the state?

    m_boundDescriptorSets[index] = descriptorSetImpl;
}

Result GLRenderer::createDescriptorSetLayout(const DescriptorSetLayout::Desc& desc, DescriptorSetLayout** outLayout)
{
    RefPtr<DescriptorSetLayoutImpl> layoutImpl = new DescriptorSetLayoutImpl();

    Int counts[int(GLDescriptorSlotType::CountOf)] = { 0, };

    Int rangeCount = desc.slotRangeCount;
    for(Int rr = 0; rr < rangeCount; ++rr)
    {
        auto rangeDesc = desc.slotRanges[rr];
        DescriptorSetLayoutImpl::RangeInfo rangeInfo;

        GLDescriptorSlotType glSlotType;
        switch( rangeDesc.type )
        {
        default:
            assert(!"unsupported");
            break;

        // TODO: There are many other slot types we could support here,
        // in particular including storage buffers.

        case DescriptorSlotType::CombinedImageSampler:
            glSlotType = GLDescriptorSlotType::CombinedTextureSampler;
            break;

        case DescriptorSlotType::RootConstant:
            rangeDesc.count = 1;
        case DescriptorSlotType::UniformBuffer:
        case DescriptorSlotType::DynamicUniformBuffer:
            glSlotType = GLDescriptorSlotType::ConstantBuffer;
            break;
        }

        rangeInfo.type = glSlotType;
        rangeInfo.arrayIndex = counts[int(glSlotType)];
        counts[int(glSlotType)] += rangeDesc.count;

        layoutImpl->m_ranges.add(rangeInfo);
    }

    for( Int ii = 0; ii < int(GLDescriptorSlotType::CountOf); ++ii )
    {
        layoutImpl->m_counts[ii] = counts[ii];
    }

    *outLayout = layoutImpl.detach();
    return SLANG_OK;
}

Result GLRenderer::createPipelineLayout(const PipelineLayout::Desc& desc, PipelineLayout** outLayout)
{
    RefPtr<PipelineLayoutImpl> layoutImpl = new PipelineLayoutImpl();

    static const int kSlotTypeCount = int(GLDescriptorSlotType::CountOf);
    Int counts[kSlotTypeCount] = { 0, };

    Int setCount = desc.descriptorSetCount;
    for( Int ii = 0; ii < setCount; ++ii )
    {
        auto setLayout = (DescriptorSetLayoutImpl*) desc.descriptorSets[ii].layout;

        PipelineLayoutImpl::DescriptorSetInfo setInfo;
        setInfo.layout = setLayout;

        for( Int ii = 0; ii < int(GLDescriptorSlotType::CountOf); ++ii )
        {
            setInfo.baseArrayIndex[ii] = counts[ii];
            counts[ii] += setLayout->m_counts[ii];
        }

        layoutImpl->m_sets.add(setInfo);
    }

    *outLayout = layoutImpl.detach();
    return SLANG_OK;
}

Result GLRenderer::createDescriptorSet(DescriptorSetLayout* layout, DescriptorSet** outDescriptorSet)
{
    auto layoutImpl = (DescriptorSetLayoutImpl*) layout;

    RefPtr<DescriptorSetImpl> descriptorSetImpl = new DescriptorSetImpl();

    descriptorSetImpl->m_layout = layoutImpl;

    // TODO: storage for the arrays of bound objects could be tail allocated
    // as part of the descriptor set, with offsets pre-computed in the
    // descriptor set layout.

    {
        auto slotTypeIndex = int(GLDescriptorSlotType::ConstantBuffer);
        auto slotCount = layoutImpl->m_counts[slotTypeIndex];
        descriptorSetImpl->m_constantBuffers.setCount(slotCount);
    }

    {
        auto slotTypeIndex = int(GLDescriptorSlotType::CombinedTextureSampler);
        auto slotCount = layoutImpl->m_counts[slotTypeIndex];

        descriptorSetImpl->m_textures.setCount(slotCount);
        descriptorSetImpl->m_samplers.setCount(slotCount);
    }

    *outDescriptorSet = descriptorSetImpl.detach();
    return SLANG_OK;
}

Result GLRenderer::createProgram(const ShaderProgram::Desc& desc, ShaderProgram** outProgram)
{
    auto programID = glCreateProgram();
    if(desc.pipelineType == PipelineType::Compute )
    {
        auto computeKernel = desc.findKernel(StageType::Compute);
        auto computeShaderID = loadShader(GL_COMPUTE_SHADER, (char const*) computeKernel->codeBegin);
        glAttachShader(programID, computeShaderID);
        glLinkProgram(programID);
        glDeleteShader(computeShaderID);
    }
    else
    {
        auto vertexKernel = desc.findKernel(StageType::Vertex);
        auto fragmentKernel = desc.findKernel(StageType::Fragment);

        auto vertexShaderID = loadShader(GL_VERTEX_SHADER, (char const*) vertexKernel->codeBegin);
        auto fragmentShaderID = loadShader(GL_FRAGMENT_SHADER, (char const*) fragmentKernel->codeBegin);

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
        return SLANG_FAIL;
    }

    *outProgram = new ShaderProgramImpl(m_weakRenderer, programID);
    return SLANG_OK;
}

Result GLRenderer::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, PipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

    auto programImpl        = (ShaderProgramImpl*)  desc.program;
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;
    auto inputLayoutImpl    = (InputLayoutImpl*)    desc.inputLayout;

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_program = programImpl;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->m_inputLayout = inputLayoutImpl;
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

Result GLRenderer::createComputePipelineState(const ComputePipelineStateDesc& inDesc, PipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

    auto programImpl        = (ShaderProgramImpl*)  desc.program;
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_program = programImpl;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}


} // renderer_test

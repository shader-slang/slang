// render-gl.cpp
#include "render-gl.h"

#include "../nvapi/nvapi-util.h"

#include "../immediate-renderer-base.h"

#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-secure-crt.h"
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
#include "external/wglext.h"

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
    F(glCopyBufferSubData,  PFNGLCOPYBUFFERSUBDATAPROC) \
	F(glDeleteBuffers,		PFNGLDELETEBUFFERSPROC)		\
	F(glMapBuffer,          PFNGLMAPBUFFERPROC)         \
    F(glUnmapBuffer,        PFNGLUNMAPBUFFERPROC)       \
    F(glUseProgram,         PFNGLUSEPROGRAMPROC)        \
    F(glBindBufferBase,     PFNGLBINDBUFFERBASEPROC)    \
    F(glBindBufferRange,    PFNGLBINDBUFFERRANGEPROC)   \
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
    F(glBindImageTexture,           PFNGLBINDIMAGETEXTUREPROC) \
    F(glSamplerParameteri,          PFNGLSAMPLERPARAMETERIPROC) \
    F(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC) \
    F(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC) \
    F(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC) \
    F(glDrawBuffers, PFNGLDRAWBUFFERSPROC) \
    F(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC) \
    F(glFramebufferTextureLayer, PFNGLFRAMEBUFFERTEXTURELAYERPROC) \
    F(glBlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC) \
    F(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC) \
    F(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC) \
    F(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC) \
    F(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC) \
    F(glDrawElementsBaseVertex, PFNGLDRAWELEMENTSBASEVERTEXPROC) \
    /* end */

#define MAP_WGL_EXTENSION_FUNCS(F) \
    F(wglCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC) \
    /* end */
using namespace Slang;

namespace gfx {

class GLDevice : public ImmediateRendererBase
{
public:
    // Renderer    implementation
    virtual SLANG_NO_THROW Result SLANG_MCALL initialize(const Desc& desc) override;
    virtual void clearFrame(uint32_t mask, bool clearDepth, bool clearStencil) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebufferLayout(
        const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebuffer(
        const IFramebuffer::Desc& desc,
        IFramebuffer** outFramebuffer) override;
    virtual void setFramebuffer(IFramebuffer* frameBuffer) override;
    virtual void setStencilReference(uint32_t referenceValue) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        IResource::Usage initialUsage,
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override;

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout,
        ShaderObjectLayoutBase** outLayout) override;
    virtual Result createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject) override;
    virtual Result createRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override;
    virtual void bindRootShaderObject(IShaderObject* shaderObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;

    virtual void copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL readTextureResource(
        ITextureResource* texture, ResourceState state, ISlangBlob** outBlob, size_t* outRowPitch, size_t* outPixelSize) override;

    virtual void* map(IBufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(IBufferResource* buffer) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual void setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override;
    virtual void setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset) override;
    virtual void setViewports(UInt count, Viewport const* viewports) override;
    virtual void setScissorRects(UInt count, ScissorRect const* rects) override;
    virtual void setPipelineState(IPipelineState* state) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;
    virtual void submitGpuWork() override {}
    virtual void waitForGpu() override {}
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override
    {
        return m_info;
    }

    HGLRC createGLContext(HDC hdc);
    GLDevice();
    ~GLDevice();

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

    class InputLayoutImpl: public IInputLayout, public RefObject
	{
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IInputLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IInputLayout)
                return static_cast<IInputLayout*>(this);
            return nullptr;
        }
		public:
        VertexAttributeDesc m_attributes[kMaxVertexStreams];
        UInt m_attributeCount = 0;
    };

	class BufferResourceImpl: public BufferResource
	{
		public:
        typedef BufferResource Parent;

        BufferResourceImpl(Usage initialUsage, const Desc& desc, WeakSink<GLDevice>* renderer, GLuint id, GLenum target):
            Parent(desc),
			m_renderer(renderer),
			m_handle(id),
            m_initialUsage(initialUsage),
            m_target(target),
            m_size(desc.sizeInBytes)
		{}
		~BufferResourceImpl()
		{
			if (auto renderer = m_renderer->get())
			{
				renderer->glDeleteBuffers(1, &m_handle);
			}
		}

        Usage m_initialUsage;
		RefPtr<WeakSink<GLDevice> > m_renderer;
		GLuint m_handle;
        GLenum m_target;
        UInt m_size;
	};

    class TextureResourceImpl: public TextureResource
    {
        public:
        typedef TextureResource Parent;

        TextureResourceImpl(Usage initialUsage, const Desc& desc, WeakSink<GLDevice>* renderer):
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
        RefPtr<WeakSink<GLDevice> > m_renderer;
        GLenum m_target;
        GLuint m_handle;
    };

    class SamplerStateImpl : public ISamplerState, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ISamplerState* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISamplerState)
                return static_cast<ISamplerState*>(this);
            return nullptr;
        }
    public:
        GLuint m_samplerID;
    };

    class ResourceViewImpl : public IResourceView, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IResourceView* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResourceView)
                return static_cast<IResourceView*>(this);
            return nullptr;
        }
    public:
        enum class Type
        {
            Texture, Buffer
        };
        Type type;
    };

    class TextureViewImpl : public ResourceViewImpl
    {
    public:
        RefPtr<TextureResourceImpl> m_resource;
        GLuint m_textureID;
        GLuint m_target;
        enum class TextureViewType
        {
            Texture, Image
        };
        TextureViewType textureViewType;
        GLint level;
        GLboolean layered;
        GLint layer;
        GLenum access;
        GLenum format;
    };

    class BufferViewImpl : public ResourceViewImpl
    {
    public:
        RefPtr<BufferResourceImpl>  m_resource;
        GLuint                      m_bufferID;
    };

    class FramebufferLayoutImpl
        : public IFramebufferLayout
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IFramebufferLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IFramebufferLayout)
                return static_cast<IFramebufferLayout*>(this);
            return nullptr;
        }

    public:
        ShortList<IFramebufferLayout::AttachmentLayout> m_renderTargets;
        bool m_hasDepthStencil = false;
        IFramebufferLayout::AttachmentLayout m_depthStencil;
    };

    class FramebufferImpl
        : public IFramebuffer
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IFramebuffer* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IFramebuffer)
                return static_cast<IFramebuffer*>(this);
            return nullptr;
        }

    public:
        GLuint m_framebuffer;
        ShortList<GLenum> m_drawBuffers;
        WeakSink<GLDevice>* m_renderer;
        ShortList<RefPtr<TextureViewImpl>> renderTargetViews;
        RefPtr<TextureViewImpl> depthStencilView;
        ShortList<ColorClearValue> m_colorClearValues;
        bool m_sameClearValues = true;
        DepthStencilClearValue m_depthStencilClearValue;

        FramebufferImpl(WeakSink<GLDevice>* renderer) :m_renderer(renderer) {}
        ~FramebufferImpl()
        {
            if (auto renderer = m_renderer->get())
            {
                renderer->glDeleteFramebuffers(1, &m_framebuffer);
            }
        }
        void createGLFramebuffer()
        {
            auto renderer = m_renderer->get();
            renderer->glGenFramebuffers(1, &m_framebuffer);
            renderer->glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
            m_drawBuffers.clear();
            m_colorClearValues.clear();
            for (Index i = 0; i < renderTargetViews.getCount(); i++)
            {
                auto rtv = renderTargetViews[i].Ptr();
                renderer->glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (uint32_t)i, GL_TEXTURE_2D, rtv->m_textureID, 0);
                m_drawBuffers.add((GLenum)(GL_COLOR_ATTACHMENT0 + i));
                m_colorClearValues.add(rtv->m_resource->getDesc()->optimalClearValue.color);
            }
            m_sameClearValues = true;
            for (Index i = 1; i < m_colorClearValues.getCount() && m_sameClearValues; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (m_colorClearValues[i].floatValues[j] !=
                        m_colorClearValues[0].floatValues[j])
                    {
                        m_sameClearValues = false;
                        break;
                    }
                }
            }
            if (depthStencilView)
            {
                renderer->glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_2D,
                    depthStencilView->m_textureID,
                    0);
                m_depthStencilClearValue =
                    depthStencilView->m_resource->getDesc()->optimalClearValue.depthStencil;
            }
            auto error = renderer->glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (error != GL_FRAMEBUFFER_COMPLETE)
            {
                return;
            }
        }
    };

    class SwapchainImpl
        : public ISwapchain
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ISwapchain* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISwapchain)
                return static_cast<ISwapchain*>(this);
            return nullptr;
        }

    public:
        ~SwapchainImpl()
        {
            destroyBackBufferAndFBO();
            wglDeleteContext(m_glrc);
            ::ReleaseDC(m_hwnd, m_hdc);
        }
        void destroyBackBufferAndFBO()
        {
            if (m_images.getCount())
            {
                wglMakeCurrent(m_rendererHDC, m_rendererRC);
                if (auto rendererRef = m_renderer->get())
                {
                    rendererRef->glDeleteFramebuffers(1, &m_framebuffer);
                }
                wglMakeCurrent(m_hdc, m_glrc);
                glDeleteTextures(1, &m_backBuffer);
                for (auto image : m_images)
                    image->m_handle = 0;
                m_images.clear();
            }
        }
        void createBackBufferAndFBO()
        {
            if (m_desc.width > 0 && m_desc.height > 0)
            {
                wglMakeCurrent(m_rendererHDC, m_rendererRC);

                glGenTextures(1, &m_backBuffer);
                glBindTexture(GL_TEXTURE_2D, m_backBuffer);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RGBA8,
                    m_desc.width,
                    m_desc.height,
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    nullptr);

                wglMakeCurrent(m_hdc, m_glrc);
                m_renderer->get()->glGenFramebuffers(1, &m_framebuffer);
                m_renderer->get()->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
                m_renderer->get()->glFramebufferTexture2D(
                    GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_backBuffer, 0);

                m_images.clear();
                for (uint32_t i = 0; i < m_desc.imageCount; i++)
                {
                    ITextureResource::Desc texDesc = {};
                    texDesc.init2D(
                        IResource::Type::Texture2D,
                        gfx::Format::RGBA_Unorm_UInt8,
                        m_desc.width,
                        m_desc.height,
                        1);
                    RefPtr<TextureResourceImpl> tex = new TextureResourceImpl(
                        IResource::Usage::RenderTarget, texDesc, m_renderer);
                    tex->m_handle = m_backBuffer;
                    m_images.add(tex);
                }
                wglMakeCurrent(m_rendererHDC, m_rendererRC);
            }
        }
        Result init(GLDevice* renderer, const ISwapchain::Desc& desc, WindowHandle window)
        {
            m_renderer = renderer->m_weakRenderer.Ptr();
            m_rendererHDC = renderer->m_hdc;
            m_rendererRC = renderer->m_glContext;

            m_hwnd = (HWND)window.handleValues[0];
            m_hdc = ::GetDC(m_hwnd);
            m_glrc = renderer->createGLContext(m_hdc);
            m_desc = desc;

            createBackBufferAndFBO();
            return SLANG_OK;
        }
        virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override { return m_desc; }
        virtual SLANG_NO_THROW Result SLANG_MCALL
            getImage(uint32_t index, ITextureResource** outResource) override
        {
            m_images[index]->addRef();
            *outResource = m_images[index].Ptr();
            return SLANG_OK;
        }
        virtual SLANG_NO_THROW Result SLANG_MCALL present() override
        {
            glFlush();
            wglMakeCurrent(m_hdc, m_glrc);
            auto renderer = m_renderer->get();
            renderer->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            renderer->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
            renderer->glBlitFramebuffer(
                0,
                0,
                m_desc.width,
                m_desc.height,
                0,
                0,
                m_desc.width,
                m_desc.height,
                GL_COLOR_BUFFER_BIT,
                GL_NEAREST);
            SwapBuffers(m_hdc);
            wglMakeCurrent(renderer->m_hdc, renderer->m_glContext);
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override
        {
            if (m_desc.width > 0 && m_desc.height > 0)
                return 0;
            return -1;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL resize(uint32_t width, uint32_t height) override
        {
            if (width > 0 && height > 0 && (width != m_desc.width || height != m_desc.height))
            {
                m_desc.width = width;
                m_desc.height = height;
                destroyBackBufferAndFBO();
                createBackBufferAndFBO();
            }
            return SLANG_OK;
        }

    public:
        WeakSink<GLDevice>* m_renderer = nullptr;
        GLuint m_framebuffer;
        GLuint m_backBuffer;
        HGLRC m_glrc;
        HWND m_hwnd;
        HDC m_hdc;

        HDC m_rendererHDC;
        HGLRC m_rendererRC;
        ISwapchain::Desc m_desc;
        ShortList<RefPtr<TextureResourceImpl>> m_images;
    };

	class ShaderProgramImpl : public ShaderProgramBase
    {
	public:
		ShaderProgramImpl(WeakSink<GLDevice>* renderer, GLuint id):
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
		RefPtr<WeakSink<GLDevice> > m_renderer;
	};

    class PipelineStateImpl : public PipelineStateBase
    {
    public:
        RefPtr<InputLayoutImpl>     m_inputLayout;
        void init(const GraphicsPipelineStateDesc& inDesc)
        {
            PipelineStateDesc pipelineDesc;
            pipelineDesc.type = PipelineType::Graphics;
            pipelineDesc.graphics = inDesc;
            initializeBase(pipelineDesc);
        }
        void init(const ComputePipelineStateDesc& inDesc)
        {
            PipelineStateDesc pipelineDesc;
            pipelineDesc.type = PipelineType::Compute;
            pipelineDesc.compute = inDesc;
            initializeBase(pipelineDesc);
        }
    };

    struct RootBindingState
    {
        List<RefPtr<TextureViewImpl>> textureBindings;
        List<RefPtr<TextureViewImpl>> imageBindings;
        List<GLuint> samplerBindings;
        List<GLuint> uniformBufferBindings;
        List<GLuint> storageBufferBindings;
    };

    class ShaderObjectLayoutImpl : public ShaderObjectLayoutBase
    {
    public:
        struct BindingRangeInfo
        {
            slang::BindingType bindingType;
            Index count;
            Index baseIndex;

            // Returns true if this binding range consumes a specialization argument slot.
            bool isSpecializationArg() const
            {
                return bindingType == slang::BindingType::ExistentialValue;
            }
        };

        struct SubObjectRangeInfo
        {
            RefPtr<ShaderObjectLayoutImpl> layout;
            Index bindingRangeIndex;
        };

        struct Builder
        {
        public:
            Builder(RendererBase* renderer)
                : m_renderer(renderer)
            {}

            RendererBase* m_renderer;
            slang::TypeLayoutReflection* m_elementTypeLayout;

            List<BindingRangeInfo> m_bindingRanges;
            List<SubObjectRangeInfo> m_subObjectRanges;

            Index m_textureCount = 0;
            Index m_imageCount = 0;
            Index m_storageBufferCount = 0;
            Index m_subObjectCount = 0;

            Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout)
            {
                typeLayout = _unwrapParameterGroups(typeLayout);

                m_elementTypeLayout = typeLayout;

                // Compute the binding ranges that are used to store
                // the logical contents of the object in memory.

                SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
                for (SlangInt r = 0; r < bindingRangeCount; ++r)
                {
                    slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                    SlangInt count = typeLayout->getBindingRangeBindingCount(r);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(r);

                    BindingRangeInfo bindingRangeInfo;
                    bindingRangeInfo.bindingType = slangBindingType;
                    bindingRangeInfo.count = count;

                    switch (slangBindingType)
                    {
                    case slang::BindingType::ConstantBuffer:
                    case slang::BindingType::ParameterBlock:
                    case slang::BindingType::ExistentialValue:
                        bindingRangeInfo.baseIndex = m_subObjectCount;
                        m_subObjectCount += count;
                        break;

                    case slang::BindingType::Sampler:
                        break;

                    case slang::BindingType::Texture:
                    case slang::BindingType::CombinedTextureSampler:
                        bindingRangeInfo.baseIndex = m_textureCount;
                        m_textureCount += count;
                        break;

                    case slang::BindingType::MutableTexture:
                        bindingRangeInfo.baseIndex = m_imageCount;
                        m_imageCount += count;
                        break;

                    case slang::BindingType::MutableRawBuffer:
                    case slang::BindingType::MutableTypedBuffer:
                        bindingRangeInfo.baseIndex = m_storageBufferCount;
                        m_storageBufferCount += count;
                        break;
                    case slang::BindingType::VaryingInput:
                    case slang::BindingType::VaryingOutput:
                        break;
                    default:
                        SLANG_ASSERT(!"unsupported binding type.");
                        break;
                    }
                    m_bindingRanges.add(bindingRangeInfo);
                }

                SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
                for (SlangInt r = 0; r < subObjectRangeCount; ++r)
                {
                    SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
                    auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

                    // A sub-object range can either represent a sub-object of a known
                    // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
                    // (in which case we can pre-compute a layout to use, based on
                    // the type `Foo`) *or* it can represent a sub-object of some
                    // existential type (e.g., `IBar`) in which case we cannot
                    // know the appropraite type/layout of sub-object to allocate.
                    //
                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    if (slangBindingType != slang::BindingType::ExistentialValue)
                    {
                        createForElementType(
                            m_renderer,
                            slangLeafTypeLayout->getElementTypeLayout(),
                            subObjectLayout.writeRef());
                    }

                    SubObjectRangeInfo subObjectRange;
                    subObjectRange.bindingRangeIndex = bindingRangeIndex;
                    subObjectRange.layout = subObjectLayout;

                    m_subObjectRanges.add(subObjectRange);
                }
                return SLANG_OK;
            }

            SlangResult build(ShaderObjectLayoutImpl** outLayout)
            {
                auto layout =
                    RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
                SLANG_RETURN_ON_FAIL(layout->_init(this));

                *outLayout = layout.detach();
                return SLANG_OK;
            }
        };

        static Result createForElementType(
            RendererBase* renderer,
            slang::TypeLayoutReflection* elementType,
            ShaderObjectLayoutImpl** outLayout)
        {
            Builder builder(renderer);
            builder.setElementTypeLayout(elementType);
            return builder.build(outLayout);
        }

        List<BindingRangeInfo> const& getBindingRanges() { return m_bindingRanges; }

        Index getBindingRangeCount() { return m_bindingRanges.getCount(); }

        BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRanges[index]; }

        slang::TypeLayoutReflection* getElementTypeLayout() { return m_elementTypeLayout; }

        Index getTextureCount() { return m_textureCount; }
        Index getImageCount() { return m_imageCount; }
        Index getStorageBufferCount() { return m_storageBufferCount; }
        Index getSubObjectCount() { return m_subObjectCount; }

        SubObjectRangeInfo const& getSubObjectRange(Index index) { return m_subObjectRanges[index]; }
        List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }

        RendererBase* getRenderer() { return m_renderer; }

        slang::TypeReflection* getType()
        {
            return m_elementTypeLayout->getType();
        }
    protected:
        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            initBase(renderer, builder->m_elementTypeLayout);

            m_bindingRanges = builder->m_bindingRanges;

            m_textureCount = builder->m_textureCount;
            m_imageCount = builder->m_imageCount;
            m_storageBufferCount = builder->m_storageBufferCount;
            m_subObjectCount = builder->m_subObjectCount;
            m_subObjectRanges = builder->m_subObjectRanges;
            return SLANG_OK;
        }

        List<BindingRangeInfo> m_bindingRanges;
        Index m_textureCount = 0;
        Index m_imageCount = 0;
        Index m_storageBufferCount = 0;
        Index m_subObjectCount = 0;
        List<SubObjectRangeInfo> m_subObjectRanges;
    };

    class RootShaderObjectLayoutImpl : public ShaderObjectLayoutImpl
    {
        typedef ShaderObjectLayoutImpl Super;

    public:
        struct EntryPointInfo
        {
            RefPtr<ShaderObjectLayoutImpl> layout;
        };

        struct Builder : Super::Builder
        {
            Builder(
                RendererBase* renderer,
                slang::IComponentType* program,
                slang::ProgramLayout* programLayout)
                : Super::Builder(renderer)
                , m_program(program)
                , m_programLayout(programLayout)
            {}

            Result build(RootShaderObjectLayoutImpl** outLayout)
            {
                RefPtr<RootShaderObjectLayoutImpl> layout = new RootShaderObjectLayoutImpl();
                SLANG_RETURN_ON_FAIL(layout->_init(this));

                *outLayout = layout.detach();
                return SLANG_OK;
            }

            void addGlobalParams(slang::VariableLayoutReflection* globalsLayout)
            {
                setElementTypeLayout(globalsLayout->getTypeLayout());
            }

            void addEntryPoint(SlangStage stage, ShaderObjectLayoutImpl* entryPointLayout)
            {
                EntryPointInfo info;
                info.layout = entryPointLayout;
                m_entryPoints.add(info);
            }

            slang::IComponentType* m_program;
            slang::ProgramLayout* m_programLayout;
            List<EntryPointInfo> m_entryPoints;
        };

        EntryPointInfo& getEntryPoint(Index index) { return m_entryPoints[index]; }

        List<EntryPointInfo>& getEntryPoints() { return m_entryPoints; }

        static Result create(
            RendererBase* renderer,
            slang::IComponentType* program,
            slang::ProgramLayout* programLayout,
            RootShaderObjectLayoutImpl** outLayout)
        {
            RootShaderObjectLayoutImpl::Builder builder(renderer, program, programLayout);
            builder.addGlobalParams(programLayout->getGlobalParamsVarLayout());

            SlangInt entryPointCount = programLayout->getEntryPointCount();
            for (SlangInt e = 0; e < entryPointCount; ++e)
            {
                auto slangEntryPoint = programLayout->getEntryPointByIndex(e);
                RefPtr<ShaderObjectLayoutImpl> entryPointLayout;
                SLANG_RETURN_ON_FAIL(ShaderObjectLayoutImpl::createForElementType(
                    renderer, slangEntryPoint->getTypeLayout(), entryPointLayout.writeRef()));
                builder.addEntryPoint(slangEntryPoint->getStage(), entryPointLayout);
            }

            SLANG_RETURN_ON_FAIL(builder.build(outLayout));

            return SLANG_OK;
        }

        slang::IComponentType* getSlangProgram() const { return m_program; }
        slang::ProgramLayout* getSlangProgramLayout() const { return m_programLayout; }

    protected:
        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            SLANG_RETURN_ON_FAIL(Super::_init(builder));

            m_program = builder->m_program;
            m_programLayout = builder->m_programLayout;
            m_entryPoints = builder->m_entryPoints;
            return SLANG_OK;
        }

        ComPtr<slang::IComponentType>   m_program;
        slang::ProgramLayout* m_programLayout = nullptr;

        List<EntryPointInfo> m_entryPoints;
    };

    class ShaderObjectImpl : public ShaderObjectBase
    {
    public:
        static Result create(
            IDevice* device,
            ShaderObjectLayoutImpl* layout,
            ShaderObjectImpl** outShaderObject)
        {
            auto object = ComPtr<ShaderObjectImpl>(new ShaderObjectImpl());
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            *outShaderObject = object.detach();
            return SLANG_OK;
        }

        RendererBase* getDevice() { return m_layout->getDevice(); }

        SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return 0; }

        SLANG_NO_THROW Result SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint)
            SLANG_OVERRIDE
        {
            *outEntryPoint = nullptr;
            return SLANG_OK;
        }

        ShaderObjectLayoutImpl* getLayout()
        {
            return static_cast<ShaderObjectLayoutImpl*>(m_layout.Ptr());
        }

        SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() SLANG_OVERRIDE
        {
            return m_layout->getElementTypeLayout();
        }

        SLANG_NO_THROW Result SLANG_MCALL
            setData(ShaderOffset const& inOffset, void const* data, size_t inSize) SLANG_OVERRIDE
        {
            Index offset = inOffset.uniformOffset;
            Index size = inSize;

            char* dest = m_ordinaryData.getBuffer();
            Index availableSize = m_ordinaryData.getCount();

            // TODO: We really should bounds-check access rather than silently ignoring sets
            // that are too large, but we have several test cases that set more data than
            // an object actually stores on several targets...
            //
            if (offset < 0)
            {
                size += offset;
                offset = 0;
            }
            if ((offset + size) >= availableSize)
            {
                size = availableSize - offset;
            }

            memcpy(dest + offset, data, size);

            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            setObject(ShaderOffset const& offset, IShaderObject* object)
            SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;

            auto subObject = static_cast<ShaderObjectImpl*>(object);

            auto bindingRangeIndex = offset.bindingRangeIndex;
            auto& bindingRange = layout->getBindingRange(bindingRangeIndex);

            m_objects[bindingRange.baseIndex + offset.bindingArrayIndex] = subObject;

            // If the range being assigned into represents an interface/existential-type leaf field,
            // then we need to consider how the `object` being assigned here affects specialization.
            // We may also need to assign some data from the sub-object into the ordinary data
            // buffer for the parent object.
            //
            if (bindingRange.bindingType == slang::BindingType::ExistentialValue)
            {
                // A leaf field of interface type is laid out inside of the parent object
                // as a tuple of `(RTTI, WitnessTable, Payload)`. The layout of these fields
                // is a contract between the compiler and any runtime system, so we will
                // need to rely on details of the binary layout.

                // We start by querying the layout/type of the concrete value that the application
                // is trying to store into the field, and also the layout/type of the leaf
                // existential-type field itself.
                //
                auto concreteTypeLayout = subObject->getElementTypeLayout();
                auto concreteType = concreteTypeLayout->getType();
                //
                auto existentialTypeLayout = layout->getElementTypeLayout()->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                auto existentialType = existentialTypeLayout->getType();

                // The first field of the tuple (offset zero) is the run-time type information (RTTI)
                // ID for the concrete type being stored into the field.
                //
                // TODO: We need to be able to gather the RTTI type ID from `object` and then
                // use `setData(offset, &TypeID, sizeof(TypeID))`.

                // The second field of the tuple (offset 8) is the ID of the "witness" for the
                // conformance of the concrete type to the interface used by this field.
                //
                auto witnessTableOffset = offset;
                witnessTableOffset.uniformOffset += 8;
                //
                // Conformances of a type to an interface are computed and then stored by the
                // Slang runtime, so we can look up the ID for this particular conformance (which
                // will create it on demand).
                //
                ComPtr<slang::ISession> slangSession;
                SLANG_RETURN_ON_FAIL(getRenderer()->getSlangSession(slangSession.writeRef()));
                //
                // Note: If the type doesn't actually conform to the required interface for
                // this sub-object range, then this is the point where we will detect that
                // fact and error out.
                //
                uint32_t conformanceID = 0xFFFFFFFF;
                SLANG_RETURN_ON_FAIL(slangSession->getTypeConformanceWitnessSequentialID(
                    concreteType, existentialType, &conformanceID));
                //
                // Once we have the conformance ID, then we can write it into the object
                // at the required offset.
                //
                SLANG_RETURN_ON_FAIL(setData(witnessTableOffset, &conformanceID, sizeof(conformanceID)));

                // The third field of the tuple (offset 16) is the "payload" that is supposed to
                // hold the data for a value of the given concrete type.
                //
                auto payloadOffset = offset;
                payloadOffset.uniformOffset += 16;

                // There are two cases we need to consider here for how the payload might be used:
                //
                // * If the concrete type of the value being bound is one that can "fit" into the
                //   available payload space,  then it should be stored in the payload.
                //
                // * If the concrete type of the value cannot fit in the payload space, then it
                //   will need to be stored somewhere else.
                //
                if (_doesValueFitInExistentialPayload(concreteTypeLayout, existentialTypeLayout))
                {
                    // If the value can fit in the payload area, then we will go ahead and copy
                    // its bytes into that area.
                    //
                    setData(payloadOffset, subObject->m_ordinaryData.getBuffer(), subObject->m_ordinaryData.getCount());
                }
                else
                {
                    // If the value does *not *fit in the payload area, then there is nothing
                    // we can do at this point (beyond saving a reference to the sub-object, which
                    // was handled above).
                    //
                    // Once all the sub-objects have been set into the parent object, we can
                    // compute a specialized layout for it, and that specialized layout can tell
                    // us where the data for these sub-objects has been laid out.
                }
            }

            return SLANG_E_NOT_IMPLEMENTED;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            getObject(ShaderOffset const& offset, IShaderObject** outObject)
            SLANG_OVERRIDE
        {
            SLANG_ASSERT(outObject);
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

            auto object = m_objects[bindingRange.baseIndex + offset.bindingArrayIndex].Ptr();
            object->addRef();
            *outObject = object;
            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL
            setResource(ShaderOffset const& offset, IResourceView* resourceView) SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

            auto resourceViewImpl = static_cast<ResourceViewImpl*>(resourceView);
            switch (bindingRange.bindingType)
            {
            case slang::BindingType::MutableRawBuffer:
            case slang::BindingType::MutableTypedBuffer:
            case slang::BindingType::RawBuffer:
            case slang::BindingType::TypedBuffer:
                m_storageBuffers[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<BufferViewImpl*>(resourceView);
                break;
            case slang::BindingType::MutableTexture:
                m_images[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<TextureViewImpl*>(resourceView);
                break;
            case slang::BindingType::Texture:
                m_textures[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<TextureViewImpl*>(resourceView);
                m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] = nullptr;
                break;
            }
            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL setSampler(ShaderOffset const& offset, ISamplerState* sampler)
            SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

            m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<SamplerStateImpl*>(sampler);
            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
            ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);
            m_textures[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<TextureViewImpl*>(textureView);
            m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<SamplerStateImpl*>(sampler);
            return SLANG_OK;
        }

    public:
        // Appends all types that are used to specialize the element type of this shader object in `args` list.
        virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
        {
            auto& subObjectRanges = getLayout()->getSubObjectRanges();
            // The following logic is built on the assumption that all fields that involve existential types (and
            // therefore require specialization) will results in a sub-object range in the type layout.
            // This allows us to simply scan the sub-object ranges to find out all specialization arguments.
            Index subObjectRangeCount = subObjectRanges.getCount();
            for (Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount; subObjectRangeIndex++)
            {
                auto const& subObjectRange = subObjectRanges[subObjectRangeIndex];
                auto const& bindingRange = getLayout()->getBindingRange(subObjectRange.bindingRangeIndex);

                Index count = bindingRange.count;
                SLANG_ASSERT(count == 1);

                Index subObjectIndexInRange = 0;
                auto subObject = m_objects[bindingRange.baseIndex + subObjectIndexInRange];

                switch (bindingRange.bindingType)
                {
                case slang::BindingType::ExistentialValue:
                {
                    // A binding type of `ExistentialValue` means the sub-object represents a interface-typed field.
                    // In this case the specialization argument for this field is the actual specialized type of the bound
                    // shader object. If the shader object's type is an ordinary type without existential fields, then the
                    // type argument will simply be the ordinary type. But if the sub object's type is itself a specialized
                    // type, we need to make sure to use that type as the specialization argument.

                    ExtendedShaderObjectType specializedSubObjType;
                    SLANG_RETURN_ON_FAIL(subObject->getSpecializedShaderObjectType(&specializedSubObjType));
                    args.add(specializedSubObjType);
                    break;
                }
                case slang::BindingType::ParameterBlock:
                case slang::BindingType::ConstantBuffer:
                    // Currently we only handle the case where the field's type is
                    // `ParameterBlock<SomeStruct>` or `ConstantBuffer<SomeStruct>`, where `SomeStruct` is a struct type
                    // (not directly an interface type). In this case, we just recursively collect the specialization arguments
                    // from the bound sub object.
                    SLANG_RETURN_ON_FAIL(subObject->collectSpecializationArgs(args));
                    // TODO: we need to handle the case where the field is of the form `ParameterBlock<IFoo>`. We should treat
                    // this case the same way as the `ExistentialValue` case here, but currently we lack a mechanism to distinguish
                    // the two scenarios.
                    break;
                }
                // TODO: need to handle another case where specialization happens on resources fields e.g. `StructuredBuffer<IFoo>`.
            }
            return SLANG_OK;
        }


    protected:
        friend class ProgramVars;

        Result init(IDevice* device, ShaderObjectLayoutImpl* layout)
        {
            m_layout = layout;

            // If the layout tells us that there is any uniform data,
            // then we will allocate a CPU memory buffer to hold that data
            // while it is being set from the host.
            //
            // Once the user is done setting the parameters/fields of this
            // shader object, we will produce a GPU-memory version of the
            // uniform data (which includes values from this object and
            // any existential-type sub-objects).
            //
            size_t uniformSize = layout->getElementTypeLayout()->getSize();
            if (uniformSize)
            {
                m_ordinaryData.setCount(uniformSize);
                memset(m_ordinaryData.getBuffer(), 0, uniformSize);
            }

            m_samplers.setCount(layout->getTextureCount());
            m_textures.setCount(layout->getTextureCount());
            m_images.setCount(layout->getImageCount());
            m_storageBuffers.setCount(layout->getStorageBufferCount());

            // If the layout specifies that we have any sub-objects, then
            // we need to size the array to account for them.
            //
            Index subObjectCount = layout->getSubObjectCount();
            m_objects.setCount(subObjectCount);

            for (auto subObjectRangeInfo : layout->getSubObjectRanges())
            {
                auto subObjectLayout = subObjectRangeInfo.layout;

                // In the case where the sub-object range represents an
                // existential-type leaf field (e.g., an `IBar`), we
                // cannot pre-allocate the object(s) to go into that
                // range, since we can't possibly know what to allocate
                // at this point.
                //
                if (!subObjectLayout)
                    continue;
                //
                // Otherwise, we will allocate a sub-object to fill
                // in each entry in this range, based on the layout
                // information we already have.

                auto& bindingRangeInfo = layout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);
                for (Index i = 0; i < bindingRangeInfo.count; ++i)
                {
                    RefPtr<ShaderObjectImpl> subObject;
                    SLANG_RETURN_ON_FAIL(
                        ShaderObjectImpl::create(device, subObjectLayout, subObject.writeRef()));
                    m_objects[bindingRangeInfo.baseIndex + i] = subObject;
                }
            }

            return SLANG_OK;
        }

        /// Write the uniform/ordinary data of this object into the given `dest` buffer at the given `offset`
        Result _writeOrdinaryData(
            GLDevice* device,
            BufferResourceImpl* buffer,
            size_t offset,
            size_t                              destSize,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            auto src = m_ordinaryData.getBuffer();
            auto srcSize = size_t(m_ordinaryData.getCount());

            SLANG_ASSERT(srcSize <= destSize);

            device->uploadBufferData(buffer, offset, srcSize, src);

            // In the case where this object has any sub-objects of
            // existential/interface type, we need to recurse on those objects
            // that need to write their state into an appropriate "pending" allocation.
            //
            // Note: Any values that could fit into the "payload" included
            // in the existential-type field itself will have already been
            // written as part of `setObject()`. This loop only needs to handle
            // those sub-objects that do not "fit."
            //
            // An implementers looking at this code might wonder if things could be changed
            // so that *all* writes related to sub-objects for interface-type fields could
            // be handled in this one location, rather than having some in `setObject()` and
            // others handled here.
            //
            Index subObjectRangeCounter = 0;
            for (auto const& subObjectRangeInfo : specializedLayout->getSubObjectRanges())
            {
                Index subObjectRangeIndex = subObjectRangeCounter++;
                auto const& bindingRangeInfo = specializedLayout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);

                // We only need to handle sub-object ranges for interface/existential-type fields,
                // because fields of constant-buffer or parameter-block type are responsible for
                // the ordinary/uniform data of their own existential/interface-type sub-objects.
                //
                if (bindingRangeInfo.bindingType != slang::BindingType::ExistentialValue)
                    continue;

                // Each sub-object range represents a single "leaf" field, but might be nested
                // under zero or more outer arrays, such that the number of existential values
                // in the same range can be one or more.
                //
                auto count = bindingRangeInfo.count;

                // We are not concerned with the case where the existential value(s) in the range
                // git into the payload part of the leaf field.
                //
                // In the case where the value didn't fit, the Slang layout strategy would have
                // considered the requirements of the value as a "pending" allocation, and would
                // allocate storage for the ordinary/uniform part of that pending allocation inside
                // of the parent object's type layout.
                //
                // Here we assume that the Slang reflection API can provide us with a single byte
                // offset and stride for the location of the pending data allocation in the specialized
                // type layout, which will store the values for this sub-object range.
                //
                // TODO: The reflection API functions we are assuming here haven't been implemented
                // yet, so the functions being called here are stubs.
                //
                // TODO: It might not be that a single sub-object range can reliably map to a single
                // contiguous array with a single stride; we need to carefully consider what the layout
                // logic does for complex cases with multiple layers of nested arrays and structures.
                //
                size_t subObjectRangePendingDataOffset = _getSubObjectRangePendingDataOffset(specializedLayout, subObjectRangeIndex);
                size_t subObjectRangePendingDataStride = _getSubObjectRangePendingDataStride(specializedLayout, subObjectRangeIndex);

                // If the range doesn't actually need/use the "pending" allocation at all, then
                // we need to detect that case and skip such ranges.
                //
                // TODO: This should probably be handled on a per-object basis by caching a "does it fit?"
                // bit as part of the information for bound sub-objects, given that we already
                // compute the "does it fit?" status as part of `setObject()`.
                //
                if (subObjectRangePendingDataOffset == 0)
                    continue;

                for (Slang::Index i = 0; i < count; ++i)
                {
                    auto subObject = m_objects[bindingRangeInfo.baseIndex + i];

                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    SLANG_RETURN_ON_FAIL(subObject->_getSpecializedLayout(subObjectLayout.writeRef()));

                    auto subObjectOffset = subObjectRangePendingDataOffset + i * subObjectRangePendingDataStride;

                    subObject->_writeOrdinaryData(device, buffer, offset + subObjectOffset, destSize - subObjectOffset, subObjectLayout);
                }
            }

            return SLANG_OK;
        }

        // As discussed in `_writeOrdinaryData()`, these methods are just stubs waiting for
        // the "flat" Slang refelction information to provide access to the relevant data.
        //
        size_t _getSubObjectRangePendingDataOffset(ShaderObjectLayoutImpl* specializedLayout, Index subObjectRangeIndex) { return 0; }
        size_t _getSubObjectRangePendingDataStride(ShaderObjectLayoutImpl* specializedLayout, Index subObjectRangeIndex) { return 0; }

        /// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed
        Result _ensureOrdinaryDataBufferCreatedIfNeeded(GLDevice* device)
        {
            // If we have already created a buffer to hold ordinary data, then we should
            // simply re-use that buffer rather than re-create it.
            //
            // TODO: Simply re-using the buffer without any kind of validation checks
            // means that we are assuming that users cannot or will not perform any `set`
            // operations on a shader object once an operation has requested this buffer
            // be created. We need to enforce that rule if we want to rely on it.
            //
            if (m_ordinaryDataBuffer)
                return SLANG_OK;

            // Computing the size of the ordinary data buffer is *not* just as simple
            // as using the size of the `m_ordinayData` array that we store. The reason
            // for the added complexity is that interface-type fields may lead to the
            // storage being specialized such that it needs extra appended data to
            // store the concrete values that logically belong in those interface-type
            // fields but wouldn't fit in the fixed-size allocation we gave them.
            //
            // TODO: We need to actually implement that logic by using reflection
            // data computed for the specialized type of this shader object.
            // For now we just make the simple assumption described above despite
            // knowing that it is false.
            //
            RefPtr<ShaderObjectLayoutImpl> specializedLayout;
            SLANG_RETURN_ON_FAIL(_getSpecializedLayout(specializedLayout.writeRef()));

            auto specializedOrdinaryDataSize = specializedLayout->getElementTypeLayout()->getSize();
            if (specializedOrdinaryDataSize == 0)
                return SLANG_OK;

            // Once we have computed how large the buffer should be, we can allocate
            // it using the existing public `IDevice` API.
            //

            ComPtr<IBufferResource> bufferResourcePtr;
            IBufferResource::Desc bufferDesc;
            bufferDesc.init(specializedOrdinaryDataSize);
            bufferDesc.cpuAccessFlags |= IResource::AccessFlag::Write;
            SLANG_RETURN_ON_FAIL(device->createBufferResource(
                IResource::Usage::ConstantBuffer, bufferDesc, nullptr, bufferResourcePtr.writeRef()));
            m_ordinaryDataBuffer = static_cast<BufferResourceImpl*>(bufferResourcePtr.get());

            // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
            //
            // Note that `_writeOrdinaryData` is potentially recursive in the case
            // where this object contains interface/existential-type fields, so we
            // don't need or want to inline it into this call site.
            //
            SLANG_RETURN_ON_FAIL(_writeOrdinaryData(device, m_ordinaryDataBuffer, 0, specializedOrdinaryDataSize, specializedLayout));

            return SLANG_OK;
        }

        /// Bind the buffer for ordinary/uniform data, if needed
        Result _bindOrdinaryDataBufferIfNeeded(
            GLDevice* device,
            RootBindingState* bindingState)
        {
            // We start by ensuring that the buffer is created, if it is needed.
            //
            SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded(device));

            // If we did indeed need/create a buffer, then we must bind it
            // into root binding state.
            //
            if (m_ordinaryDataBuffer)
            {
                bindingState->uniformBufferBindings.add(m_ordinaryDataBuffer->m_handle);
            }

            return SLANG_OK;
        }
    public:
        virtual Result bindObject(GLDevice* device, RootBindingState* bindingState)
        {
            ShaderObjectLayoutImpl* layout = getLayout();

            Index baseRangeIndex = 0;
            SLANG_RETURN_ON_FAIL(_bindOrdinaryDataBufferIfNeeded(device, bindingState));

            for (auto sampler : m_samplers)
                bindingState->samplerBindings.add(sampler ? sampler->m_samplerID : 0);

            bindingState->textureBindings.addRange(m_textures);
            bindingState->imageBindings.addRange(m_images);

            for (auto buffer : m_storageBuffers)
                bindingState->storageBufferBindings.add(buffer ? buffer->m_bufferID : 0);

            for (auto subObject : m_objects)
                subObject->bindObject(device, bindingState);

            return SLANG_OK;
        }

        /// Any "ordinary" / uniform data for this object
        List<char> m_ordinaryData;

        List<RefPtr<TextureViewImpl>> m_textures;

        List<RefPtr<TextureViewImpl>> m_images;

        List<RefPtr<SamplerStateImpl>> m_samplers;

        List<RefPtr<BufferViewImpl>> m_storageBuffers;

        List<RefPtr<ShaderObjectImpl>> m_objects;

        /// A constant buffer used to stored ordinary data for this object
        /// and existential-type sub-objects.
        ///
        /// Created on demand with `_createOrdinaryDataBufferIfNeeded()`
        RefPtr<BufferResourceImpl> m_ordinaryDataBuffer;

        /// Get the layout of this shader object with specialization arguments considered
        ///
        /// This operation should only be called after the shader object has been
        /// fully filled in and finalized.
        ///
        Result _getSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
        {
            if (!m_specializedLayout)
            {
                SLANG_RETURN_ON_FAIL(_createSpecializedLayout(m_specializedLayout.writeRef()));
            }
            *outLayout = RefPtr<ShaderObjectLayoutImpl>(m_specializedLayout).detach();
            return SLANG_OK;
        }

        /// Create the layout for this shader object with specialization arguments considered
        ///
        /// This operation is virtual so that it can be customized by `ProgramVars`.
        ///
        virtual Result _createSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
        {
            ExtendedShaderObjectType extendedType;
            SLANG_RETURN_ON_FAIL(getSpecializedShaderObjectType(&extendedType));

            auto renderer = getRenderer();
            RefPtr<ShaderObjectLayoutBase> layout;
            SLANG_RETURN_ON_FAIL(renderer->getShaderObjectLayout(extendedType.slangType, layout.writeRef()));

            *outLayout = static_cast<ShaderObjectLayoutImpl*>(layout.detach());
            return SLANG_OK;
        }

        RefPtr<ShaderObjectLayoutImpl> m_specializedLayout;
    };

    class RootShaderObjectImpl : public ShaderObjectImpl
    {
        typedef ShaderObjectImpl Super;

    public:
        static Result create(IDevice* device, RootShaderObjectLayoutImpl* layout, RootShaderObjectImpl** outShaderObject)
        {
            RefPtr<RootShaderObjectImpl> object = new RootShaderObjectImpl();
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            *outShaderObject = object.detach();
            return SLANG_OK;
        }

        RootShaderObjectLayoutImpl* getLayout() { return static_cast<RootShaderObjectLayoutImpl*>(m_layout.Ptr()); }

        UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return (UInt)m_entryPoints.getCount(); }
        SlangResult SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint) SLANG_OVERRIDE
        {
            *outEntryPoint = m_entryPoints[index];
            m_entryPoints[index]->addRef();
            return SLANG_OK;
        }

        virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
        {
            SLANG_RETURN_ON_FAIL(ShaderObjectImpl::collectSpecializationArgs(args));
            for (auto& entryPoint : m_entryPoints)
            {
                SLANG_RETURN_ON_FAIL(entryPoint->collectSpecializationArgs(args));
            }
            return SLANG_OK;
        }

    protected:
        virtual Result bindObject(GLDevice* device, RootBindingState* bindingState) override
        {
            SLANG_RETURN_ON_FAIL(Super::bindObject(device, bindingState));

            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPoint = m_entryPoints[i];
                SLANG_RETURN_ON_FAIL(entryPoint->bindObject(device, bindingState));
            }

            return SLANG_OK;
        }

        Result init(IDevice* device, RootShaderObjectLayoutImpl* layout)
        {
            SLANG_RETURN_ON_FAIL(Super::init(device, layout));

            for (auto entryPointInfo : layout->getEntryPoints())
            {
                RefPtr<ShaderObjectImpl> entryPoint;
                SLANG_RETURN_ON_FAIL(
                    ShaderObjectImpl::create(device, entryPointInfo.layout, entryPoint.writeRef()));
                m_entryPoints.add(entryPoint);
            }

            return SLANG_OK;
        }

        Result _createSpecializedLayout(ShaderObjectLayoutImpl** outLayout) SLANG_OVERRIDE
        {
            ExtendedShaderObjectTypeList specializationArgs;
            SLANG_RETURN_ON_FAIL(collectSpecializationArgs(specializationArgs));

            // Note: There is an important policy decision being made here that we need
            // to approach carefully.
            //
            // We are doing two different things that affect the layout of a program:
            //
            // 1. We are *composing* one or more pieces of code (notably the shared global/module
            //    stuff and the per-entry-point stuff).
            //
            // 2. We are *specializing* code that includes generic/existential parameters
            //    to concrete types/values.
            //
            // We need to decide the relative *order* of these two steps, because of how it impacts
            // layout. The layout for `specialize(compose(A,B), X, Y)` is potentially different
            // form that of `compose(specialize(A,X), speciealize(B,Y))`, even when both are
            // semantically equivalent programs.
            //
            // Right now we are using the first option: we are first generating a full composition
            // of all the code we plan to use (global scope plus all entry points), and then
            // specializing it to the concatenated specialization argumenst for all of that.
            //
            // In some cases, though, this model isn't appropriate. For example, when dealing with
            // ray-tracing shaders and local root signatures, we really want the parameters of each
            // entry point (actually, each entry-point *group*) to be allocated distinct storage,
            // which really means we want to compute something like:
            //
            //      SpecializedGlobals = specialize(compose(ModuleA, ModuleB, ...), X, Y, ...)
            //
            //      SpecializedEP1 = compose(SpecializedGlobals, specialize(EntryPoint1, T, U, ...))
            //      SpecializedEP2 = compose(SpecializedGlobals, specialize(EntryPoint2, A, B, ...))
            //
            // Note how in this case all entry points agree on the layout for the shared/common
            // parmaeters, but their layouts are also independent of one another.
            //
            // Furthermore, in this example, loading another entry point into the system would not
            // rquire re-computing the layouts (or generated kernel code) for any of the entry points
            // that had already been loaded (in contrast to a compose-then-specialize approach).
            //
            ComPtr<slang::IComponentType> specializedComponentType;
            ComPtr<slang::IBlob> diagnosticBlob;
            auto result = getLayout()->getSlangProgram()->specialize(
                specializationArgs.components.getArrayView().getBuffer(),
                specializationArgs.getCount(),
                specializedComponentType.writeRef(),
                diagnosticBlob.writeRef());

            // TODO: print diagnostic message via debug output interface.

            if (result != SLANG_OK)
                return result;

            auto slangSpecializedLayout = specializedComponentType->getLayout();
            RefPtr<RootShaderObjectLayoutImpl> specializedLayout;
            RootShaderObjectLayoutImpl::create(getRenderer(), specializedComponentType, slangSpecializedLayout, specializedLayout.writeRef());

            // Note: Computing the layout for the specialized program will have also computed
            // the layouts for the entry points, and we really need to attach that information
            // to them so that they don't go and try to compute their own specializations.
            //
            // TODO: Well, if we move to the specialization model described above then maybe
            // we *will* want entry points to do their own specialization work...
            //
            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPointInfo = specializedLayout->getEntryPoint(i);
                auto entryPointVars = m_entryPoints[i];

                entryPointVars->m_specializedLayout = entryPointInfo.layout;
            }

            *outLayout = specializedLayout.detach();
            return SLANG_OK;
        }


        List<RefPtr<ShaderObjectImpl>> m_entryPoints;
    };

    enum class GlPixelFormat
    {
        Unknown,
        RGBA_Unorm_UInt8,
        D_Float32,
        D_Unorm24_S8,
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

    // GLDevice members.

    DeviceInfo m_info;
    String m_adapterName;

    HDC     m_hdc;
    HGLRC   m_glContext = 0;
    uint32_t m_stencilRef = 0;

    GLuint m_vao;
    RefPtr<PipelineStateImpl> m_currentPipelineState;
    RefPtr<FramebufferImpl> m_currentFramebuffer;
    RefPtr<WeakSink<GLDevice> > m_weakRenderer;

    RootBindingState m_rootBindingState;

    GLenum m_boundPrimitiveTopology = GL_TRIANGLES;
    GLuint  m_boundVertexStreamBuffers[kMaxVertexStreams];
    UInt    m_boundVertexStreamStrides[kMaxVertexStreams];
    UInt    m_boundVertexStreamOffsets[kMaxVertexStreams];
    GLuint m_boundIndexBuffer = 0;
    UInt m_boundIndexBufferOffset = 0;
    UInt m_boundIndexBufferSize = 0;

    Desc m_desc;
    WindowHandle m_windowHandle;
    // Declare a function pointer for each OpenGL
    // extension function we need to load
#define DECLARE_GL_EXTENSION_FUNC(NAME, TYPE) TYPE NAME;
    MAP_GL_EXTENSION_FUNCS(DECLARE_GL_EXTENSION_FUNC)
    MAP_WGL_EXTENSION_FUNCS(DECLARE_GL_EXTENSION_FUNC)
#undef DECLARE_GL_EXTENSION_FUNC

    static const GlPixelFormatInfo s_pixelFormatInfos[];            /// Maps GlPixelFormat to a format info
};

/* static */GLDevice::GlPixelFormat GLDevice::_getGlPixelFormat(Format format)
{
    switch (format)
    {
        case Format::RGBA_Unorm_UInt8:      return GlPixelFormat::RGBA_Unorm_UInt8;
        case Format::D_Float32:             return GlPixelFormat::D_Float32;
        case Format::D_Unorm24_S8:          return GlPixelFormat::D_Unorm24_S8;

        default:                            return GlPixelFormat::Unknown;
    }
}

/* static */ const GLDevice::GlPixelFormatInfo GLDevice::s_pixelFormatInfos[] =
{
    // internalType, format, formatType
    { 0,                0,          0},                         // GlPixelFormat::Unknown
    { GL_RGBA8,         GL_RGBA,    GL_UNSIGNED_BYTE },         // GlPixelFormat::RGBA_Unorm_UInt8
    { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE}, // GlPixelFormat::D_Float32
    { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE}, // GlPixelFormat::D_Unorm24_S8

};

/* static */void GLDevice::compileTimeAsserts()
{
    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(s_pixelFormatInfos) == int(GlPixelFormat::CountOf));
}

SlangResult SLANG_MCALL createGLDevice(const IDevice::Desc* desc, IDevice** outRenderer)
{
    RefPtr<GLDevice> result = new GLDevice();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    *outRenderer = result.detach();
    return SLANG_OK;
}

void GLDevice::debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message)
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

/* static */void APIENTRY GLDevice::staticDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    ((GLDevice*)userParam)->debugCallback(source, type, id, severity, length, message);
}

/* static */GLDevice::VertexAttributeFormat GLDevice::getVertexAttributeFormat(Format format)
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

void GLDevice::bindBufferImpl(int target, UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* offsets)
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

void GLDevice::flushStateForDraw()
{
    if (m_currentFramebuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_currentFramebuffer->m_framebuffer);
        glDrawBuffers(
            (GLsizei)m_currentFramebuffer->m_drawBuffers.getCount(),
            m_currentFramebuffer->m_drawBuffers.getArrayView().getBuffer());
    }
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
    if (m_boundIndexBuffer)
    {
        glBindBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, m_boundIndexBuffer, m_boundIndexBufferOffset, m_boundIndexBufferSize);
    }
}

GLuint GLDevice::loadShader(GLenum stage, const char* source)
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

#ifdef _WIN32
LRESULT CALLBACK WindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

WindowHandle createWindow()
{
    WindowHandle window = {};
#ifdef _WIN32
    const wchar_t className[] = L"OpenGLContextWindow";
    static bool windowClassRegistered = false;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!windowClassRegistered)
    {
        windowClassRegistered = true;
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = className;
        RegisterClass(&wc);
    }

    HWND hwnd = CreateWindowEx(
        0, // Optional window styles.
        className, // Window class
        L"GLWindow", // Window text
        WS_OVERLAPPEDWINDOW, // Window style
        // Size and position
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL, // Parent window
        NULL, // Menu
        hInstance, // Instance handle
        NULL // Additional application data
    );

    if (hwnd == NULL)
    {
        return window;
    }
    window = WindowHandle::FromHwnd(hwnd);
#endif
    return window;
}

void destroyWindow(WindowHandle window)
{
#ifdef _WIN32
    DestroyWindow((HWND)window.handleValues[0]);
#endif
}

GLDevice::GLDevice() { m_weakRenderer = new WeakSink<GLDevice>(this); }

GLDevice::~GLDevice()
{
    // We can destroy things whilst in this state
    m_currentPipelineState.setNull();
    m_currentFramebuffer.setNull();
    if (glDeleteVertexArrays)
    {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_glContext)
    {
        wglDeleteContext(m_glContext);
    }
    destroyWindow(m_windowHandle);

    // By resetting the weak pointer, other objects accessing through WeakSink<GLDevice> will no
    // longer be able to access this object which is entering a 'being destroyed' to 'destroyed'
    // state
    if (m_weakRenderer)
    {
        SLANG_ASSERT(m_weakRenderer->get() == this);
        m_weakRenderer->detach();
    }
}

HGLRC GLDevice::createGLContext(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pixelFormatDesc = {sizeof(PIXELFORMATDESCRIPTOR)};
    pixelFormatDesc.nVersion = 1;
    pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDesc.cColorBits = 32;
    pixelFormatDesc.cDepthBits = 24;
    pixelFormatDesc.cStencilBits = 8;
    pixelFormatDesc.iLayerType = PFD_MAIN_PLANE;
    int pixelFormatIndex = ChoosePixelFormat(hdc, &pixelFormatDesc);
    SetPixelFormat(hdc, pixelFormatIndex, &pixelFormatDesc);

    int attributeList[5];

    attributeList[0] = WGL_CONTEXT_MAJOR_VERSION_ARB;
    attributeList[1] = 4;
    attributeList[2] = WGL_CONTEXT_MINOR_VERSION_ARB;
    attributeList[3] = 3;
    attributeList[4] = 0;

    HGLRC newGLContext = wglCreateContextAttribsARB(hdc, m_glContext, attributeList);
    return newGLContext;
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::initialize(const Desc& desc)
{
    SLANG_RETURN_ON_FAIL(slangContext.initialize(desc.slang, SLANG_GLSL, "glsl_440"));

    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));

    // Initialize DeviceInfo
    {
        m_info.deviceType = DeviceType::OpenGl;
        m_info.bindingStyle = BindingStyle::OpenGl;
        m_info.projectionStyle = ProjectionStyle::OpenGl;
        m_info.apiName = "OpenGL";
        static const float kIdentity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
    }

    m_windowHandle = createWindow();
    m_desc = desc;

    m_hdc = ::GetDC((HWND)m_windowHandle.handleValues[0]);

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
    m_info.adapterName = (char*)renderer;

    if (renderer && desc.adapter)
    {
        String lowerAdapter = String(desc.adapter).toLower();
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
    MAP_WGL_EXTENSION_FUNCS(LOAD_GL_EXTENSION_FUNC)
#undef LOAD_GL_EXTENSION_FUNC

    wglMakeCurrent(m_hdc, 0);
    wglDeleteContext(m_glContext);
    m_glContext = 0;

    if (!wglCreateContextAttribsARB)
    {
        return SLANG_FAIL;
    }

    m_glContext = createGLContext(m_hdc);

    if (m_glContext == NULL)
    {
        return SLANG_FAIL;
    }
    wglMakeCurrent(m_hdc, m_glContext);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if (!glGenVertexArrays)
        return SLANG_FAIL;

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    if (glDebugMessageCallback)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(staticDebugCallback, this);
    }

    return SLANG_OK;
}

void GLDevice::clearFrame(uint32_t mask, bool clearDepth, bool clearStencil)
{
    uint32_t clearMask = 0;
    if (clearDepth)
    {
        clearMask |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(m_currentFramebuffer->m_depthStencilClearValue.depth);
    }
    if (clearStencil)
    {
        clearMask |= GL_STENCIL_BUFFER_BIT;
        glClearStencil(m_currentFramebuffer->m_depthStencilClearValue.stencil);
    }
    if (clearMask)
    {
        // If clear value for all attachments are the same, issue one `glClear` command.
        if (m_currentFramebuffer->m_sameClearValues &&
            m_currentFramebuffer->m_colorClearValues.getCount() > 0)
        {
            ShortList<GLenum> clearBuffers;
            auto clearColor = m_currentFramebuffer->m_colorClearValues[0];
            glClearColor(
                clearColor.floatValues[0],
                clearColor.floatValues[1],
                clearColor.floatValues[2],
                clearColor.floatValues[3]);
            for (Index i = 0; i < m_currentFramebuffer->m_colorClearValues.getCount(); i++)
            {
                if (mask & uint32_t(1 << i))
                    clearBuffers.add(GLenum(GL_COLOR_ATTACHMENT0 + i));
            }
            if (clearBuffers.getCount())
            {
                glDrawBuffers((GLsizei)clearBuffers.getCount(), clearBuffers.getArrayView().getBuffer());
                clearMask |= GL_COLOR_BUFFER_BIT;
            }
            glClear(clearMask);
            glDrawBuffers(
                (GLsizei)m_currentFramebuffer->m_drawBuffers.getCount(),
                m_currentFramebuffer->m_drawBuffers.getArrayView().getBuffer());
            return;
        }
        // If clear values are different, clear attachments separately.
        for (Index i = 0; i < m_currentFramebuffer->m_colorClearValues.getCount(); i++)
        {
            if (mask & uint32_t(1 << i))
            {
                GLenum drawBuffer = GLenum(GL_COLOR_ATTACHMENT0 + i);
                glDrawBuffers(1, &drawBuffer);
                auto clearColor = m_currentFramebuffer->m_colorClearValues[i];
                glClearColor(
                    clearColor.floatValues[0],
                    clearColor.floatValues[1],
                    clearColor.floatValues[2],
                    clearColor.floatValues[3]);
                glClear(GL_COLOR_BUFFER_BIT);
            }
        }
        // Clear depth/stencil attachments.
        glClear(clearMask);
        glDrawBuffers(
            (GLsizei)m_currentFramebuffer->m_drawBuffers.getCount(),
            m_currentFramebuffer->m_drawBuffers.getArrayView().getBuffer());
    }
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> swapchain = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(swapchain->init(this, desc, window));
    *outSwapchain = swapchain.detach();
    wglMakeCurrent(m_hdc, m_glContext);
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createFramebufferLayout(
    const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl();
    layout->m_renderTargets.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; i++)
    {
        layout->m_renderTargets[i] = desc.renderTargets[i];
    }

    if (desc.depthStencil)
    {
        layout->m_hasDepthStencil = true;
        layout->m_depthStencil = *desc.depthStencil;
    }
    else
    {
        layout->m_hasDepthStencil = false;
    }
    *outLayout = layout.detach();
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL
    GLDevice::createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer)
{
    RefPtr<FramebufferImpl> framebuffer = new FramebufferImpl(m_weakRenderer);
    framebuffer->renderTargetViews.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; i++)
    {
        framebuffer->renderTargetViews[i] =
            static_cast<TextureViewImpl*>(desc.renderTargetViews[i]);
    }
    framebuffer->depthStencilView = static_cast<TextureViewImpl*>(desc.depthStencilView);
    framebuffer->createGLFramebuffer();
    *outFramebuffer = framebuffer.detach();
    return SLANG_OK;
}

void GLDevice::setFramebuffer(IFramebuffer* frameBuffer)
{
    m_currentFramebuffer = static_cast<FramebufferImpl*>(frameBuffer);
}

void GLDevice::setStencilReference(uint32_t referenceValue)
{
    m_stencilRef = referenceValue;
    // TODO: actually set the stencil state.
}

void GLDevice::copyBuffer(
    IBufferResource* dst,
    size_t dstOffset,
    IBufferResource* src,
    size_t srcOffset,
    size_t size)
{
    auto dstImpl = static_cast<BufferResourceImpl*>(dst);
    auto srcImpl = static_cast<BufferResourceImpl*>(src);
    glBindBuffer(GL_COPY_READ_BUFFER, srcImpl->m_handle);
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstImpl->m_handle);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, size);
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::readTextureResource(
    ITextureResource* texture, ResourceState state, ISlangBlob** outBlob, size_t* outRowPitch, size_t* outPixelSize)
{
    SLANG_UNUSED(state);
    auto resource = static_cast<TextureResourceImpl*>(texture);
    auto size = resource->getDesc()->size;
    size_t requiredSize = size.width * size.height * sizeof(uint32_t);
    if (outRowPitch)
        *outRowPitch = size.width * sizeof(uint32_t);
    if (outPixelSize)
        *outPixelSize = sizeof(uint32_t);

    RefPtr<ListBlob> blob = new ListBlob();
    blob->m_data.setCount(requiredSize);
    auto buffer = blob->m_data.begin();
    glBindTexture(resource->m_target, resource->m_handle);
    glGetTexImage(resource->m_target, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    // Flip pixels vertically in-place.
    for (int y = 0; y < size.height / 2; y++)
    {
        for (int x = 0; x < size.width; x++)
        {
            std::swap(
                *((uint32_t*)buffer + y * size.width + x),
                *((uint32_t*)buffer + (size.height - y - 1) * size.width + x));
        }
    }

    *outBlob = blob.detach();

    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createTextureResource(
    IResource::Usage initialUsage,
    const ITextureResource::Desc& descIn,
    const ITextureResource::SubresourceData* initData,
    ITextureResource** outResource)
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

    // Set on texture so will be freed if failure
    texture->m_handle = handle;

    // TODO: The logic below seems to be ignoring the row/layer stride of
    // the subresources that have been passed in, despite OpenGL having
    // the ability to set the image unpack stride, etc.

    switch (srcDesc.type)
    {
        case IResource::Type::Texture1D:
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
                        // TODO: Double-check this logic - we are passing in `i` as the height?
                        glTexImage2D(
                            target,
                            j,
                            internalFormat,
                            Math::Max(1, srcDesc.size.width >> j),
                            i,
                            0,
                            format,
                            formatType,
                            initData ? initData[slice++].data : nullptr);
                    }
                }
            }
            else
            {
                target = GL_TEXTURE_1D;
                glBindTexture(target, handle);
                for (int i = 0; i < srcDesc.numMipLevels; i++)
                {
                    glTexImage1D(
                        target,
                        i,
                        internalFormat,
                        Math::Max(1, srcDesc.size.width >> i),
                        0,
                        format,
                        formatType,
                        initData ? initData[i].data : nullptr);
                }
            }
            break;
        }
        case IResource::Type::TextureCube:
        case IResource::Type::Texture2D:
        {
            if (srcDesc.arraySize > 0)
            {
                if (srcDesc.type == IResource::Type::TextureCube)
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
                        glTexImage3D(
                            target,
                            j,
                            internalFormat,
                            Math::Max(1, srcDesc.size.width >> j),
                            Math::Max(1, srcDesc.size.height >> j),
                            slice,
                            0,
                            format,
                            formatType,
                            initData ? initData[slice++].data : nullptr);
                    }
                }
            }
            else
            {
                if (srcDesc.type == IResource::Type::TextureCube)
                {
                    target = GL_TEXTURE_CUBE_MAP;
                    glBindTexture(target, handle);

                    int slice = 0;
                    for (int j = 0; j < 6; j++)
                    {
                        for (int i = 0; i < srcDesc.numMipLevels; i++)
                        {
                            glTexImage2D(
                                GL_TEXTURE_CUBE_MAP_POSITIVE_X + j,
                                i,
                                internalFormat,
                                Math::Max(1, srcDesc.size.width >> i),
                                Math::Max(1, srcDesc.size.height >> i),
                                0,
                                format,
                                formatType,
                                initData ? initData[slice++].data : nullptr);
                        }
                    }
                }
                else
                {
                    target = GL_TEXTURE_2D;
                    glBindTexture(target, handle);
                    for (int i = 0; i < srcDesc.numMipLevels; i++)
                    {
                        glTexImage2D(
                            target,
                            i,
                            internalFormat,
                            Math::Max(1, srcDesc.size.width >> i),
                            Math::Max(1, srcDesc.size.height >> i),
                            0,
                            format,
                            formatType,
                            initData ? initData[i].data : nullptr);
                    }
                }
            }
            break;
        }
        case IResource::Type::Texture3D:
        {
            target = GL_TEXTURE_3D;
            glBindTexture(target, handle);
            for (int i = 0; i < srcDesc.numMipLevels; i++)
            {
                glTexImage3D(
                    target,
                    i,
                    internalFormat,
                    Math::Max(1, srcDesc.size.width >> i),
                    Math::Max(1, srcDesc.size.height >> i),
                    Math::Max(1, srcDesc.size.depth >> i),
                    0,
                    format,
                    formatType,
                    initData ? initData[i].data : nullptr);
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

static GLenum _calcUsage(IResource::Usage usage)
{
    typedef IResource::Usage Usage;
    switch (usage)
    {
        case Usage::ConstantBuffer:     return GL_DYNAMIC_DRAW;
        default:                        return GL_STATIC_READ;
    }
}

static GLenum _calcTarget(IResource::Usage usage)
{
    typedef IResource::Usage Usage;
    switch (usage)
    {
        case Usage::ConstantBuffer:     return GL_UNIFORM_BUFFER;
        default:                        return GL_SHADER_STORAGE_BUFFER;
    }
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createBufferResource(
    IResource::Usage initialUsage,
    const IBufferResource::Desc& descIn,
    const void* initData,
    IBufferResource** outResource)
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

SLANG_NO_THROW Result SLANG_MCALL
    GLDevice::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
{
    GLuint samplerID;
    glCreateSamplers(1, &samplerID);

    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_samplerID = samplerID;
    *outSampler = samplerImpl.detach();
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createTextureView(
    ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = static_cast<TextureResourceImpl*>(texture);

    // TODO: actually do something?

    RefPtr<TextureViewImpl> viewImpl = new TextureViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_textureID = resourceImpl->m_handle;
    viewImpl->type = ResourceViewImpl::Type::Texture;
    viewImpl->m_target = resourceImpl->m_target;
    if (desc.type == IResourceView::Type::ShaderResource)
    {
        viewImpl->access = GL_READ_ONLY;
        viewImpl->textureViewType = TextureViewImpl::TextureViewType::Texture;
    }
    else
    {
        viewImpl->access = GL_READ_WRITE;
        viewImpl->textureViewType = TextureViewImpl::TextureViewType::Image;
    }
    const GlPixelFormatInfo& info = s_pixelFormatInfos[int(_getGlPixelFormat(desc.format))];
    viewImpl->format = info.internalFormat;
    viewImpl->layered = GL_TRUE;
    viewImpl->level = 0;
    viewImpl->layer = 0;
    *outView = viewImpl.detach();
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createBufferView(
    IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;

    // TODO: actually do something?

    RefPtr<BufferViewImpl> viewImpl = new BufferViewImpl();
    viewImpl->type = ResourceViewImpl::Type::Buffer;
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_bufferID = resourceImpl->m_handle;
    *outView = viewImpl.detach();
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createInputLayout(
    const InputElementDesc* inputElements, UInt inputElementCount, IInputLayout** outLayout)
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

void* GLDevice::map(IBufferResource* bufferIn, MapFlavor flavor)
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

void GLDevice::unmap(IBufferResource* bufferIn)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);
    glUnmapBuffer(buffer->m_target);
}

void GLDevice::setPrimitiveTopology(PrimitiveTopology topology)
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

void GLDevice::setVertexBuffers(
    UInt startSlot,
    UInt slotCount,
    IBufferResource* const* buffers,
    const UInt* strides,
    const UInt* offsets)
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

void GLDevice::setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset)
{
    auto bufferImpl = static_cast<BufferResourceImpl*>(buffer);
    m_boundIndexBuffer = bufferImpl->m_handle;
    m_boundIndexBufferOffset = offset;
    m_boundIndexBufferSize = bufferImpl->m_size;
}

void GLDevice::setViewports(UInt count, Viewport const* viewports)
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

void GLDevice::setScissorRects(UInt count, ScissorRect const* rects)
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

void GLDevice::setPipelineState(IPipelineState* state)
{
    auto pipelineStateImpl = static_cast<PipelineStateImpl*>(state);

    m_currentPipelineState = pipelineStateImpl;

    auto program = static_cast<ShaderProgramImpl*>(pipelineStateImpl->m_program.get());
    GLuint programID = program ? program->m_id : 0;
    glUseProgram(programID);
}

void GLDevice::draw(UInt vertexCount, UInt startVertex = 0)
{
    flushStateForDraw();

    glDrawArrays(m_boundPrimitiveTopology, (GLint)startVertex, (GLsizei)vertexCount);
}

void GLDevice::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
    flushStateForDraw();

    glDrawElementsBaseVertex(
        m_boundPrimitiveTopology,
        (GLsizei)indexCount,
        GL_UNSIGNED_INT,
        (GLvoid*)(startIndex*sizeof(uint32_t)),
        (GLint)baseVertex);
}

void GLDevice::dispatchCompute(int x, int y, int z)
{
    glDispatchCompute(x, y, z);
}

Result GLDevice::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
    if (desc.slangProgram->getSpecializationParamCount() != 0)
    {
        // For a specializable program, we don't invoke any actual slang compilation yet.
        RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl(m_weakRenderer, 0);
        shaderProgram->slangProgram = desc.slangProgram;
        *outProgram = shaderProgram.detach();
        return SLANG_OK;
    }

    auto programID = glCreateProgram();
    auto programLayout = desc.slangProgram->getLayout();
    ShortList<GLuint> shaderIDs;
    for (SlangUInt i = 0; i < programLayout->getEntryPointCount(); i++)
    {
        ComPtr<ISlangBlob> kernelCode;
        ComPtr<ISlangBlob> diagnostics;
        SLANG_RETURN_ON_FAIL(desc.slangProgram->getEntryPointCode(i, 0, kernelCode.writeRef(), diagnostics.writeRef()));
        GLenum glShaderType = 0;
        auto stage = programLayout->getEntryPointByIndex(i)->getStage();
        switch (stage)
        {
        case SLANG_STAGE_COMPUTE:
            glShaderType = GL_COMPUTE_SHADER;
            break;
        case SLANG_STAGE_VERTEX:
            glShaderType = GL_VERTEX_SHADER;
            break;
        case SLANG_STAGE_FRAGMENT:
            glShaderType = GL_FRAGMENT_SHADER;
            break;
        case SLANG_STAGE_GEOMETRY:
            glShaderType = GL_GEOMETRY_SHADER;
            break;
        case SLANG_STAGE_DOMAIN:
            glShaderType = GL_TESS_CONTROL_SHADER;
            break;
        case SLANG_STAGE_HULL:
            glShaderType = GL_TESS_EVALUATION_SHADER;
            break;
        default:
            SLANG_ASSERT(!"unsupported shader type.");
            break;
        }
        auto shaderID = loadShader(glShaderType, (char const*)kernelCode->getBufferPointer());
        shaderIDs.add(shaderID);
        glAttachShader(programID, shaderID);
    }
    glLinkProgram(programID);
    for (auto shaderID : shaderIDs)
        glDeleteShader(shaderID);
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

    RefPtr<ShaderProgramImpl> program = new ShaderProgramImpl(m_weakRenderer, programID);
    program->slangProgram = desc.slangProgram;
    *outProgram = program.detach();
    return SLANG_OK;
}

Result GLDevice::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;

    auto programImpl        = (ShaderProgramImpl*)  desc.program;
    auto inputLayoutImpl    = (InputLayoutImpl*)    desc.inputLayout;

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_inputLayout = inputLayoutImpl;
    pipelineStateImpl->init(desc);
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

Result GLDevice::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;

    auto programImpl        = (ShaderProgramImpl*)  desc.program;

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_program = programImpl;
    pipelineStateImpl->init(desc);
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

Result GLDevice::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout,
    ShaderObjectLayoutBase** outLayout)
{
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(ShaderObjectLayoutImpl::createForElementType(
        this, typeLayout, layout.writeRef()));
    *outLayout = layout.detach();
    return SLANG_OK;
}

Result GLDevice::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(this,
        static_cast<ShaderObjectLayoutImpl*>(layout), shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result GLDevice::createRootShaderObject(IShaderProgram* program, IShaderObject** outObject)
{
    auto programImpl = static_cast<ShaderProgramImpl*>(program);
    RefPtr<RootShaderObjectImpl> shaderObject;
    RefPtr<RootShaderObjectLayoutImpl> rootLayout;
    SLANG_RETURN_ON_FAIL(RootShaderObjectLayoutImpl::create(
        this, programImpl->slangProgram, programImpl->slangProgram->getLayout(), rootLayout.writeRef()));
    SLANG_RETURN_ON_FAIL(RootShaderObjectImpl::create(
        this, rootLayout.Ptr(), shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

void GLDevice::bindRootShaderObject(IShaderObject* shaderObject)
{
    RootShaderObjectImpl* rootShaderObjectImpl = static_cast<RootShaderObjectImpl*>(shaderObject);
    RefPtr<PipelineStateBase> specializedPipeline;
    maybeSpecializePipeline(m_currentPipelineState, rootShaderObjectImpl, specializedPipeline);
    setPipelineState(specializedPipeline.Ptr());

    m_rootBindingState.imageBindings.clear();
    m_rootBindingState.samplerBindings.clear();
    m_rootBindingState.textureBindings.clear();
    m_rootBindingState.storageBufferBindings.clear();
    m_rootBindingState.uniformBufferBindings.clear();
    static_cast<ShaderObjectImpl*>(shaderObject)->bindObject(this, &m_rootBindingState);
    for (Index i = 0; i < m_rootBindingState.imageBindings.getCount(); i++)
    {
        auto binding = m_rootBindingState.imageBindings[i];
        glBindImageTexture((GLuint)i, binding->m_textureID, binding->level, binding->layered, binding->layer, binding->access, binding->format);
    }
    for (Index i = 0; i < m_rootBindingState.textureBindings.getCount(); i++)
    {
        glActiveTexture((GLenum)(GL_TEXTURE0 + i));
        auto binding = m_rootBindingState.textureBindings[i];
        if (binding)
            glBindTexture(binding->m_target, binding->m_textureID);
        glBindSampler((GLuint)i, m_rootBindingState.samplerBindings[i]);
    }
    for (Index i = 0; i < m_rootBindingState.storageBufferBindings.getCount(); i++)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, (GLuint)i, m_rootBindingState.storageBufferBindings[i]);
    }
    for (Index i = 0; i < m_rootBindingState.uniformBufferBindings.getCount(); i++)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, (GLuint)i, m_rootBindingState.uniformBufferBindings[i]);
    }
}

} // renderer_test

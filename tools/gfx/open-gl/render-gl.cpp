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
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override;
    virtual SLANG_NO_THROW void SLANG_MCALL clearFrame(uint32_t mask, bool clearDepth, bool clearStencil) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebufferLayout(
        const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setFramebuffer(IFramebuffer* frameBuffer) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) override;

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

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createDescriptorSet(IDescriptorSetLayout* layout, IDescriptorSet::Flag::Enum flag, IDescriptorSet** outDescriptorSet) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;

    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL readTextureResource(
        ITextureResource* texture, ResourceState state, ISlangBlob** outBlob, size_t* outRowPitch, size_t* outPixelSize) override;

    virtual void* map(IBufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(IBufferResource* buffer) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
        PipelineType pipelineType,
        IPipelineLayout* layout,
        UInt index,
        IDescriptorSet* descriptorSet) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(UInt count, Viewport const* viewports) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(UInt count, ScissorRect const* rects) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) override;
    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex) override;
    virtual void SLANG_MCALL
        drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;
    virtual SLANG_NO_THROW void SLANG_MCALL submitGpuWork() override {}
    virtual SLANG_NO_THROW void SLANG_MCALL waitForGpu() override {}
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
		RefPtr<WeakSink<GLDevice> > m_renderer;
		GLuint m_handle;
        GLenum m_target;
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

    enum class GLDescriptorSlotType
    {
        ConstantBuffer,
        CombinedTextureSampler,
        StorageBuffer,
        CountOf,
    };

    class DescriptorSetLayoutImpl : public IDescriptorSetLayout, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IDescriptorSetLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IDescriptorSetLayout)
                return static_cast<IDescriptorSetLayout*>(this);
            return nullptr;
        }
    public:
        struct RangeInfo
        {
            GLDescriptorSlotType    type;
            UInt                    arrayIndex;
        };
        List<RangeInfo> m_ranges;
        Int             m_counts[int(GLDescriptorSlotType::CountOf)];
    };

    class PipelineLayoutImpl : public IPipelineLayout, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IPipelineLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IPipelineLayout)
            {
                return static_cast<IPipelineLayout*>(this);
            }
            return nullptr;
        }
    public:
        struct DescriptorSetInfo
        {
            RefPtr<DescriptorSetLayoutImpl> layout;
            UInt                            baseArrayIndex[int(GLDescriptorSlotType::CountOf)];
        };

        List<DescriptorSetInfo> m_sets;
    };

    class DescriptorSetImpl : public IDescriptorSet, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IDescriptorSet* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IDescriptorSet)
                return static_cast<IDescriptorSet*>(this);
            return nullptr;
        }
    public:
        virtual SLANG_NO_THROW void SLANG_MCALL
            setConstantBuffer(UInt range, UInt index, IBufferResource* buffer) override;
        virtual SLANG_NO_THROW void SLANG_MCALL
            setResource(UInt range, UInt index, IResourceView* view) override;
        virtual SLANG_NO_THROW void SLANG_MCALL
            setSampler(UInt range, UInt index, ISamplerState* sampler) override;
        virtual SLANG_NO_THROW void SLANG_MCALL setCombinedTextureSampler(
            UInt range,
            UInt index,
            IResourceView*   textureView,
            ISamplerState*   sampler) override;
        virtual SLANG_NO_THROW void SLANG_MCALL
            setRootConstants(
            UInt range,
            UInt offset,
            UInt size,
            void const* data) override;

        RefPtr<DescriptorSetLayoutImpl>     m_layout;
        List<RefPtr<BufferResourceImpl>>    m_constantBuffers;
        List<RefPtr<BufferResourceImpl>>    m_storageBuffers;
        List<RefPtr<TextureViewImpl>>       m_textures;
        List<RefPtr<SamplerStateImpl>>      m_samplers;
    };

	class ShaderProgramImpl : public GraphicsCommonShaderProgram
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

    RefPtr<DescriptorSetImpl>   m_boundDescriptorSets[kMaxDescriptorSetCount];

    GLenum m_boundPrimitiveTopology = GL_TRIANGLES;
    GLuint  m_boundVertexStreamBuffers[kMaxVertexStreams];
    UInt    m_boundVertexStreamStrides[kMaxVertexStreams];
    UInt    m_boundVertexStreamOffsets[kMaxVertexStreams];

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
    
    glBindVertexArray(m_vao);
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
    auto pipelineLayout =
        static_cast<PipelineLayoutImpl*>(m_currentPipelineState->m_pipelineLayout.get());
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
            // Then we will bind any storage buffers that were specified.

            auto slotTypeIndex = int(GLDescriptorSlotType::StorageBuffer);
            auto count = descriptorSetLayout->m_counts[slotTypeIndex];
            auto baseIndex = descriptorSetInfo.baseArrayIndex[slotTypeIndex];

            for (Int ii = 0; ii < count; ++ii)
            {
                auto bufferImpl = descriptorSet->m_storageBuffers[ii];
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GLuint(ii), bufferImpl->m_handle);
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

    SLANG_RETURN_ON_FAIL(GraphicsAPIRenderer::initialize(desc));

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

    if (glDebugMessageCallback)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(staticDebugCallback, this);
    }

    return SLANG_OK;
}

SLANG_NO_THROW void SLANG_MCALL
    GLDevice::clearFrame(uint32_t mask, bool clearDepth, bool clearStencil)
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

SLANG_NO_THROW void SLANG_MCALL GLDevice::setFramebuffer(IFramebuffer* frameBuffer)
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
    *outView = viewImpl.detach();
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createBufferView(
    IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;

    // TODO: actually do something?

    RefPtr<BufferViewImpl> viewImpl = new BufferViewImpl();
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

SLANG_NO_THROW void SLANG_MCALL GLDevice::setPrimitiveTopology(PrimitiveTopology topology)
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

SLANG_NO_THROW void SLANG_MCALL GLDevice::setVertexBuffers(
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

SLANG_NO_THROW void SLANG_MCALL
    GLDevice::setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset)
{
}

SLANG_NO_THROW void SLANG_MCALL GLDevice::setViewports(UInt count, Viewport const* viewports)
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

SLANG_NO_THROW void SLANG_MCALL GLDevice::setScissorRects(UInt count, ScissorRect const* rects)
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

SLANG_NO_THROW void SLANG_MCALL GLDevice::setPipelineState(IPipelineState* state)
{
    auto pipelineStateImpl = static_cast<PipelineStateImpl*>(state);

    m_currentPipelineState = pipelineStateImpl;

    auto program = static_cast<ShaderProgramImpl*>(pipelineStateImpl->m_program.get());
    GLuint programID = program ? program->m_id : 0;
    glUseProgram(programID);
}

SLANG_NO_THROW void SLANG_MCALL GLDevice::draw(UInt vertexCount, UInt startVertex = 0)
{
    flushStateForDraw();

    glDrawArrays(m_boundPrimitiveTopology, (GLint)startVertex, (GLsizei)vertexCount);
}

SLANG_NO_THROW void SLANG_MCALL
    GLDevice::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
    assert(!"unimplemented");
}

SLANG_NO_THROW void SLANG_MCALL GLDevice::dispatchCompute(int x, int y, int z)
{
    glDispatchCompute(x, y, z);
}

SLANG_NO_THROW void SLANG_MCALL GLDevice::DescriptorSetImpl::setConstantBuffer(
    UInt range, UInt index, IBufferResource* buffer)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;

    auto layout = m_layout;
    auto rangeInfo = layout->m_ranges[range];
    auto arrayIndex = rangeInfo.arrayIndex + index;

    m_constantBuffers[arrayIndex] = resourceImpl;
}

SLANG_NO_THROW void SLANG_MCALL
    GLDevice::DescriptorSetImpl::setResource(UInt range, UInt index, IResourceView* view)
{
    auto viewImpl = static_cast<ResourceViewImpl*>(view);

    auto layout = m_layout;
    auto rangeInfo = layout->m_ranges[range];
    auto arrayIndex = rangeInfo.arrayIndex + index;

    switch (rangeInfo.type)
    {
    case GLDescriptorSlotType::StorageBuffer:
        m_storageBuffers[arrayIndex] = static_cast<BufferViewImpl*>(viewImpl)->m_resource;
        break;
    default:
        assert(!"unimplemented");
        break;
    }
}

SLANG_NO_THROW void SLANG_MCALL
    GLDevice::DescriptorSetImpl::setSampler(UInt range, UInt index, ISamplerState* sampler)
{
    assert(!"unsupported");
}

SLANG_NO_THROW void SLANG_MCALL GLDevice::DescriptorSetImpl::setCombinedTextureSampler(
    UInt range,
    UInt index,
    IResourceView*   textureView,
    ISamplerState*   sampler)
{
    auto viewImpl = (TextureViewImpl*) textureView;
    auto samplerImpl = (SamplerStateImpl*) sampler;

    auto layout = m_layout;
    auto rangeInfo = layout->m_ranges[range];
    auto arrayIndex = rangeInfo.arrayIndex + index;

    m_textures[arrayIndex] = viewImpl;
    m_samplers[arrayIndex] = samplerImpl;
}

SLANG_NO_THROW void SLANG_MCALL GLDevice::DescriptorSetImpl::setRootConstants(
    UInt range,
    UInt offset,
    UInt size,
    void const* data)
{
    SLANG_UNEXPECTED("unimplemented: setRootConstants for GLDevice");
}

SLANG_NO_THROW void SLANG_MCALL GLDevice::setDescriptorSet(
    PipelineType pipelineType, IPipelineLayout* layout, UInt index, IDescriptorSet* descriptorSet)
{
    auto descriptorSetImpl = (DescriptorSetImpl*)descriptorSet;

    // TODO: can we just bind things immediately here, rather than shadowing the state?

    m_boundDescriptorSets[index] = descriptorSetImpl;
}

SLANG_NO_THROW Result SLANG_MCALL GLDevice::createDescriptorSetLayout(
    const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout)
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

        case DescriptorSlotType::StorageBuffer:
            glSlotType = GLDescriptorSlotType::StorageBuffer;
            break;
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

SLANG_NO_THROW Result SLANG_MCALL
    GLDevice::createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout)
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

SLANG_NO_THROW Result SLANG_MCALL
    GLDevice::createDescriptorSet(IDescriptorSetLayout* layout, IDescriptorSet::Flag::Enum flag, IDescriptorSet** outDescriptorSet)
{
    SLANG_UNUSED(flag);

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
        auto slotTypeIndex = int(GLDescriptorSlotType::StorageBuffer);
        auto slotCount = layoutImpl->m_counts[slotTypeIndex];
        descriptorSetImpl->m_storageBuffers.setCount(slotCount);
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

Result GLDevice::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
    if (desc.slangProgram && desc.slangProgram->getSpecializationParamCount() != 0)
    {
        // For a specializable program, we don't invoke any actual slang compilation yet.
        RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl(m_weakRenderer, 0);
        initProgramCommon(shaderProgram, desc);
        *outProgram = shaderProgram.detach();
        return SLANG_OK;
    }

    if( desc.kernelCount == 0 )
    {
        return createProgramFromSlang(this, desc, outProgram);
    }

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

    RefPtr<ShaderProgramImpl> program = new ShaderProgramImpl(m_weakRenderer, programID);
    initProgramCommon(program, desc);
    *outProgram = program.detach();
    return SLANG_OK;
}

Result GLDevice::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

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
    preparePipelineDesc(desc);

    auto programImpl        = (ShaderProgramImpl*)  desc.program;
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_program = programImpl;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->init(desc);
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}


} // renderer_test


#include "slangvm-browser-host.h"

#include "slang-hgvm/slang-hgvm-resource.h"
#include "slang-hgvm/slang-hgvm-texture-blitter.h"
#include "slangvm-browser-call.h"

namespace Slang::HGVM
{
using emscripten::val;

namespace
{
template<typename... T>
val arguments(T&&... values)
{
    val result = val::array();
    unsigned index = 0;
    (result.set(index++, std::forward<T>(values)), ...);
    return result;
}

SlangResult fail(String& error, const char* message)
{
    error = message;
    return SLANG_FAIL;
}

struct BrowserWindow : HGVMHostObject
{
    BrowserWindow(IHGVMHostServices* owner, val host, uint32_t id)
        : HGVMHostObject(HGVMHostObjectKind::Window, owner), host(host), id(id)
    {
    }

    ~BrowserWindow()
    {
        // Release listeners and DOM ownership when the last native Window reference disappears.
        // Cleanup is synchronous; browser errors are contained by the service boundary.
        callBrowserHost(host, "unregisterCanvas", arguments(id));
    }

    val host;
    uint32_t id;
    std::string selector;
    // A one-shot main can release its Slang Surface before JavaScript reads the canvas.
    // Keep the underlying presentation surface with the Window until the embedding page
    // captures the final image and releases the Window. RHI surfaces do not refer back to us.
    ComPtr<rhi::ISurface> presentationSurface;
};

struct BrowserSurface : HGVMHostObject
{
    explicit BrowserSurface(IHGVMHostServices* owner)
        : HGVMHostObject(HGVMHostObjectKind::Surface, owner)
    {
    }

    RefPtr<BrowserWindow> window;
    ComPtr<rhi::ISurface> surface;
    // User code may suspend after acquisition. Only this ordinary texture crosses that boundary.
    ComPtr<rhi::ITexture> image;
    rhi::Format requestedFormat = rhi::Format::Undefined;
    uint32_t imageCount = 2;
    bool acquired = false;
};
} // namespace

struct BrowserHostServices::Impl
{
    Impl(rhi::IDevice* device, val host)
        : device(device), host(host), blitter(device)
    {
    }

    SlangResult requireOpen(String& error)
    {
        val closed = callBrowserHost(host, "isClosed", arguments());
        if (getBrowserError(closed, error))
            return SLANG_FAIL;
        if (closed.as<bool>())
        {
            error = "The slangvm VM is closed";
            return SLANG_E_ABORT;
        }
        return SLANG_OK;
    }

    template<typename T>
    T* getObject(HGVMHostObject* object, HGVMHostObjectKind kind, String& error)
    {
        if (!object || object->getOwner() != owner || object->getKind() != kind)
        {
            error = "The HGVM host object has the wrong type or belongs to another VM";
            return nullptr;
        }
        return static_cast<T*>(object);
    }

    SlangResult getSize(BrowserWindow* window, uint32_t& width, uint32_t& height, String& error)
    {
        val size = callBrowserHost(host, "getWindowSize", arguments(window->id));
        if (getBrowserError(size, error))
            return SLANG_FAIL;
        width = size[0].as<uint32_t>();
        height = size[1].as<uint32_t>();
        return SLANG_OK;
    }

    SlangResult configure(BrowserSurface* surface, uint32_t width, uint32_t height, String& error)
    {
        if (!width || !height)
            return SLANG_E_NOT_AVAILABLE;
        const auto& info = surface->surface->getInfo();
        rhi::Format format = surface->requestedFormat == rhi::Format::Undefined
                                 ? info.preferredFormat
                                 : selectSurfaceFormat(info, surface->requestedFormat);
        if (format == rhi::Format::Undefined)
            return fail(error, "The requested HGVM surface format is unavailable");

        rhi::SurfaceConfig config = {};
        config.format = format;
        config.usage = rhi::TextureUsage::RenderTarget;
        config.width = width;
        config.height = height;
        config.desiredImageCount = surface->imageCount;
        config.vsync = true;
        SLANG_RETURN_ON_FAIL(surface->surface->configure(config));

        rhi::TextureDesc desc = {};
        desc.type = rhi::TextureType::Texture2D;
        desc.size = {width, height, 1};
        desc.format = format;
        desc.usage = rhi::TextureUsage::RenderTarget | rhi::TextureUsage::ShaderResource |
                     rhi::TextureUsage::CopySource | rhi::TextureUsage::CopyDestination;
        desc.defaultState = rhi::ResourceState::RenderTarget;
        ComPtr<rhi::ITexture> image = device->createTexture(desc);
        if (!image)
            return fail(error, "RHI failed to create the HGVM browser presentation image");
        SLANG_RETURN_ON_FAIL(blitter.prepare(format, error));
        surface->image = image;
        return SLANG_OK;
    }

    BrowserHostServices* owner = nullptr;
    ComPtr<rhi::IDevice> device;
    val host;
    TextureBlitter blitter;
};

BrowserHostServices::BrowserHostServices(rhi::IDevice* device, val host)
    : m_impl(new Impl(device, host))
{
    m_impl->owner = this;
}

BrowserHostServices::~BrowserHostServices() = default;

SlangResult BrowserHostServices::adoptWindow(
    uint32_t id,
    RefPtr<HGVMHostObject>& outWindow,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    val selector = callBrowserHost(m_impl->host, "getCanvasSelector", arguments(id));
    if (getBrowserError(selector, outError))
        return SLANG_FAIL;
    RefPtr<BrowserWindow> window = new BrowserWindow(this, m_impl->host, id);
    window->selector = selector.as<std::string>();
    outWindow = window;
    return SLANG_OK;
}

SlangResult BrowserHostServices::createWindow(
    std::string_view title,
    uint32_t width,
    uint32_t height,
    RefPtr<HGVMHostObject>& outWindow,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    val id =
        callBrowserHost(m_impl->host, "createWindow", arguments(std::string(title), width, height));
    if (getBrowserError(id, outError))
        return SLANG_FAIL;
    return adoptWindow(id.as<uint32_t>(), outWindow, outError);
}

SlangResult BrowserHostServices::processWindowEvents(
    HGVMHostObject* object,
    bool& outOpen,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    auto window = m_impl->getObject<BrowserWindow>(object, HGVMHostObjectKind::Window, outError);
    if (!window)
        return SLANG_E_INVALID_ARG;
    val open = callBrowserHost(m_impl->host, "processWindowEvents", arguments(window->id));
    if (getBrowserError(open, outError))
        return SLANG_FAIL;
    // Closing the VM wakes a pending frame wait. Abort before writing an instruction result so
    // the interpreter's normal failure cleanup can release every managed value safely.
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    outOpen = open.as<bool>();
    return SLANG_OK;
}

SlangResult BrowserHostServices::pollInputEvent(
    HGVMHostObject* object,
    HGVMInputEvent& outEvent,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    auto window = m_impl->getObject<BrowserWindow>(object, HGVMHostObjectKind::Window, outError);
    if (!window)
        return SLANG_E_INVALID_ARG;
    val event = callBrowserHost(m_impl->host, "pollInputEvent", arguments(window->id));
    if (getBrowserError(event, outError))
        return SLANG_FAIL;
    outEvent = {};
    if (event.isNull())
        return SLANG_OK;
    outEvent.type = HGVMInputEventType(event["type"].as<uint32_t>());
    outEvent.key = event["key"].as<uint32_t>();
    outEvent.button = event["button"].as<uint32_t>();
    for (unsigned i = 0; i < 2; ++i)
    {
        outEvent.position[i] = event["position"][i].as<float>();
        outEvent.scroll[i] = event["scroll"][i].as<float>();
        outEvent.size[i] = event["size"][i].as<uint32_t>();
    }
    return SLANG_OK;
}

SlangResult BrowserHostServices::closeWindow(HGVMHostObject* object, String& outError)
{
    auto window = m_impl->getObject<BrowserWindow>(object, HGVMHostObjectKind::Window, outError);
    if (!window)
        return SLANG_E_INVALID_ARG;
    val result = callBrowserHost(m_impl->host, "closeWindow", arguments(window->id));
    return getBrowserError(result, outError) ? SLANG_FAIL : SLANG_OK;
}

SlangResult BrowserHostServices::setWindowTitle(
    HGVMHostObject* object,
    std::string_view title,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    auto window = m_impl->getObject<BrowserWindow>(object, HGVMHostObjectKind::Window, outError);
    if (!window)
        return SLANG_E_INVALID_ARG;
    val result =
        callBrowserHost(m_impl->host, "setWindowTitle", arguments(window->id, std::string(title)));
    return getBrowserError(result, outError) ? SLANG_FAIL : SLANG_OK;
}

SlangResult BrowserHostServices::createSurface(
    HGVMHostObject* object,
    rhi::Format format,
    uint32_t imageCount,
    RefPtr<HGVMHostObject>& outSurface,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    auto window = m_impl->getObject<BrowserWindow>(object, HGVMHostObjectKind::Window, outError);
    if (!window)
        return SLANG_E_INVALID_ARG;
    if (!m_impl->device)
        return fail(outError, "Browser presentation requires the webgpu backend");
    RefPtr<BrowserSurface> surface = new BrowserSurface(this);
    surface->window = window;
    if (!window->presentationSurface)
        window->presentationSurface = m_impl->device->createSurface(
            rhi::WindowHandle::fromWGPUCanvas(window->selector.c_str()));
    surface->surface = window->presentationSurface;
    if (!surface->surface)
        return fail(outError, "RHI failed to create the HGVM browser surface");
    surface->requestedFormat = format;
    surface->imageCount = imageCount ? imageCount : 2;
    uint32_t width, height;
    SLANG_RETURN_ON_FAIL(m_impl->getSize(window, width, height, outError));
    SlangResult result = m_impl->configure(surface, width, height, outError);
    if (result != SLANG_E_NOT_AVAILABLE)
        SLANG_RETURN_ON_FAIL(result);
    outSurface = surface;
    return SLANG_OK;
}

SlangResult BrowserHostServices::resizeSurface(
    HGVMHostObject* object,
    uint32_t width,
    uint32_t height,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    auto surface = m_impl->getObject<BrowserSurface>(object, HGVMHostObjectKind::Surface, outError);
    if (!surface)
        return SLANG_E_INVALID_ARG;
    if (surface->acquired)
        return fail(outError, "Cannot resize an HGVM surface with an acquired image");
    // HGVM has retired graph work. Finish the host blit's submissions before replacing its image.
    auto queue = m_impl->device->getQueue(rhi::QueueType::Graphics);
    if (!queue || SLANG_FAILED(queue->waitOnHost()))
        return fail(outError, "RHI failed to finish browser presentation before resize");
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    return m_impl->configure(surface, width, height, outError);
}

SlangResult BrowserHostServices::getSurfaceDimensions(
    HGVMHostObject* object,
    uint32_t& outWidth,
    uint32_t& outHeight,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    auto surface = m_impl->getObject<BrowserSurface>(object, HGVMHostObjectKind::Surface, outError);
    if (!surface)
        return SLANG_E_INVALID_ARG;
    outWidth = surface->image ? surface->image->getDesc().size.width : 0;
    outHeight = surface->image ? surface->image->getDesc().size.height : 0;
    return SLANG_OK;
}

SlangResult BrowserHostServices::waitForSurface(HGVMHostObject* object, String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    auto surface = m_impl->getObject<BrowserSurface>(object, HGVMHostObjectKind::Surface, outError);
    if (!surface)
        return SLANG_E_INVALID_ARG;
    if (surface->acquired)
        return fail(outError, "Cannot wait for an HGVM surface with an acquired image");
    // Browser frame pacing belongs to processWindowEvents. The graph renders into an offscreen
    // image; reserving a canvas texture here would let it expire during a later suspension.
    uint32_t width, height;
    SLANG_RETURN_ON_FAIL(m_impl->getSize(surface->window, width, height, outError));
    if (!surface->image || !width || !height || width != surface->image->getDesc().size.width ||
        height != surface->image->getDesc().size.height)
        return SLANG_E_NOT_AVAILABLE;
    return SLANG_OK;
}

SlangResult BrowserHostServices::acquireSurfaceTexture(
    HGVMHostObject* object,
    ComPtr<rhi::ITexture>& outTexture,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(waitForSurface(object, outError));
    auto surface = static_cast<BrowserSurface*>(object);
    surface->acquired = true;
    outTexture = surface->image;
    return SLANG_OK;
}

SlangResult BrowserHostServices::presentSurface(HGVMHostObject* object, String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    auto surface = m_impl->getObject<BrowserSurface>(object, HGVMHostObjectKind::Surface, outError);
    if (!surface)
        return SLANG_E_INVALID_ARG;
    if (!surface->acquired)
        return fail(outError, "No HGVM browser surface image is ready to present");
    surface->acquired = false;
    uint32_t width, height;
    SLANG_RETURN_ON_FAIL(m_impl->getSize(surface->window, width, height, outError));
    if (width != surface->image->getDesc().size.width ||
        height != surface->image->getDesc().size.height)
        return SLANG_E_NOT_AVAILABLE;

    SLANG_RETURN_ON_FAIL(m_impl->blitter.prepare(surface->image->getDesc().format, outError));
    // No suspending calls are allowed between acquisition and submission: WebGPU expires this
    // canvas texture when control returns to the browser. The shader and pipeline are ready.
    ComPtr<rhi::ITexture> canvasImage;
    SLANG_RETURN_ON_FAIL(surface->surface->acquireNextImage(canvasImage.writeRef()));
    SLANG_RETURN_ON_FAIL(m_impl->blitter.blit(canvasImage, surface->image, outError));
    SLANG_RETURN_ON_FAIL(surface->surface->present());
    // Notify while this frame's canvas image is still current. A later browser task can see
    // an empty drawing buffer even though the compositor continues displaying the last frame.
    val result = callBrowserHost(m_impl->host, "didPresent", arguments(surface->window->id));
    return getBrowserError(result, outError) ? SLANG_FAIL : SLANG_OK;
}

SlangResult BrowserHostServices::blitTexture2D(
    rhi::ITexture* destination,
    rhi::ITexture* source,
    String& outError)
{
    SLANG_RETURN_ON_FAIL(m_impl->requireOpen(outError));
    return m_impl->blitter.blit(destination, source, outError);
}
} // namespace Slang::HGVM

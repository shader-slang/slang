// hgvm-native-host.cpp
#include "hgvm-native-host.h"

#include "core/slang-file-system.h"
#include "hgvm-input.h"
#include "platform/window.h"
#include "slang-hgvm/slang-hgvm-debug.h"
#include "slang-hgvm/slang-hgvm-resource.h"
#include "slang-hgvm/slang-hgvm-texture-blitter.h"
#include "slang-hgvm/slang-hgvm.h"


namespace SlangITool
{
using namespace Slang;
using namespace Slang::HGVM;

namespace
{

SlangResult fail(String& outError, const char* message)
{
    outError = message;
    return SLANG_FAIL;
}

struct NativeDebugCallback : rhi::IDebugCallback
{
    std::string messages;

    void SLANG_MCALL handleMessage(
        rhi::DebugMessageType type,
        rhi::DebugMessageSource source,
        const char* message) override
    {
        if (!message || !message[0])
            return;

        if (source != rhi::DebugMessageSource::Slang)
        {
            messages += "[RHI ";
            messages += type == rhi::DebugMessageType::Error     ? "error"
                        : type == rhi::DebugMessageType::Warning ? "warning"
                                                                 : "info";
            messages += "] ";
        }
        messages += message;
        if (messages.back() != '\n')
            messages += '\n';
    }
};

void prependDebugMessages(String& ioError, const std::string& messages)
{
    if (messages.empty())
        return;

    StringBuilder combined;
    combined.append(messages.data(), messages.size());
    combined << ioError;
    ioError = combined.produceString();
}

String makeString(std::string_view value)
{
    StringBuilder builder;
    builder.append(value.data(), value.size());
    return builder.produceString();
}

uint32_t translateKeyCode(platform::KeyCode key)
{
    const uint32_t keyValue = uint32_t(key);
    if ((key >= platform::KeyCode::A && key <= platform::KeyCode::Z) ||
        (key >= platform::KeyCode::Key0 && key <= platform::KeyCode::Key9))
    {
        // ASCII letters and digits already match the public HGVM key codes.
        return keyValue;
    }
    if (key >= platform::KeyCode::F1 && key <= platform::KeyCode::F12)
        return 290 + keyValue - uint32_t(platform::KeyCode::F1);

    switch (key)
    {
    case platform::KeyCode::Space:
        return 32;
    case platform::KeyCode::Quote:
        return 39;
    case platform::KeyCode::Comma:
        return 44;
    case platform::KeyCode::Minus:
        return 45;
    case platform::KeyCode::Dot:
        return 46;
    case platform::KeyCode::Slash:
        return 47;
    case platform::KeyCode::Semicolon:
        return 59;
    case platform::KeyCode::Plus:
        return 61;
    case platform::KeyCode::LBracket:
        return 91;
    case platform::KeyCode::Backslash:
        return 92;
    case platform::KeyCode::RBracket:
        return 93;
    case platform::KeyCode::Tilde:
        return 96;
    case platform::KeyCode::Escape:
        return 256;
    case platform::KeyCode::Return:
        return 257;
    case platform::KeyCode::Tab:
        return 258;
    case platform::KeyCode::Backspace:
        return 259;
    case platform::KeyCode::Insert:
        return 260;
    case platform::KeyCode::Delete:
        return 261;
    case platform::KeyCode::Right:
        return 262;
    case platform::KeyCode::Left:
        return 263;
    case platform::KeyCode::Down:
        return 264;
    case platform::KeyCode::Up:
        return 265;
    case platform::KeyCode::PageUp:
        return 266;
    case platform::KeyCode::PageDown:
        return 267;
    case platform::KeyCode::Home:
        return 268;
    case platform::KeyCode::End:
        return 269;
    case platform::KeyCode::Shift:
        return 340;
    case platform::KeyCode::Ctrl:
        return 341;
    case platform::KeyCode::Alt:
        return 342;
    default:
        return 0;
    }
}

struct NativeWindow : HGVMHostObject
{
    explicit NativeWindow(IHGVMHostServices* owner)
        : HGVMHostObject(HGVMHostObjectKind::Window, owner)
    {
    }

    ~NativeWindow()
    {
        if (window)
        {
            window->events = {};
            window->close();
        }
    }

    RefPtr<platform::Window> window;
    List<HGVMInputEvent> events;
    Index nextEventIndex = 0;
    bool open = true;
    uint32_t requestedWidth = 0;
    uint32_t requestedHeight = 0;
};

struct NativeSurface : HGVMHostObject
{
    explicit NativeSurface(IHGVMHostServices* owner)
        : HGVMHostObject(HGVMHostObjectKind::Surface, owner)
    {
    }

    RefPtr<NativeWindow> window;
    ComPtr<rhi::ISurface> surface;
    ComPtr<rhi::ITexture> acquiredTexture;
    uint32_t imageCount = 2;
    rhi::Format requestedFormat = rhi::Format::Undefined;
};

class NativeHostServices : public IHGVMHostServices
{
public:
    void getDebugProperties(HGVMHostObject* object, List<DebugObjectProperty>& properties) override
    {
        SLANG_RELEASE_ASSERT(object->getOwner() == this);
        if (object->getKind() == HGVMHostObjectKind::Window)
        {
            auto window = static_cast<NativeWindow*>(object);
            properties.add({"open", window->open ? "true" : "false"});
            if (window->open)
            {
                auto rect = window->window->getClientRect();
                properties.add({"size", String(rect.width) + " x " + String(rect.height)});
            }
        }
        else if (object->getKind() == HGVMHostObjectKind::Surface)
        {
            auto surface = static_cast<NativeSurface*>(object);
            if (const auto* config = surface->surface->getConfig())
            {
                properties.add({"size", String(config->width) + " x " + String(config->height)});
                properties.add({"format", rhi::getFormatInfo(config->format).name});
                properties.add({"images", String(config->desiredImageCount)});
                properties.add({"usage", getTextureUsageName(config->usage)});
            }
        }
    }

    explicit NativeHostServices(rhi::IDevice* device)
        : m_device(device), m_blitter(device)
    {
    }

    SlangResult createWindow(
        std::string_view title,
        uint32_t width,
        uint32_t height,
        RefPtr<HGVMHostObject>& outWindow,
        String& outError) override
    {
        if (!width || !height)
            return fail(outError, "HGVM native window dimensions must be non-zero");

        String titleString = makeString(title);
        platform::WindowDesc desc = {};
        desc.title = titleString.getBuffer();
        desc.width = int(width);
        desc.height = int(height);
        desc.style = platform::WindowStyle::Default;
        RefPtr<platform::Window> platformWindow = platform::Application::createWindow(desc);
        if (!platformWindow)
            return fail(outError, "The native platform failed to create an HGVM window");
        rhi::WindowHandle nativeHandle =
            platformWindow->getNativeHandle().convert<rhi::WindowHandle>();
        if (nativeHandle.type == rhi::WindowHandleType::Undefined || !nativeHandle.handleValues[0])
            return fail(outError, "The native platform returned an invalid HGVM window handle");

        RefPtr<NativeWindow> window = new NativeWindow(this);
        window->window = platformWindow;
        window->requestedWidth = width;
        window->requestedHeight = height;
        installEventHandlers(window);
        platformWindow->show();
        outWindow = window;
        return SLANG_OK;
    }

    SlangResult processWindowEvents(HGVMHostObject* value, bool& outOpen, String& outError) override
    {
        NativeWindow* window = getWindow(value, outError);
        if (!window)
            return SLANG_E_INVALID_ARG;
        platform::Application::doEvents();
        outOpen = window->open;
        return SLANG_OK;
    }

    SlangResult pollInputEvent(HGVMHostObject* value, HGVMInputEvent& outEvent, String& outError)
        override
    {
        NativeWindow* window = getWindow(value, outError);
        if (!window)
            return SLANG_E_INVALID_ARG;
        outEvent = {};
        if (window->nextEventIndex < window->events.getCount())
        {
            outEvent = window->events[window->nextEventIndex++];
            if (window->nextEventIndex == window->events.getCount())
            {
                window->events.clear();
                window->nextEventIndex = 0;
            }
        }
        return SLANG_OK;
    }

    SlangResult closeWindow(HGVMHostObject* value, String& outError) override
    {
        NativeWindow* window = getWindow(value, outError);
        if (!window)
            return SLANG_E_INVALID_ARG;
        window->open = false;
        if (window->window)
            window->window->close();
        return SLANG_OK;
    }

    SlangResult setWindowTitle(HGVMHostObject* value, std::string_view title, String& outError)
        override
    {
        NativeWindow* window = getWindow(value, outError);
        if (!window)
            return SLANG_E_INVALID_ARG;
        window->window->setText(makeString(title));
        return SLANG_OK;
    }

    SlangResult createSurface(
        HGVMHostObject* windowValue,
        rhi::Format requestedFormat,
        uint32_t imageCount,
        RefPtr<HGVMHostObject>& outSurface,
        String& outError) override
    {
        NativeWindow* window = getWindow(windowValue, outError);
        if (!window)
            return SLANG_E_INVALID_ARG;
        if (!m_device)
            return fail(outError, "HGVM native surface creation requires an RHI device");

        rhi::WindowHandle handle = window->window->getNativeHandle().convert<rhi::WindowHandle>();
        ComPtr<rhi::ISurface> rhiSurface = m_device->createSurface(handle);
        if (!rhiSurface)
            return fail(outError, "RHI failed to create the HGVM native surface");

        RefPtr<NativeSurface> surface = new NativeSurface(this);
        surface->window = window;
        surface->surface = rhiSurface;
        surface->requestedFormat = requestedFormat;
        surface->imageCount = imageCount ? imageCount : 2;
        platform::Rect rect = window->window->getClientRect();
        SlangResult result = configureSurface(
            surface,
            rect.width > 0 ? uint32_t(rect.width) : 0,
            rect.height > 0 ? uint32_t(rect.height) : 0,
            outError);
        if (result != SLANG_E_NOT_AVAILABLE)
            SLANG_RETURN_ON_FAIL(result);
        outSurface = surface;
        return SLANG_OK;
    }

    SlangResult resizeSurface(
        HGVMHostObject* value,
        uint32_t width,
        uint32_t height,
        String& outError) override
    {
        NativeSurface* surface = getSurface(value, outError);
        if (!surface)
            return SLANG_E_INVALID_ARG;
        if (surface->acquiredTexture)
            return fail(outError, "Cannot resize an HGVM surface with an acquired image");

        // HGVM has retired its submissions before entering this host operation. Waiting here
        // also releases the native command buffers that referenced the previous swapchain.
        ComPtr<rhi::ICommandQueue> queue = m_device->getQueue(rhi::QueueType::Graphics);
        if (!queue || SLANG_FAILED(queue->waitOnHost()))
            return fail(outError, "RHI failed to finish work before HGVM surface resize");
        return configureSurface(surface, width, height, outError);
    }

    SlangResult acquireSurfaceTexture(
        HGVMHostObject* value,
        ComPtr<rhi::ITexture>& outTexture,
        String& outError) override
    {
        NativeSurface* surface = getSurface(value, outError);
        if (!surface)
            return SLANG_E_INVALID_ARG;
        if (surface->acquiredTexture)
            return fail(outError, "The HGVM native surface image has already been acquired");

        platform::Rect rect = surface->window->window->getClientRect();
        if (rect.width <= 0 || rect.height <= 0)
            return SLANG_E_NOT_AVAILABLE;
        if (!surface->surface->getConfig())
            return SLANG_E_NOT_AVAILABLE;
        SlangResult result =
            surface->surface->acquireNextImage(surface->acquiredTexture.writeRef());
        if (result == SLANG_E_NOT_AVAILABLE)
            return result;
        if (SLANG_FAILED(result) || !surface->acquiredTexture)
            return fail(outError, "RHI failed to acquire the next HGVM native surface image");
        outTexture = surface->acquiredTexture;
        return SLANG_OK;
    }

    SlangResult presentSurface(HGVMHostObject* value, String& outError) override
    {
        NativeSurface* surface = getSurface(value, outError);
        if (!surface)
            return SLANG_E_INVALID_ARG;
        if (!surface->acquiredTexture)
            return fail(outError, "No HGVM native surface image is ready to present");
        SlangResult result = surface->surface->present();
        surface->acquiredTexture = nullptr;
        // A resize can invalidate the swapchain after acquisition. The frame was discarded;
        // the application must explicitly resize the surface before trying again.
        if (result == SLANG_E_NOT_AVAILABLE)
            return result;
        if (SLANG_FAILED(result))
            return fail(outError, "RHI failed to present the HGVM native surface image");
        return SLANG_OK;
    }

    SlangResult blitTexture2D(rhi::ITexture* destination, rhi::ITexture* source, String& outError)
        override
    {
        return m_blitter.blit(destination, source, outError);
    }

private:
    void installEventHandlers(NativeWindow* window)
    {
        window->window->events.close = [window]() { window->open = false; };
        window->window->events.sizeChanged = [window]()
        {
            platform::Rect rect = window->window->getClientRect();
            HGVMInputEvent event;
            event.type = HGVMInputEventType::WindowResize;
            event.size[0] = rect.width > 0 ? uint32_t(rect.width) : 0;
            event.size[1] = rect.height > 0 ? uint32_t(rect.height) : 0;

            // Interactive resizing may enqueue many platform notifications before the graph polls
            // them. Only the newest extent in a consecutive run of resize events is observable.
            if (window->events.getCount() > window->nextEventIndex &&
                window->events.getLast().type == HGVMInputEventType::WindowResize)
            {
                window->events.getLast() = event;
            }
            else
            {
                window->events.add(event);
            }
        };
        window->window->events.keyDown = [window](platform::KeyEventArgs& args)
        {
            uint32_t key = translateKeyCode(args.key);
            if (!key)
                return;
            HGVMInputEvent event;
            event.type = HGVMInputEventType::KeyPress;
            event.key = key;
            window->events.add(event);
        };
        window->window->events.keyUp = [window](platform::KeyEventArgs& args)
        {
            uint32_t key = translateKeyCode(args.key);
            if (!key)
                return;
            HGVMInputEvent event;
            event.type = HGVMInputEventType::KeyRelease;
            event.key = key;
            window->events.add(event);
        };
        window->window->events.mouseDown = [window](platform::MouseEventArgs args)
        {
            HGVMInputEvent event;
            if (translateMouseButtonEvent(args, true, event))
                window->events.add(event);
        };
        window->window->events.mouseUp = [window](platform::MouseEventArgs args)
        {
            HGVMInputEvent event;
            if (translateMouseButtonEvent(args, false, event))
                window->events.add(event);
        };
        window->window->events.mouseMove = [window](platform::MouseEventArgs args)
        {
            HGVMInputEvent event;
            event.type = HGVMInputEventType::MouseMove;
            event.position[0] = float(args.x);
            event.position[1] = float(args.y);
            window->events.add(event);
        };
        window->window->events.mouseEnter = [window](platform::MouseEventArgs args)
        {
            HGVMInputEvent event;
            event.type = HGVMInputEventType::MouseEnter;
            event.position[0] = float(args.x);
            event.position[1] = float(args.y);
            window->events.add(event);
        };
        window->window->events.mouseLeave = [window]()
        {
            HGVMInputEvent event;
            event.type = HGVMInputEventType::MouseLeave;
            window->events.add(event);
        };
        window->window->events.mouseCaptureLost = [window]()
        {
            HGVMInputEvent event;
            event.type = HGVMInputEventType::MouseCaptureLost;
            window->events.add(event);
        };
        window->window->events.focus = [window]()
        {
            HGVMInputEvent event;
            event.type = HGVMInputEventType::WindowFocusGained;
            window->events.add(event);
        };
        window->window->events.lostFocus = [window]()
        {
            HGVMInputEvent event;
            event.type = HGVMInputEventType::WindowFocusLost;
            window->events.add(event);
        };
        window->window->events.mouseWheel = [window](platform::MouseEventArgs args)
        {
            HGVMInputEvent event;
            event.type = HGVMInputEventType::MouseScroll;
            event.scroll[1] = float(args.delta) / 120.0f;
            window->events.add(event);
        };
    }

    NativeWindow* getWindow(HGVMHostObject* value, String& outError)
    {
        if (!value || value->getKind() != HGVMHostObjectKind::Window || value->getOwner() != this)
        {
            outError = "The HGVM Window belongs to a different native host";
            return nullptr;
        }
        return static_cast<NativeWindow*>(value);
    }

    NativeSurface* getSurface(HGVMHostObject* value, String& outError)
    {
        if (!value || value->getKind() != HGVMHostObjectKind::Surface || value->getOwner() != this)
        {
            outError = "The HGVM Surface belongs to a different native host";
            return nullptr;
        }
        return static_cast<NativeSurface*>(value);
    }

    SlangResult configureSurface(
        NativeSurface* surface,
        uint32_t width,
        uint32_t height,
        String& outError)
    {
        platform::Rect rect = surface->window->window->getClientRect();
        if (!width || !height || rect.width <= 0 || rect.height <= 0)
            return SLANG_E_NOT_AVAILABLE;
        const rhi::SurfaceInfo& info = surface->surface->getInfo();
        rhi::Format selectedFormat = selectSurfaceFormat(info, surface->requestedFormat);
        if (selectedFormat == rhi::Format::Undefined)
            return fail(outError, "The requested HGVM surface format is unavailable");

        rhi::SurfaceConfig config = {};
        config.format = selectedFormat;
        config.usage = rhi::TextureUsage::RenderTarget;
        config.width = width;
        config.height = height;
        config.desiredImageCount = surface->imageCount;
        config.vsync = true;
        SlangResult result = surface->surface->configure(config);
        if (result == SLANG_E_NOT_AVAILABLE)
            return result;
        if (SLANG_FAILED(result))
            return fail(outError, "RHI failed to configure the HGVM native surface");
        return SLANG_OK;
    }

    ComPtr<rhi::IDevice> m_device;
    TextureBlitter m_blitter;
};

struct PlatformScope
{
    PlatformScope() { platform::Application::init(); }
    ~PlatformScope() { platform::Application::dispose(); }
};

rhi::DeviceType getRhiDeviceType(RenderApiType api)
{
    switch (api)
    {
    case RenderApiType::Unknown:
        return rhi::DeviceType::Default;
    case RenderApiType::Vulkan:
        return rhi::DeviceType::Vulkan;
    case RenderApiType::D3D12:
        return rhi::DeviceType::D3D12;
    case RenderApiType::D3D11:
        return rhi::DeviceType::D3D11;
    case RenderApiType::Metal:
        return rhi::DeviceType::Metal;
    case RenderApiType::CPU:
    case RenderApiType::LLVM:
        return rhi::DeviceType::CPU;
    case RenderApiType::CUDA:
        return rhi::DeviceType::CUDA;
    case RenderApiType::WebGPU:
        return rhi::DeviceType::WGPU;
    case RenderApiType::CountOf:
        break;
    }
    SLANG_RELEASE_ASSERT(!"Invalid render API");
    return rhi::DeviceType::Default;
}

} // namespace

struct NativeHGVMEnvironment::Impl
{
    NativeDebugCallback debugCallback;
    std::unique_ptr<PlatformScope> platformScope;
    ComPtr<rhi::IDevice> device;
    RefPtr<NativeHostServices> host;
};

NativeHGVMEnvironment::NativeHGVMEnvironment()
    : m_impl(new Impl)
{
}

NativeHGVMEnvironment::~NativeHGVMEnvironment() = default;

SlangResult NativeHGVMEnvironment::waitForIdle(String& outError)
{
    if (!m_impl->device)
        return fail(outError, "Native HGVM environment has no RHI device");
    ComPtr<rhi::ICommandQueue> queue = m_impl->device->getQueue(rhi::QueueType::Graphics);
    if (!queue || SLANG_FAILED(queue->waitOnHost()))
        return fail(outError, "Failed to wait for native HGVM work to complete");
    return SLANG_OK;
}

SlangResult NativeHGVMEnvironment::create(
    RenderApiType api,
    SlangDiagnosticColor diagnosticColor,
    SlangOptimizationLevel shaderOptimizationLevel,
    RefPtr<NativeHGVMEnvironment>& outEnvironment,
    String& outError,
    bool reportCheckpointIntermediates)
{
    outEnvironment = nullptr;
    RefPtr<NativeHGVMEnvironment> environment = new NativeHGVMEnvironment;
    environment->m_impl->platformScope = std::make_unique<PlatformScope>();
    rhi::DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = getRhiDeviceType(api);
    deviceDesc.debugCallback = &environment->m_impl->debugCallback;
    slang::CompilerOptionEntry compilerOptions[4] = {};
    compilerOptions[0].name = slang::CompilerOptionName::DiagnosticColor;
    compilerOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[0].value.intValue0 = diagnosticColor;
    // Shader programs compiled on demand by HGVM inherit the same experimental-language policy
    // as the host graph that submitted them.
    compilerOptions[1].name = slang::CompilerOptionName::ExperimentalFeature;
    compilerOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[1].value.intValue0 = 1;
    // HGVM dispatches compile through this device-owned Slang session. Keeping the optimization
    // option here lets callers tune downstream shaders without changing the HGBC host program.
    compilerOptions[2].name = slang::CompilerOptionName::Optimization;
    compilerOptions[2].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[2].value.intValue0 = shaderOptimizationLevel;
    compilerOptions[3].name = slang::CompilerOptionName::ReportCheckpointIntermediates;
    compilerOptions[3].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[3].value.intValue0 = reportCheckpointIntermediates;
    deviceDesc.slang.compilerOptionEntries = compilerOptions;
    deviceDesc.slang.compilerOptionEntryCount = SLANG_COUNT_OF(compilerOptions);
    environment->m_impl->device = rhi::getRHI()->createDevice(deviceDesc);
    if (!environment->m_impl->device)
    {
        UnownedStringSlice apiName =
            api == RenderApiType::Unknown ? toSlice("default") : RenderApiUtil::getApiName(api);
        String message =
            String("Failed to create the '") + apiName + "' RHI device for native HGVM execution";
        SlangResult result = fail(outError, message.getBuffer());
        SlangITool::prependDebugMessages(outError, environment->m_impl->debugCallback.messages);
        return result;
    }

    environment->m_impl->host = new NativeHostServices(environment->m_impl->device);
    outEnvironment = std::move(environment);
    return SLANG_OK;
}

struct NativeHGVMRuntime::Impl
{
    RefPtr<NativeHGVMEnvironment> environment;
    RefPtr<Program> program;
    RefPtr<Slang::HGVirtualMachine> virtualMachine;
    RefPtr<DebugSession> debugSession;
};

NativeHGVMRuntime::NativeHGVMRuntime()
    : m_impl(new Impl)
{
}

NativeHGVMRuntime::~NativeHGVMRuntime()
{
    if (m_impl->virtualMachine && m_impl->program)
        m_impl->virtualMachine->releaseProgram(m_impl->program);
}

SlangResult NativeHGVMRuntime::create(
    slang::IBlob* code,
    const char* programName,
    const char* workingDirectory,
    Slang::HGVM::GPUExecutionMode gpuExecution,
    IHGVMOutputSink* outputSink,
    SlangDiagnosticColor diagnosticColor,
    RefPtr<NativeHGVMRuntime>& outRuntime,
    String& outError,
    IEventSink* eventSink,
    bool enableDebugging)
{
    return create(
        code,
        programName,
        workingDirectory,
        gpuExecution,
        outputSink,
        RenderApiType::Unknown,
        diagnosticColor,
        SLANG_OPTIMIZATION_LEVEL_DEFAULT,
        outRuntime,
        outError,
        eventSink,
        enableDebugging);
}

SlangResult NativeHGVMRuntime::create(
    slang::IBlob* code,
    const char* programName,
    const char* workingDirectory,
    Slang::HGVM::GPUExecutionMode gpuExecution,
    IHGVMOutputSink* outputSink,
    RenderApiType api,
    SlangDiagnosticColor diagnosticColor,
    SlangOptimizationLevel shaderOptimizationLevel,
    RefPtr<NativeHGVMRuntime>& outRuntime,
    String& outError,
    IEventSink* eventSink,
    bool enableDebugging,
    bool reportCheckpointIntermediates)
{
    RefPtr<NativeHGVMEnvironment> environment;
    SLANG_RETURN_ON_FAIL(NativeHGVMEnvironment::create(
        api,
        diagnosticColor,
        shaderOptimizationLevel,
        environment,
        outError,
        reportCheckpointIntermediates));
    return create(
        environment,
        code,
        programName,
        workingDirectory,
        gpuExecution,
        outputSink,
        outRuntime,
        outError,
        eventSink,
        enableDebugging);
}

SlangResult NativeHGVMRuntime::create(
    NativeHGVMEnvironment* environment,
    slang::IBlob* code,
    const char* programName,
    const char* workingDirectory,
    Slang::HGVM::GPUExecutionMode gpuExecution,
    IHGVMOutputSink* outputSink,
    RefPtr<NativeHGVMRuntime>& outRuntime,
    String& outError,
    IEventSink* eventSink,
    bool enableDebugging)
{
    outRuntime = nullptr;
    if (!environment || !environment->m_impl->device || !environment->m_impl->host || !code)
        return fail(outError, "Native HGVM runtime creation requires an environment and code");

    environment->m_impl->debugCallback.messages.clear();
    RefPtr<NativeHGVMRuntime> runtime = new NativeHGVMRuntime;
    runtime->m_impl->environment = environment;
    SLANG_RETURN_ON_FAIL(Program::load(
        code->getBufferPointer(),
        code->getBufferSize(),
        programName ? programName : "slang-hgvm",
        runtime->m_impl->program,
        outError));

    Desc desc = {};
    desc.device = environment->m_impl->device;
    desc.fileSystem = OSFileSystem::getMutableSingleton();
    desc.workingDirectory = workingDirectory;
    desc.gpuExecution = gpuExecution;
    desc.hostServices = environment->m_impl->host;
    desc.outputSink = outputSink;
    desc.enableEventTracking = eventSink != nullptr;
    desc.eventSink = eventSink;
    if (enableDebugging)
    {
        runtime->m_impl->debugSession = new DebugSession(desc);
        runtime->m_impl->virtualMachine = runtime->m_impl->debugSession->getInterpreter();
    }
    else
        runtime->m_impl->virtualMachine = new Slang::HGVirtualMachine(desc);
    outRuntime = std::move(runtime);
    return SLANG_OK;
}

Program* NativeHGVMRuntime::getProgram() const
{
    return m_impl->program;
}

Slang::HGVirtualMachine* NativeHGVMRuntime::getVirtualMachine() const
{
    return m_impl->virtualMachine;
}

DebugSession* NativeHGVMRuntime::getDebugSession() const
{
    return m_impl->debugSession;
}

void NativeHGVMRuntime::prependDebugMessages(String& ioError) const
{
    SlangITool::prependDebugMessages(ioError, m_impl->environment->m_impl->debugCallback.messages);
}

SlangResult executeNativeHGBCProgram(
    slang::IBlob* code,
    const char* programName,
    const char* workingDirectory,
    const char* entryPointName,
    Slang::HGVM::GPUExecutionMode gpuExecution,
    RenderApiType api,
    SlangDiagnosticColor diagnosticColor,
    SlangOptimizationLevel shaderOptimizationLevel,
    String& outError,
    IEventSink* eventSink,
    DebugSessionRunner runDebugger,
    IHGVMOutputSink* outputSink,
    bool reportCheckpointIntermediates)
{
    if (!code || !entryPointName)
        return fail(outError, "Native HGVM execution requires code and an entry-point name");

    RefPtr<NativeHGVMRuntime> runtime;
    SLANG_RETURN_ON_FAIL(NativeHGVMRuntime::create(
        code,
        programName,
        workingDirectory,
        gpuExecution,
        outputSink,
        api,
        diagnosticColor,
        shaderOptimizationLevel,
        runtime,
        outError,
        eventSink,
        runDebugger != nullptr,
        reportCheckpointIntermediates));
    SlangResult callResult =
        runDebugger
            ? runDebugger(
                  runtime->getDebugSession(),
                  runtime->getProgram(),
                  entryPointName,
                  outError)
            : runtime->getVirtualMachine()->call(runtime->getProgram(), entryPointName, outError);
    if (SLANG_FAILED(callResult))
    {
        runtime->prependDebugMessages(outError);
        return callResult;
    }

    // RHI delivers successful shader compiler diagnostics through its debug callback too.
    // Checkpoint reports must reach the caller even when execution succeeds.
    if (reportCheckpointIntermediates)
        runtime->prependDebugMessages(outError);
    return SLANG_OK;
}

} // namespace SlangITool

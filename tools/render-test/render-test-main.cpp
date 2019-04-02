// render-test-main.cpp

#include "options.h"
#include "render.h"
#include "render-d3d11.h"
#include "render-d3d12.h"
#include "render-gl.h"
#include "render-vk.h"

#include "slang-support.h"
#include "surface.h"
#include "png-serialize-util.h"

#include "shader-renderer-util.h"

#include "shader-input-layout.h"
#include <stdio.h>
#include <stdlib.h>

#include "../../source/core/slang-test-tool-util.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

namespace renderer_test {

using Slang::Result;

int gWindowWidth = 1024;
int gWindowHeight = 768;

class Window: public RefObject
{
public:
    SlangResult initialize(int width, int height);

    void show();

    void* getHandle() const { return m_hwnd; }

    Window() {}
    ~Window();

    static LRESULT CALLBACK windowProc(HWND    windowHandle,
        UINT    message,
        WPARAM  wParam,
        LPARAM  lParam);

protected:

    HINSTANCE m_hinst = nullptr;
    HWND m_hwnd = nullptr;
};

//
// We use a bare-minimum window procedure to get things up and running.
//

/* static */LRESULT CALLBACK Window::windowProc(
    HWND    windowHandle,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(windowHandle, message, wParam, lParam);
}

static ATOM _getWindowClassAtom(HINSTANCE hinst)
{
    static ATOM s_windowClassAtom;

    if (s_windowClassAtom)
    {
        return s_windowClassAtom;
    }
    WNDCLASSEXW windowClassDesc;
    windowClassDesc.cbSize = sizeof(windowClassDesc);
    windowClassDesc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClassDesc.lpfnWndProc = &Window::windowProc;
    windowClassDesc.cbClsExtra = 0;
    windowClassDesc.cbWndExtra = 0;
    windowClassDesc.hInstance = hinst;
    windowClassDesc.hIcon = 0;
    windowClassDesc.hCursor = 0;
    windowClassDesc.hbrBackground = 0;
    windowClassDesc.lpszMenuName = 0;
    windowClassDesc.lpszClassName = L"SlangRenderTest";
    windowClassDesc.hIconSm = 0;
    s_windowClassAtom = RegisterClassExW(&windowClassDesc);
        
    return s_windowClassAtom;
}

SlangResult Window::initialize(int widthIn, int heightIn)
{
    // Do initial window-creation stuff here, rather than in the renderer-specific files

    m_hinst = GetModuleHandleA(0);

    // First we register a window class.
    ATOM windowClassAtom = _getWindowClassAtom(m_hinst);
    if (!windowClassAtom)
    {
        fprintf(stderr, "error: failed to register window class\n");
        return SLANG_FAIL;
    }

    // Next, we create a window using that window class.

    // We will create a borderless window since our screen-capture logic in GL
    // seems to get thrown off by having to deal with a window frame.
    DWORD windowStyle = WS_POPUP;
    DWORD windowExtendedStyle = 0;

    RECT windowRect = { 0, 0, widthIn, heightIn };
    AdjustWindowRectEx(&windowRect, windowStyle, /*hasMenu=*/false, windowExtendedStyle);

    {
        auto width = windowRect.right - windowRect.left;
        auto height = windowRect.bottom - windowRect.top;

        LPWSTR windowName = L"Slang Render Test";
        m_hwnd = CreateWindowExW(
            windowExtendedStyle,
            (LPWSTR)windowClassAtom,
            windowName,
            windowStyle,
            0, 0, // x, y
            width, height,
            NULL, // parent
            NULL, // menu
            m_hinst,
            NULL);
    }
    if (!m_hwnd)
    {
        fprintf(stderr, "error: failed to create window\n");
        return SLANG_FAIL;
    }

    return SLANG_OK;
}


void Window::show()
{
    // Once initialization is all complete, we show the window...
    int showCommand = SW_SHOW;
    ShowWindow(m_hwnd, showCommand);
}

Window::~Window()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
    }
}

//
// For the purposes of a small example, we will define the vertex data for a
// single triangle directly in the source file. It should be easy to extend
// this example to load data from an external source, if desired.
//

struct Vertex
{
    float position[3];
    float color[3];
    float uv[2];
};

static const Vertex kVertexData[] =
{
    { { 0,  0, 0.5 }, {1, 0, 0} , {0, 0} },
    { { 0,  1, 0.5 }, {0, 0, 1} , {1, 0} },
    { { 1,  0, 0.5 }, {0, 1, 0} , {1, 1} },
};
static const int kVertexCount = SLANG_COUNT_OF(kVertexData);

using namespace Slang;

class RenderTestApp
{
	public:

		// At initialization time, we are going to load and compile our Slang shader
		// code, and then create the API objects we need for rendering.
	Result initialize(Renderer* renderer, ShaderCompiler* shaderCompiler);
	void runCompute();
	void renderFrame();
	void finalize();

	BindingStateImpl* getBindingState() const { return m_bindingState; }

    Result writeBindingOutput(const char* fileName);

    Result writeScreen(const char* filename);

	protected:
		/// Called in initialize
	Result initializeShaders(ShaderCompiler* shaderCompiler);

	// variables for state to be used for rendering...
	uintptr_t m_constantBufferSize, m_computeResultBufferSize;

	RefPtr<Renderer> m_renderer;

	RefPtr<BufferResource>	m_constantBuffer;
	RefPtr<InputLayout>     m_inputLayout;
	RefPtr<BufferResource>  m_vertexBuffer;
	RefPtr<ShaderProgram>   m_shaderProgram;
    RefPtr<PipelineState>   m_pipelineState;
	RefPtr<BindingStateImpl>    m_bindingState;

	ShaderInputLayout m_shaderInputLayout;              ///< The binding layout
    int m_numAddedConstantBuffers;                      ///< Constant buffers can be added to the binding directly. Will be added at the end.
};

// Entry point name to use for vertex/fragment shader
static const char vertexEntryPointName[]    = "vertexMain";
static const char fragmentEntryPointName[]  = "fragmentMain";
static const char computeEntryPointName[]	= "computeMain";

SlangResult RenderTestApp::initialize(Renderer* renderer, ShaderCompiler* shaderCompiler)
{
    SLANG_RETURN_ON_FAIL(initializeShaders(shaderCompiler));

    m_numAddedConstantBuffers = 0;
	m_renderer = renderer;

    // TODO(tfoley): use each API's reflection interface to query the constant-buffer size needed
    {
        m_constantBufferSize = 16 * sizeof(float);

        BufferResource::Desc constantBufferDesc;
        constantBufferDesc.init(m_constantBufferSize);
        constantBufferDesc.cpuAccessFlags = Resource::AccessFlag::Write;

        m_constantBuffer = renderer->createBufferResource(Resource::Usage::ConstantBuffer, constantBufferDesc);
        if (!m_constantBuffer)
            return SLANG_FAIL;
    }

    {
        //! Hack -> if doing a graphics test, add an extra binding for our dynamic constant buffer
        //
        // TODO: Should probably be more sophisticated than this - with 'dynamic' constant buffer/s binding always being specified
        // in the test file
        RefPtr<BufferResource> addedConstantBuffer;
        switch(gOptions.shaderType)
        {
        default:
            break;

        case Options::ShaderProgramType::Graphics:
        case Options::ShaderProgramType::GraphicsCompute:
            addedConstantBuffer = m_constantBuffer;
            m_numAddedConstantBuffers++;
            break;
        }

        BindingStateImpl* bindingState = nullptr;
        SLANG_RETURN_ON_FAIL(ShaderRendererUtil::createBindingState(m_shaderInputLayout, m_renderer, addedConstantBuffer, &bindingState));
        m_bindingState = bindingState;
    }

    // Do other initialization that doesn't depend on the source language.

    // Input Assembler (IA)

    const InputElementDesc inputElements[] = {
        { "A", 0, Format::RGB_Float32, offsetof(Vertex, position) },
        { "A", 1, Format::RGB_Float32, offsetof(Vertex, color) },
        { "A", 2, Format::RG_Float32, offsetof(Vertex, uv) },
    };

    m_inputLayout = renderer->createInputLayout(inputElements, SLANG_COUNT_OF(inputElements));
    if(!m_inputLayout)
        return SLANG_FAIL;

    BufferResource::Desc vertexBufferDesc;
    vertexBufferDesc.init(kVertexCount * sizeof(Vertex));

    m_vertexBuffer = renderer->createBufferResource(Resource::Usage::VertexBuffer, vertexBufferDesc, kVertexData);
    if(!m_vertexBuffer)
        return SLANG_FAIL;

    {
        switch(gOptions.shaderType)
        {
        default:
            assert(!"unexpected test shader type");
            return SLANG_FAIL;

        case Options::ShaderProgramType::Compute:
            {
                ComputePipelineStateDesc desc;
                desc.pipelineLayout = m_bindingState->pipelineLayout;
                desc.program = m_shaderProgram;

                m_pipelineState = renderer->createComputePipelineState(desc);
            }
            break;

        case Options::ShaderProgramType::Graphics:
        case Options::ShaderProgramType::GraphicsCompute:
            {
                GraphicsPipelineStateDesc desc;
                desc.pipelineLayout = m_bindingState->pipelineLayout;
                desc.program = m_shaderProgram;
                desc.inputLayout = m_inputLayout;
                desc.renderTargetCount = m_bindingState->m_numRenderTargets;

                m_pipelineState = renderer->createGraphicsPipelineState(desc);
            }
            break;
        }
    }

    // If success must have a pipeline state
    return m_pipelineState ? SLANG_OK : SLANG_FAIL;
}

Result RenderTestApp::initializeShaders(ShaderCompiler* shaderCompiler)
{
	// Read in the source code
	char const* sourcePath = gOptions.sourcePath;
	FILE* sourceFile = fopen(sourcePath, "rb");
	if (!sourceFile)
	{
		fprintf(stderr, "error: failed to open '%s' for reading\n", sourcePath);
		return SLANG_FAIL;
	}
	fseek(sourceFile, 0, SEEK_END);
	size_t sourceSize = ftell(sourceFile);
	fseek(sourceFile, 0, SEEK_SET);

    List<char> sourceText;
    sourceText.SetSize(sourceSize + 1);
	fread(sourceText.Buffer(), sourceSize, 1, sourceFile);
	fclose(sourceFile);
	sourceText[sourceSize] = 0;

    switch( gOptions.shaderType )
    {
    default:
        m_shaderInputLayout.numRenderTargets = 1;
        break;

    case Options::ShaderProgramType::Compute:
        m_shaderInputLayout.numRenderTargets = 0;
        break;
    }
	m_shaderInputLayout.Parse(sourceText.Buffer());

	ShaderCompileRequest::SourceInfo sourceInfo;
	sourceInfo.path = sourcePath;
	sourceInfo.dataBegin = sourceText.Buffer();
	sourceInfo.dataEnd = sourceText.Buffer() + sourceSize;

	ShaderCompileRequest compileRequest;
	compileRequest.source = sourceInfo;
	if (gOptions.shaderType == Options::ShaderProgramType::Graphics || gOptions.shaderType == Options::ShaderProgramType::GraphicsCompute)
	{
		compileRequest.vertexShader.source = sourceInfo;
		compileRequest.vertexShader.name = vertexEntryPointName;
		compileRequest.fragmentShader.source = sourceInfo;
		compileRequest.fragmentShader.name = fragmentEntryPointName;
	}
	else
	{
		compileRequest.computeShader.source = sourceInfo;
		compileRequest.computeShader.name = computeEntryPointName;
	}
	compileRequest.globalGenericTypeArguments = m_shaderInputLayout.globalGenericTypeArguments;
	compileRequest.entryPointGenericTypeArguments = m_shaderInputLayout.entryPointGenericTypeArguments;
	compileRequest.globalExistentialTypeArguments = m_shaderInputLayout.globalExistentialTypeArguments;
	compileRequest.entryPointExistentialTypeArguments = m_shaderInputLayout.entryPointExistentialTypeArguments;
	m_shaderProgram = shaderCompiler->compileProgram(compileRequest);

    return m_shaderProgram ? SLANG_OK : SLANG_FAIL;
}

void RenderTestApp::renderFrame()
{
    auto mappedData = m_renderer->map(m_constantBuffer, MapFlavor::WriteDiscard);
    if(mappedData)
    {
        const ProjectionStyle projectionStyle = RendererUtil::getProjectionStyle(m_renderer->getRendererType());
        RendererUtil::getIdentityProjection(projectionStyle, (float*)mappedData);

		m_renderer->unmap(m_constantBuffer);
    }

    auto pipelineType = PipelineType::Graphics;

    m_renderer->setPipelineState(pipelineType, m_pipelineState);

	m_renderer->setPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_renderer->setVertexBuffer(0, m_vertexBuffer, sizeof(Vertex));

    m_bindingState->apply(m_renderer, pipelineType);

	m_renderer->draw(3);
}

void RenderTestApp::runCompute()
{
    auto pipelineType = PipelineType::Compute;
    m_renderer->setPipelineState(pipelineType, m_pipelineState);
    m_bindingState->apply(m_renderer, pipelineType);
	m_renderer->dispatchCompute(1, 1, 1);
}

void RenderTestApp::finalize()
{
}

Result RenderTestApp::writeBindingOutput(const char* fileName)
{
    // Submit the work
    m_renderer->submitGpuWork();
    // Wait until everything is complete
    m_renderer->waitForGpu();

    FILE * f = fopen(fileName, "wb");
    if (!f)
    {
        return SLANG_FAIL;
    }

    for(auto binding : m_bindingState->outputBindings)
    {
        auto i = binding.entryIndex;
        const auto& layoutBinding = m_shaderInputLayout.entries[i];

        assert(layoutBinding.isOutput);
        {
            if (binding.resource && binding.resource->isBuffer())
            {
                BufferResource* bufferResource = static_cast<BufferResource*>(binding.resource.Ptr());
                const size_t bufferSize = bufferResource->getDesc().sizeInBytes;

                unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
                if (!ptr)
                {
                    fclose(f);
                    return SLANG_FAIL;
                }

                const int size = int(bufferSize / sizeof(unsigned int));
                for (int i = 0; i < size; ++i)
                {
                    fprintf(f, "%X\n", ptr[i]);
                }
                m_renderer->unmap(bufferResource);
            }
            else
            {
                printf("invalid output type at %d.\n", int(i));
            }
        }
    }
    fclose(f);

    return SLANG_OK;
}


Result RenderTestApp::writeScreen(const char* filename)
{
    Surface surface;
    SLANG_RETURN_ON_FAIL(m_renderer->captureScreenSurface(surface));
    return PngSerializeUtil::write(filename, surface);
}

static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_Slang_ITestTool = SLANG_UUID_Slang_ITestTool;

class RenderTestTool : public ITestTool
{
public:
    typedef RenderTestTool ThisType;
    SLANG_IUNKNOWN_QUERY_INTERFACE
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }

    // ITestTool
    SLANG_NO_THROW SlangResult SLANG_MCALL run(StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL calcTestRequirements(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv, TestRequirements* ioRequirements) SLANG_OVERRIDE;

    static ThisType* getSingleton() { static ThisType s_singleton; return &s_singleton; }

private:
    RenderTestTool() {}

    ISlangUnknown* getInterface(const Guid& guid);
};

ISlangUnknown* RenderTestTool::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_Slang_ITestTool) ? static_cast<ITestTool*>(this) : nullptr;
}

struct CommandLineInfo
{
    SlangSourceLanguage sourceLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
    SlangSourceLanguage nativeLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
    SlangCompileTarget target = SLANG_TARGET_NONE;
    SlangPassThrough passThrough = SLANG_PASS_THROUGH_NONE;
    char const* profileName = "";
};

static SlangResult _parseCommandLine(Slang::StdWriters* stdWriters, int argcIn, const char*const* argvIn, CommandLineInfo* outCommandLineInfo )
{
    // Parse command-line options
    SLANG_RETURN_ON_FAIL(parseOptions(argcIn, argvIn, stdWriters->getError()));

    SlangSourceLanguage nativeLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
    SlangCompileTarget target = SLANG_TARGET_NONE;
    SlangPassThrough passThrough = SLANG_PASS_THROUGH_NONE;
    char const* profileName = "";

    switch (gOptions.rendererType)
    {
        case RenderApiType::D3D11:
            target = SLANG_DXBC;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            passThrough = SLANG_PASS_THROUGH_FXC;
            profileName = "sm_5_0";
            break;

        case RenderApiType::D3D12:
            target = SLANG_DXBC;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            passThrough = SLANG_PASS_THROUGH_FXC;
            profileName = "sm_5_0";
            if (gOptions.useDXIL)
            {
                target = SLANG_DXIL;
                passThrough = SLANG_PASS_THROUGH_DXC;
                profileName = "sm_6_0";
            }
            break;

        case RenderApiType::OpenGl:
            target = SLANG_GLSL;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            passThrough = SLANG_PASS_THROUGH_GLSLANG;
            profileName = "glsl_430";
            break;

        case RenderApiType::Vulkan:
            target = SLANG_SPIRV;
            nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            passThrough = SLANG_PASS_THROUGH_GLSLANG;
            profileName = "glsl_430";
            break;

        default:
            stdWriters->getError().put("error: unexpected\n");
            return SLANG_FAIL;
    }

    // Use the profile name set on options if set
    profileName = gOptions.profileName ? gOptions.profileName : profileName;

    SlangSourceLanguage sourceLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;

    switch (gOptions.inputLanguageID)
    {
        case Options::InputLanguageID::Slang:
            sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
            passThrough = SLANG_PASS_THROUGH_NONE;
            break;

        case Options::InputLanguageID::Native:
            sourceLanguage = nativeLanguage;
            //passThrough = passThrough;
            break;

        default:
            break;
    }

    outCommandLineInfo->target = target;
    outCommandLineInfo->sourceLanguage = sourceLanguage;
    outCommandLineInfo->nativeLanguage = nativeLanguage;
    outCommandLineInfo->passThrough = passThrough;
    outCommandLineInfo->profileName = profileName;

    return SLANG_OK;
}

static Renderer* _createRenderer(RenderApiType type)
{
    switch (gOptions.rendererType)
    {
        case RenderApiType::D3D11:      return createD3D11Renderer();
        case RenderApiType::D3D12:      return createD3D12Renderer();
        case RenderApiType::OpenGl:     return createGLRenderer();
        case RenderApiType::Vulkan:     return createVKRenderer();
        default: return nullptr;
    }
}

SlangResult RenderTestTool::run(Slang::StdWriters* stdWriters, SlangSession* session, int argcIn, const char*const* argvIn)
{
    GlobalWriters::setSingleton(stdWriters);

    CommandLineInfo commandLineInfo;
    SLANG_RETURN_ON_FAIL(_parseCommandLine(stdWriters, argcIn, argvIn, &commandLineInfo ));

    RefPtr<renderer_test::Window> window(new renderer_test::Window);
    SLANG_RETURN_ON_FAIL(window->initialize(gWindowWidth, gWindowHeight));

    StringBuilder rendererName;
    rendererName << "[" << RendererUtil::toText(gOptions.rendererType) << "] ";
    if (gOptions.adapter.Length())
    {
        rendererName << "'" << gOptions.adapter << "'";
    }

    // Create the renderer
    Slang::RefPtr<Renderer> renderer = _createRenderer(gOptions.rendererType);
    if (!renderer)
    {
        if (!gOptions.onlyStartup)
        {
            fprintf(stderr, "Unable to create renderer %s\n", rendererName.Buffer());
        }
        return SLANG_FAIL;
    }

    Renderer::Desc desc;
    desc.width = gWindowWidth;
    desc.height = gWindowHeight;
    desc.adapter = gOptions.adapter;

    {
        SlangResult res = renderer->initialize(desc, (HWND)window->getHandle());
        if (SLANG_FAILED(res))
        {
            if (!gOptions.onlyStartup)
            {
                fprintf(stderr, "Unable to initialize renderer %s\n", rendererName.Buffer());
            }
            return res;
        }
    }

    // If the only test is we can startup, then we are done
    if (gOptions.onlyStartup)
    {
        return SLANG_OK;
    }

    {
        for (const auto& feature : gOptions.renderFeatures)
        {
            // If doesn't have required feature... we have to give up
            if (!renderer->hasFeature(feature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }
    }

    ShaderCompiler shaderCompiler;
    shaderCompiler.renderer = renderer;

    shaderCompiler.target = commandLineInfo.target;
    shaderCompiler.profile = commandLineInfo.profileName;
    shaderCompiler.slangSession = session;
    shaderCompiler.sourceLanguage = commandLineInfo.sourceLanguage;
    shaderCompiler.passThrough = commandLineInfo.passThrough;
    
    {
        RenderTestApp app;

        SLANG_RETURN_ON_FAIL(app.initialize(renderer, &shaderCompiler));

        window->show();

        // ... and enter the event loop:
        for (;;)
        {
            MSG message;

            int result = PeekMessageW(&message, NULL, 0, 0, PM_REMOVE);
            if (result != 0)
            {
                if (message.message == WM_QUIT)
                {
                    return (int)message.wParam;
                }

                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            else
            {
                // Whenever we don't have Windows events to process, we render a frame.
                if (gOptions.shaderType == Options::ShaderProgramType::Compute)
                {
                    app.runCompute();
                }
                else
                {
                    static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
                    renderer->setClearColor(kClearColor);
                    renderer->clearFrame();

                    app.renderFrame();
                }
                // If we are in a mode where output is requested, we need to snapshot the back buffer here
                if (gOptions.outputPath)
                {
                    // Submit the work
                    renderer->submitGpuWork();
                    // Wait until everything is complete
                    renderer->waitForGpu();

                    if (gOptions.shaderType == Options::ShaderProgramType::Compute || gOptions.shaderType == Options::ShaderProgramType::GraphicsCompute)
                    {
                        SLANG_RETURN_ON_FAIL(app.writeBindingOutput(gOptions.outputPath));
                    }
                    else
                    {
                        SlangResult res = app.writeScreen(gOptions.outputPath);

                        if (SLANG_FAILED(res))
                        {
                            fprintf(stderr, "ERROR: failed to write screen capture to file\n");
                            return res;
                        }
                    }
                    return SLANG_OK;
                }

                renderer->presentFrame();
            }
        }
    }

    return SLANG_OK;
}

SlangResult RenderTestTool::calcTestRequirements(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv, TestRequirements* ioRequirements)
{
    CommandLineInfo commandLineInfo;
    SLANG_RETURN_ON_FAIL(_parseCommandLine(stdWriters, argc, argv, &commandLineInfo));

    if (commandLineInfo.passThrough == SLANG_PASS_THROUGH_NONE)
    {
        // Work out backends needed based on the target
        ioRequirements->addUsedBackends(TestToolUtil::getBackendFlagsForTarget(commandLineInfo.target));
    }
    else
    {
        ioRequirements->addUsed(TestToolUtil::toBackendTypeFromPassThroughType(commandLineInfo.passThrough));
    }

    auto rendererType = gOptions.rendererType;

    // Add the render api used
    ioRequirements->addUsed(rendererType);

    // Set the explicit render API (if not already set or different)
    if (rendererType != RenderApiType::Unknown)
    {
        // There should be only one explicit api
        SLANG_ASSERT(ioRequirements->explicitRenderApi == RenderApiType::Unknown || ioRequirements->explicitRenderApi == rendererType);
        // Set the explicitly set render api
        ioRequirements->explicitRenderApi = rendererType;
    }

    return SLANG_OK;
}

} //  namespace renderer_test

SLANG_TEST_TOOL_API SlangResult getTestTool(const Slang::Guid& iid, ISlangUnknown** outUnk)
{
    return renderer_test::RenderTestTool::getSingleton()->queryInterface(iid, (void**)outUnk);
}

int main(int argc, char**  argv)
{
    using namespace Slang;
    SlangSession* session = spCreateSession(nullptr);

    auto stdWriters = GlobalWriters::initDefaultSingleton();
    
    SlangResult res = renderer_test::RenderTestTool::getSingleton()->run(stdWriters, session, argc, argv);
    spDestroySession(session);

	return (int)TestToolUtil::getReturnCode(res);
}


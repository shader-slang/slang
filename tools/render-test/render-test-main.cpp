// render-test-main.cpp

#define _CRT_SECURE_NO_WARNINGS 1

#include "options.h"
#include "render.h"

#include "slang-support.h"
#include "surface.h"
#include "png-serialize-util.h"

#include "shader-renderer-util.h"

#include "../source/core/slang-io.h"

#include "core/slang-token-reader.h"

#include "shader-input-layout.h"
#include <stdio.h>
#include <stdlib.h>

#include "window.h"

#include "../../source/core/slang-test-tool-util.h"

#include "cpu-compute-util.h"

#if RENDER_TEST_CUDA
#   include "cuda/cuda-compute-util.h"
#endif

namespace renderer_test {

using Slang::Result;

int gWindowWidth = 1024;
int gWindowHeight = 768;

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

static void _outputProfileTime(uint64_t startTicks, uint64_t endTicks)
{
    WriterHelper out = StdWriters::getOut();
    double time = double(endTicks - startTicks) / ProcessUtil::getClockFrequency();
    out.print("profile-time=%g\n", time);
}

class RenderTestApp : public WindowListener
{
	public:

    // WindowListener
    virtual Result update(Window* window) SLANG_OVERRIDE;

        // At initialization time, we are going to load and compile our Slang shader
        // code, and then create the API objects we need for rendering.
    Result initialize(SlangSession* session, Renderer* renderer, const Options& options, const ShaderCompilerUtil::Input& input);
    void runCompute();
    void renderFrame();
    void finalize();

	BindingStateImpl* getBindingState() const { return m_bindingState; }

    Result writeBindingOutput(BindRoot* bindRoot, const char* fileName);

    Result writeScreen(const char* filename);

	protected:
		/// Called in initialize
	Result _initializeShaders(SlangSession* session, Renderer* renderer, Options::ShaderProgramType shaderType, const ShaderCompilerUtil::Input& input);

    uint64_t m_startTicks;

	// variables for state to be used for rendering...
	uintptr_t m_constantBufferSize;

	RefPtr<Renderer> m_renderer;

	RefPtr<BufferResource>	m_constantBuffer;
	RefPtr<InputLayout>     m_inputLayout;
	RefPtr<BufferResource>  m_vertexBuffer;
	RefPtr<ShaderProgram>   m_shaderProgram;
    RefPtr<PipelineState>   m_pipelineState;
	RefPtr<BindingStateImpl>    m_bindingState;

    ShaderCompilerUtil::OutputAndLayout m_compilationOutput;

	ShaderInputLayout m_shaderInputLayout;              ///< The binding layout
    int m_numAddedConstantBuffers;                      ///< Constant buffers can be added to the binding directly. Will be added at the end.

    Options m_options;
};

SlangResult RenderTestApp::initialize(SlangSession* session, Renderer* renderer, const Options& options, const ShaderCompilerUtil::Input& input)
{
    m_options = options;

    SLANG_RETURN_ON_FAIL(_initializeShaders(session, renderer, options.shaderType, input));

    m_numAddedConstantBuffers = 0;
	m_renderer = renderer;

    // TODO(tfoley): use each API's reflection interface to query the constant-buffer size needed
    m_constantBufferSize = 16 * sizeof(float);

    BufferResource::Desc constantBufferDesc;
    constantBufferDesc.init(m_constantBufferSize);
    constantBufferDesc.cpuAccessFlags = Resource::AccessFlag::Write;

    m_constantBuffer = renderer->createBufferResource(Resource::Usage::ConstantBuffer, constantBufferDesc);
    if (!m_constantBuffer)
        return SLANG_FAIL;

    //! Hack -> if doing a graphics test, add an extra binding for our dynamic constant buffer
    //
    // TODO: Should probably be more sophisticated than this - with 'dynamic' constant buffer/s binding always being specified
    // in the test file
    RefPtr<BufferResource> addedConstantBuffer;
    switch(m_options.shaderType)
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
        switch(m_options.shaderType)
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

Result RenderTestApp::_initializeShaders(SlangSession* session, Renderer* renderer, Options::ShaderProgramType shaderType, const ShaderCompilerUtil::Input& input)
{
    SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, gOptions, input,  m_compilationOutput));
    m_shaderInputLayout = m_compilationOutput.layout;
    m_shaderProgram = renderer->createProgram(m_compilationOutput.output.desc);
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

    m_startTicks = ProcessUtil::getClockTick();

	m_renderer->dispatchCompute(m_options.computeDispatchSize[0], m_options.computeDispatchSize[1], m_options.computeDispatchSize[2]);
}

void RenderTestApp::finalize()
{
}

Result RenderTestApp::writeBindingOutput(BindRoot* bindRoot, const char* fileName)
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
    FileWriter writer(f, WriterFlags(0));

    for(auto binding : m_bindingState->outputBindings)
    {
        auto i = binding.entryIndex;
        const auto& layoutBinding = m_shaderInputLayout.entries[i];

        assert(layoutBinding.isOutput);
        
        if (binding.resource && binding.resource->isBuffer())
        {
            BufferResource* bufferResource = static_cast<BufferResource*>(binding.resource.Ptr());
            const size_t bufferSize = bufferResource->getDesc().sizeInBytes;

            unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
            if (!ptr)
            {
                return SLANG_FAIL;
            }

            const SlangResult res = ShaderInputLayout::writeBinding(bindRoot, m_shaderInputLayout.entries[i], ptr, bufferSize, &writer);

            m_renderer->unmap(bufferResource);

            SLANG_RETURN_ON_FAIL(res);
        }
        else
        {
            printf("invalid output type at %d.\n", int(i));
        }
    }
    
    return SLANG_OK;
}


Result RenderTestApp::writeScreen(const char* filename)
{
    Surface surface;
    SLANG_RETURN_ON_FAIL(m_renderer->captureScreenSurface(surface));
    return PngSerializeUtil::write(filename, surface);
}

Result RenderTestApp::update(Window* window)
{
    // Whenever we don't have Windows events to process, we render a frame.
    if (m_options.shaderType == Options::ShaderProgramType::Compute)
    {
        runCompute();
    }
    else
    {
        static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
        m_renderer->setClearColor(kClearColor);
        m_renderer->clearFrame();

        renderFrame();
    }

    // If we are in a mode where output is requested, we need to snapshot the back buffer here
    if (m_options.outputPath || m_options.performanceProfile)
    {
        // Submit the work
        m_renderer->submitGpuWork();
        // Wait until everything is complete
        m_renderer->waitForGpu();

        if (m_options.performanceProfile)
        {
            // It might not be enough on some APIs to 'waitForGpu' to mean the computation has completed. Let's lock an output
            // buffer to be sure
            if (m_bindingState->outputBindings.getCount() > 0)
            {
                const auto& binding = m_bindingState->outputBindings[0];
                auto i = binding.entryIndex;
                const auto& layoutBinding = m_shaderInputLayout.entries[i];

                assert(layoutBinding.isOutput);
                
                if (binding.resource && binding.resource->isBuffer())
                {
                    BufferResource* bufferResource = static_cast<BufferResource*>(binding.resource.Ptr());
                    const size_t bufferSize = bufferResource->getDesc().sizeInBytes;
                    unsigned int* ptr = (unsigned int*)m_renderer->map(bufferResource, MapFlavor::HostRead);
                    if (!ptr)
                    {                            
                        return SLANG_FAIL;
                    }
                    m_renderer->unmap(bufferResource);
                }
            }

            // Note we don't do the same with screen rendering -> as that will do a lot of work, which may swamp any computation
            // so can only really profile compute shaders at the moment

            const uint64_t endTicks = ProcessUtil::getClockTick();

            _outputProfileTime(m_startTicks, endTicks);
        }

        if (gOptions.outputPath)
        {
            if (gOptions.shaderType == Options::ShaderProgramType::Compute || gOptions.shaderType == Options::ShaderProgramType::GraphicsCompute)
            {
                auto request = m_compilationOutput.output.request;
                auto slangReflection = (slang::ShaderReflection*) spGetReflection(request);

                BindSet bindSet;
                GPULikeBindRoot bindRoot;
                bindRoot.init(&bindSet, slangReflection, 0);

                BindRoot* outputBindRoot = gOptions.outputUsingType ? &bindRoot : nullptr;

                SLANG_RETURN_ON_FAIL(writeBindingOutput(outputBindRoot, gOptions.outputPath));
            }
            else
            {
                SlangResult res = writeScreen(gOptions.outputPath);
                if (SLANG_FAILED(res))
                {
                    fprintf(stderr, "ERROR: failed to write screen capture to file\n");
                    return res;
                }
            }
        }
        // We are done
        window->postQuit();
        return SLANG_OK;
    }

    m_renderer->presentFrame();
    return SLANG_OK;
}



} //  namespace renderer_test

static SlangResult _innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argcIn, const char*const* argvIn)
{
    using namespace renderer_test;
    using namespace Slang;

    StdWriters::setSingleton(stdWriters);

	// Parse command-line options
	SLANG_RETURN_ON_FAIL(parseOptions(argcIn, argvIn, StdWriters::getError()));

    // Declare window pointer before renderer, such that window is released after renderer
    RefPtr<renderer_test::Window> window;

    ShaderCompilerUtil::Input input;
    
    input.profile = "";
    input.target = SLANG_TARGET_NONE;
    input.args = &gOptions.slangArgs[0];
    input.argCount = gOptions.slangArgCount;

	SlangSourceLanguage nativeLanguage = SLANG_SOURCE_LANGUAGE_UNKNOWN;
	SlangPassThrough slangPassThrough = SLANG_PASS_THROUGH_NONE;
    char const* profileName = "";
	switch (gOptions.rendererType)
	{
		case RendererType::DirectX11:
			input.target = SLANG_DXBC;
            input.profile = "sm_5_0";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            slangPassThrough = SLANG_PASS_THROUGH_FXC;
            
			break;

		case RendererType::DirectX12:
			input.target = SLANG_DXBC;
            input.profile = "sm_5_0";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_HLSL;
            slangPassThrough = SLANG_PASS_THROUGH_FXC;
            
            if( gOptions.useDXIL )
            {
                input.target = SLANG_DXIL;
                input.profile = "sm_6_0";
                slangPassThrough = SLANG_PASS_THROUGH_DXC;
            }
			break;

		case RendererType::OpenGl:
			input.target = SLANG_GLSL;
            input.profile = "glsl_430";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            slangPassThrough = SLANG_PASS_THROUGH_GLSLANG;
			break;

		case RendererType::Vulkan:
			input.target = SLANG_SPIRV;
            input.profile = "glsl_430";
			nativeLanguage = SLANG_SOURCE_LANGUAGE_GLSL;
            slangPassThrough = SLANG_PASS_THROUGH_GLSLANG;
			break;
        case RendererType::CPU:
            input.target = SLANG_HOST_CALLABLE;
            input.profile = "";
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CPP;
            slangPassThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
            break;
        case RendererType::CUDA:
            input.target = SLANG_PTX;
            input.profile = "";
            nativeLanguage = SLANG_SOURCE_LANGUAGE_CUDA;
            slangPassThrough = SLANG_PASS_THROUGH_NVRTC;
            break;

		default:
			fprintf(stderr, "error: unexpected\n");
			return SLANG_FAIL;
	}

    switch (gOptions.inputLanguageID)
    {
        case Options::InputLanguageID::Slang:
            input.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
            input.passThrough = SLANG_PASS_THROUGH_NONE;
            break;

        case Options::InputLanguageID::Native:
            input.sourceLanguage = nativeLanguage;
            input.passThrough = slangPassThrough;
            break;

        default:
            break;
    }

    switch( gOptions.shaderType )
    {
    case Options::ShaderProgramType::Graphics:
    case Options::ShaderProgramType::GraphicsCompute:
        input.pipelineType = PipelineType::Graphics;
        break;

    case Options::ShaderProgramType::Compute:
        input.pipelineType = PipelineType::Compute;
        break;

    case Options::ShaderProgramType::RayTracing:
        input.pipelineType = PipelineType::RayTracing;
        break;

    default:
        break;
    }

    if (gOptions.sourceLanguage != SLANG_SOURCE_LANGUAGE_UNKNOWN)
    {
        input.sourceLanguage = gOptions.sourceLanguage;

        if (input.sourceLanguage == SLANG_SOURCE_LANGUAGE_C || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
        {
            input.passThrough = SLANG_PASS_THROUGH_GENERIC_C_CPP;
        }
    }

    // Use the profile name set on options if set
    input.profile = gOptions.profileName ? gOptions.profileName : input.profile;

    StringBuilder rendererName;
    rendererName << "[" << RendererUtil::toText(gOptions.rendererType) << "] ";
    if (gOptions.adapter.getLength())
    {
        rendererName << "'" << gOptions.adapter << "'";
    }

    if (gOptions.onlyStartup)
    {
        switch (gOptions.rendererType)
        {
            case RendererType::CUDA:
            {
#if RENDER_TEST_CUDA
                return SLANG_SUCCEEDED(spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_NVRTC)) && CUDAComputeUtil::canCreateDevice() ? SLANG_OK : SLANG_FAIL;
#else
                return SLANG_FAIL;
#endif
            }
            case RendererType::CPU:
            {
                // As long as we have CPU, then this should work
                return spSessionCheckPassThroughSupport(session, SLANG_PASS_THROUGH_GENERIC_C_CPP);
            }
            default: break;
        }
    }

    // If it's CPU testing we don't need a window or a renderer
    if (gOptions.rendererType == RendererType::CPU)
    {
        // Check we have all the required features
        for (const auto& renderFeature : gOptions.renderFeatures)
        {
            if (!CPUComputeUtil::hasFeature(renderFeature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }

        ShaderCompilerUtil::OutputAndLayout compilationAndLayout;
        SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, gOptions, input, compilationAndLayout));

        {
            // Get the shared library -> it contains the executable code, we need to keep around if we recompile
            ComPtr<ISlangSharedLibrary> sharedLibrary;
            SLANG_RETURN_ON_FAIL(spGetEntryPointHostCallable(compilationAndLayout.output.request, 0, 0, sharedLibrary.writeRef()));

            // This is a hack to work around, reflection when compiling straight C/C++ code. In that case the code is just passed
            // straight through to the C++ compiler so no reflection. In these tests though we should have conditional code
            // (performance-profile.slang for example), such that there is both a slang and C++ code, and it is the job
            // of the test implementer to *ensure* that the straight C++ code has the same layout as the slang C++ backend.
            //
            // If we are running c/c++ we still need binding information, so compile again as slang source
            if (gOptions.sourceLanguage == SLANG_SOURCE_LANGUAGE_C || input.sourceLanguage == SLANG_SOURCE_LANGUAGE_CPP)
            {
                ShaderCompilerUtil::Input slangInput = input;
                slangInput.sourceLanguage = SLANG_SOURCE_LANGUAGE_SLANG;
                slangInput.passThrough = SLANG_PASS_THROUGH_NONE;
                // We just want CPP, so we get suitable reflection
                slangInput.target = SLANG_CPP_SOURCE;

                SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, gOptions, slangInput, compilationAndLayout));
            }

            // calculate binding
            CPUComputeUtil::Context context;
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcBindings(compilationAndLayout, context));

            // Get the execution info from the lib
            CPUComputeUtil::ExecuteInfo info;
            SLANG_RETURN_ON_FAIL(CPUComputeUtil::calcExecuteInfo(CPUComputeUtil::ExecuteStyle::GroupRange, sharedLibrary, gOptions.computeDispatchSize, compilationAndLayout, context, info));

            const uint64_t startTicks = ProcessUtil::getClockTick();

            SLANG_RETURN_ON_FAIL(CPUComputeUtil::execute(info));

            if (gOptions.performanceProfile)
            {
                const uint64_t endTicks = ProcessUtil::getClockTick();
                _outputProfileTime(startTicks, endTicks);
            }

            if (gOptions.outputPath)
            {
                BindRoot* outputBindRoot = gOptions.outputUsingType ? &context.m_bindRoot : nullptr;


                // Dump everything out that was written
                SLANG_RETURN_ON_FAIL(ShaderInputLayout::writeBindings(outputBindRoot, compilationAndLayout.layout, context.m_buffers, gOptions.outputPath));

                // Check all execution styles produce the same result
                SLANG_RETURN_ON_FAIL(CPUComputeUtil::checkStyleConsistency(sharedLibrary, gOptions.computeDispatchSize, compilationAndLayout));
            }
        }

        return SLANG_OK;
    }

    if (gOptions.rendererType == RendererType::CUDA)
    {        
#if RENDER_TEST_CUDA
        // Check we have all the required features
        for (const auto& renderFeature : gOptions.renderFeatures)
        {
            if (!CUDAComputeUtil::hasFeature(renderFeature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }

        ShaderCompilerUtil::OutputAndLayout compilationAndLayout;
        SLANG_RETURN_ON_FAIL(ShaderCompilerUtil::compileWithLayout(session, gOptions, input, compilationAndLayout));

        const uint64_t startTicks = ProcessUtil::getClockTick();

        CUDAComputeUtil::Context context;
        SLANG_RETURN_ON_FAIL(CUDAComputeUtil::execute(compilationAndLayout, gOptions.computeDispatchSize, context));

        if (gOptions.performanceProfile)
        {
            const uint64_t endTicks = ProcessUtil::getClockTick();
            _outputProfileTime(startTicks, endTicks);
        }

        if (gOptions.outputPath)
        {
            BindRoot* outputBindRoot = gOptions.outputUsingType ? &context.m_bindRoot : nullptr;

            // Dump everything out that was written
            SLANG_RETURN_ON_FAIL(ShaderInputLayout::writeBindings(outputBindRoot, compilationAndLayout.layout, context.m_buffers, gOptions.outputPath));
        }

        return SLANG_OK;
#else
        return SLANG_FAIL;
#endif
    }

    Slang::RefPtr<Renderer> renderer;
    {
        RendererUtil::CreateFunc createFunc = RendererUtil::getCreateFunc(gOptions.rendererType);
        if (createFunc)
        {
            renderer = createFunc();
        }

        if (!renderer)
        {
            if (!gOptions.onlyStartup)
            {
                fprintf(stderr, "Unable to create renderer %s\n", rendererName.getBuffer());
            }
            return SLANG_FAIL;
        }

        Renderer::Desc desc;
        desc.width = gWindowWidth;
        desc.height = gWindowHeight;
        desc.adapter = gOptions.adapter;

        window = renderer_test::Window::create();
        SLANG_RETURN_ON_FAIL(window->initialize(gWindowWidth, gWindowHeight));

        SlangResult res = renderer->initialize(desc, window->getHandle());
        if (SLANG_FAILED(res))
        {
            if (!gOptions.onlyStartup)
            {
                fprintf(stderr, "Unable to initialize renderer %s\n", rendererName.getBuffer());
            }
            return res;
        }

        for (const auto& feature : gOptions.renderFeatures)
        {
            // If doesn't have required feature... we have to give up
            if (!renderer->hasFeature(feature.getUnownedSlice()))
            {
                return SLANG_E_NOT_AVAILABLE;
            }
        }
    }
   
    // If the only test is we can startup, then we are done
    if (gOptions.onlyStartup)
    {
        return SLANG_OK;
    }

	{
		RefPtr<RenderTestApp> app(new RenderTestApp);
		SLANG_RETURN_ON_FAIL(app->initialize(session, renderer, gOptions, input));
        window->show();
        return window->runLoop(app);
	}
}

SLANG_TEST_TOOL_API SlangResult innerMain(Slang::StdWriters* stdWriters, SlangSession* session, int argcIn, const char*const* argvIn)
{
    using namespace Slang;

    SlangResult res = SLANG_FAIL;
    try
    {
        res = _innerMain(stdWriters, session, argcIn, argvIn);
    }
    catch (const Slang::Exception& exception)
    {
        stdWriters->getOut().put(exception.Message.getUnownedSlice());
        return SLANG_FAIL;
    }
    catch (...)
    {
        stdWriters->getOut().put(UnownedStringSlice::fromLiteral("Unhandled exception"));
        return SLANG_FAIL;
    }

    return res;
}

int main(int argc, char**  argv)
{
    using namespace Slang;
    SlangSession* session = spCreateSession(nullptr);

    TestToolUtil::setSessionDefaultPrelude(argv[0], session);
    
    auto stdWriters = StdWriters::initDefaultSingleton();
    
    SlangResult res = innerMain(stdWriters, session, argc, argv);
    spDestroySession(session);

	return (int)TestToolUtil::getReturnCode(res);
}


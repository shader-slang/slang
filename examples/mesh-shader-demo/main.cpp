// main.cpp

// This example demonstrates mesh shaders using Slang, showing a rotating triangle
// that rotates based on time. It's based on the shader-toy example structure
// but adapted for mesh shader pipeline.

#include "slang-com-ptr.h"
#include "slang.h"
using Slang::ComPtr;

#include "core/slang-basic.h"
#include "examples/example-base/example-base.h"
#include "platform/performance-counter.h"
#include "platform/window.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"

#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

static const ExampleResources resourceBase("mesh-shader-demo");

using namespace rhi;

// The application struct for our mesh shader demo
struct MeshShaderApp : public WindowedAppBase
{
    // Uniform data structure that matches the shader
    // Must match UniformBufferObject in rotating-triangle.slang
    struct Uniforms
    {
        float time;
        float colors[3][3]; // Array of 3 float3 colors for the triangle vertices
    };


    // Load and compile the mesh shader program
    Result loadShaderProgram(
        IDevice* device,
        ComPtr<IShaderProgram>& outShaderProgram,
        slang::ProgramLayout*& slangReflection)
    {
        // Get Slang compilation session from the graphics device
        ComPtr<slang::ISession> slangSession;
        slangSession = device->getSlangSession();

        // Load the mesh shader module
        ComPtr<slang::IBlob> diagnosticsBlob;
        Slang::String meshShaderPath = resourceBase.resolveResource("rotating-triangle.slang");
        slang::IModule* module =
            slangSession->loadModule(meshShaderPath.getBuffer(), diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!module)
            return SLANG_FAIL;

        // Find the mesh shader entry point
        char const* meshEntryPointName = "main";
        ComPtr<slang::IEntryPoint> meshEntryPoint;
        SLANG_RETURN_ON_FAIL(
            module->findEntryPointByName(meshEntryPointName, meshEntryPoint.writeRef()));

        // Find the fragment shader entry point
        char const* fragmentEntryPointName = "fragmentMain";
        ComPtr<slang::IEntryPoint> fragmentEntryPoint;
        SLANG_RETURN_ON_FAIL(
            module->findEntryPointByName(fragmentEntryPointName, fragmentEntryPoint.writeRef()));

        // Build component types list (module + entry points)
        Slang::List<slang::IComponentType*> componentTypes;
        componentTypes.add(module);

        // Record entry point indices for later use
        int entryPointCount = 0;
        int meshEntryPointIndex = entryPointCount++;
        componentTypes.add(meshEntryPoint);

        int fragmentEntryPointIndex = entryPointCount++;
        componentTypes.add(fragmentEntryPoint);

        // Create composite component type
        ComPtr<slang::IComponentType> composedProgram;
        SlangResult result = slangSession->createCompositeComponentType(
            componentTypes.getBuffer(),
            componentTypes.getCount(),
            composedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);

        // Link the program
        ComPtr<slang::IComponentType> linkedProgram;
        result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);

        // Create shader program
        ShaderProgramDesc programDesc = {};
        programDesc.slangGlobalScope = linkedProgram.get();
        auto shaderProgram = device->createShaderProgram(programDesc);
        outShaderProgram = shaderProgram;

        // Get the program layout
        slangReflection = linkedProgram->getLayout();
        return shaderProgram ? SLANG_OK : SLANG_FAIL;
        ;
    }

    ComPtr<IShaderProgram> gShaderProgram;
    slang::ProgramLayout* slangReflection = nullptr;
    ComPtr<IRenderPipeline> gPipeline;

    bool firstTime = true;
    platform::TimePoint startTime;

    Result initialize()
    {
        SLANG_RETURN_ON_FAIL(initializeBase("Mesh Shader Demo", 1024, 768, DeviceType::Vulkan));

        // Load mesh shader program
        SLANG_RETURN_ON_FAIL(loadShaderProgram(gDevice, gShaderProgram, slangReflection));

        // Create render pipeline for mesh shaders
        ColorTargetDesc colorTarget;
        colorTarget.format = Format::RGBA8Unorm;
        
        RenderPipelineDesc desc;
        desc.program = gShaderProgram;
        desc.targetCount = 1;
        desc.targets = &colorTarget;
        desc.depthStencil.depthTestEnable = false;
        desc.depthStencil.depthWriteEnable = false;
        desc.primitiveTopology = PrimitiveTopology::TriangleList;
        // Note: No input layout needed for mesh shaders - they generate their own vertices
        
        ComPtr<IRenderPipeline> pipeline;
        Result result = gDevice->createRenderPipeline(desc, pipeline.writeRef());
        if (SLANG_FAILED(result))
        {
            // Log to debug output (visible in Visual Studio Output window)
#ifdef _WIN32
            char errorMsg[256];
            sprintf_s(errorMsg, "ERROR: Failed to create render pipeline. Result code: 0x%08X\n", result);
            OutputDebugStringA(errorMsg);
#endif
            // Also use the existing diagnostic system
            fprintf(stderr, "ERROR: Failed to create render pipeline. Result code: 0x%08X\n", result);
            return result;
        }
        gPipeline = pipeline;

        return SLANG_OK;
    }

    virtual void renderFrame(ITexture* texture) override
    {
        
        if (firstTime)
        {
            startTime = platform::PerformanceCounter::now();
            firstTime = false;
        }

        // Update uniform buffer with current time and dynamic colors
        Uniforms uniforms = {};
        float currentTime = platform::PerformanceCounter::getElapsedTimeInSeconds(startTime);
        uniforms.time = currentTime;

        // Create smoothly changing colors based on time
        // Each vertex gets different phase offsets to create variety
        float colorSpeed = 1.5f; // Speed of color change

        // Vertex 0 (Top) - cycles through warm colors
        uniforms.colors[0][0] = 0.5f + 0.5f * sin(currentTime * colorSpeed);        // Red
        uniforms.colors[0][1] = 0.5f + 0.5f * sin(currentTime * colorSpeed + 2.0f); // Green
        uniforms.colors[0][2] = 0.5f + 0.5f * sin(currentTime * colorSpeed + 4.0f); // Blue

        // Vertex 1 (Bottom Right) - cycles with different phase
        uniforms.colors[1][0] = 0.5f + 0.5f * sin(currentTime * colorSpeed + 2.1f); // Red
        uniforms.colors[1][1] = 0.5f + 0.5f * sin(currentTime * colorSpeed + 4.2f); // Green
        uniforms.colors[1][2] = 0.5f + 0.5f * sin(currentTime * colorSpeed + 0.3f); // Blue

        // Vertex 2 (Bottom Left) - cycles with another different phase
        uniforms.colors[2][0] = 0.5f + 0.5f * sin(currentTime * colorSpeed + 4.3f); // Red
        uniforms.colors[2][1] = 0.5f + 0.5f * sin(currentTime * colorSpeed + 0.4f); // Green
        uniforms.colors[2][2] = 0.5f + 0.5f * sin(currentTime * colorSpeed + 2.5f); // Blue

        // Encode render commands
        ComPtr<ITextureView> textureView = gDevice->createTextureView(texture, {});
        RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = textureView;
        colorAttachment.loadOp = LoadOp::Clear;

        RenderPassDesc renderPass = {};
        renderPass.colorAttachments = &colorAttachment;
        renderPass.colorAttachmentCount = 1;

        // Set up render state
        RenderState renderState = {};
        renderState.viewports[0] = Viewport::fromSize(windowWidth, windowHeight);
        renderState.viewportCount = 1;
        renderState.scissorRects[0] = ScissorRect::fromSize(windowWidth, windowHeight);
        renderState.scissorRectCount = 1;

        // Bind pipeline and set uniforms and color buffer
        ComPtr<IShaderObject> cbObject;
        {
            gDevice->createShaderObject(
                nullptr,
                slangReflection->findTypeByName("UniformBufferObject"),
                ShaderObjectContainerType::None,
                cbObject.writeRef());
            ShaderCursor cursor(cbObject);
            cursor["time"].setData(uniforms.time);
            cursor["colors"].setData(uniforms.colors);
            cbObject->finalize();
        }

        ComPtr<IShaderObject> rootObject = gDevice->createRootShaderObject(gShaderProgram);
        ShaderCursor rootCursor(rootObject);
        rootCursor["ubo"].setObject(cbObject);

        // We have done all the set up work, now it is time to start recording a command buffer for
        // GPU execution.
        auto commandEncoder = gQueue->createCommandEncoder();
        auto passEncoder = commandEncoder->beginRenderPass(renderPass);
        passEncoder->bindPipeline(gPipeline, rootObject);

        passEncoder->setRenderState(renderState);

        // For mesh shaders, we use draw without vertex buffers
        // The mesh shader generates its own geometry
        DrawArguments drawArgs = {};
        drawArgs.vertexCount = 3;  // This is ignored by mesh shaders, but required by API
        passEncoder->draw(drawArgs);

        passEncoder->end();
        gQueue->submit(commandEncoder->finish());
        gQueue->waitOnHost();

        if (!isTestMode())
        {
            gSurface->present();
        }
    }
};

// Main function instantiation
EXAMPLE_MAIN(innerMain<MeshShaderApp>); 

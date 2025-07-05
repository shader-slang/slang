// main.cpp

#include "../../source/core/slang-io.h"
#include "GFSDK_Aftermath.h"
#include "GFSDK_Aftermath_GpuCrashDump.h"
#include "core/slang-basic.h"
#include "examples/example-base/example-base.h"
#include "platform/window.h"
#include "slang-com-ptr.h"
#include "slang.h"

#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>

using namespace rhi;
using namespace Slang;

static const ExampleResources resourceBase("nv-aftermath-example");

// This example is based on the "triangle" sample.
//
// This examples purpose is to show how to use the aftermath SDK to capture
// a crash dump.
//
// * [nsight aftermath](https://developer.nvidia.com/nsight-aftermath)
//
// In addition it uses obfuscation and source maps to allow source level
// debugging via aftermath even with obfuscation.
//
// * [obfuscation](https://github.com/shader-slang/slang/blob/master/docs/user-guide/a1-03-obfuscation.md)
// * [source map](https://github.com/source-map/source-map-spec)

struct Vertex
{
    float position[3];
    float color[3];
};

static const int kVertexCount = 3;
static const Vertex kVertexData[kVertexCount] = {
    {{0, 0, 0.5}, {1, 0, 0}},
    {{0, 1, 0.5}, {0, 0, 1}},
    {{1, 0, 0.5}, {0, 1, 0}},
};

struct AftermathCrashExample : public WindowedAppBase
{
    void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob);

    Result loadShaderProgram(IDevice* device, IShaderProgram** outProgram);

    virtual void renderFrame(ITexture* texture) override;

    void onAftermathCrash(const void* data, const uint32_t dataSizeInBytes);

    void onAftermathDebugInfo(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);

    void onAftermathCrashDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription description);

    void onAftermathMarker(const void* pMarker, void** resolvedMarkerData, uint32_t* markerSize);

    ComPtr<IRenderPipeline> m_renderPipeline;
    ComPtr<IBuffer> m_vertexBuffer;

    /// A counter such that we can make aftermath dump file names unique
    std::atomic<int> m_uniqueId = 0;

    Slang::Result initialize()
    {
        // Defer shader debug information callbacks until an actual GPU crash dump
        // is generated. Increases memory footprint.
        const uint32_t aftermathFeatureFlags =
            GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks;

        // As per docs must be called before any device is created
        GFSDK_Aftermath_EnableGpuCrashDumps(
            GFSDK_Aftermath_Version_API,
            GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX |
                GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
            aftermathFeatureFlags,
            _crashCallback,
            _debugInfoCallback,
            _crashDescriptionCallback,
            _markerCallback,
            this);

        SLANG_RETURN_ON_FAIL(initializeBase("autodiff-texture", 1024, 768, DeviceType::Default));


        // We will create objects needed to configure the "input assembler"
        // (IA) stage of the pipeline.
        //
        // First, we create an input layout:
        //
        InputElementDesc inputElements[] = {
            {"POSITION", 0, Format::RGB32Float, offsetof(Vertex, position)},
            {"COLOR", 0, Format::RGB32Float, offsetof(Vertex, color)},
        };
        auto inputLayout = gDevice->createInputLayout(sizeof(Vertex), &inputElements[0], 2);
        if (!inputLayout)
            return SLANG_FAIL;

        // Next we allocate a vertex buffer for our pre-initialized
        // vertex data.
        //
        BufferDesc vertexBufferDesc;
        vertexBufferDesc.size = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.usage = BufferUsage::VertexBuffer;
        vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
        m_vertexBuffer = gDevice->createBuffer(vertexBufferDesc, &kVertexData[0]);
        if (!m_vertexBuffer)
            return SLANG_FAIL;

        // Now we will use our `loadShaderProgram` function to load
        // the code from `shaders.slang` into the graphics API.
        //
        ComPtr<IShaderProgram> shaderProgram;
        SLANG_RETURN_ON_FAIL(loadShaderProgram(device, shaderProgram.writeRef()));

        // Following the D3D12/Vulkan style of API, we need a pipeline state object
        // (PSO) to encapsulate the configuration of the overall graphics pipeline.
        //
        ColorTargetDesc colorTarget;
        colorTarget.format = Format::RGBA8Unorm;
        RenderPipelineDesc desc;
        desc.inputLayout = inputLayout;
        desc.program = shaderProgram;
        desc.targetCount = 1;
        desc.targets = &colorTarget;
        desc.depthStencil.depthTestEnable = false;
        desc.depthStencil.depthWriteEnable = false;
        desc.primitiveTopology = PrimitiveTopology::TriangleList;
        auto pipelineState = gDevice->createRenderPipeline(desc);
        if (!pipelineState)
            return SLANG_FAIL;

        m_renderPipeline = pipelineState;

        return SLANG_OK;
    }
};

void AftermathCrashExample::diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        printf("%s", (const char*)diagnosticsBlob->getBufferPointer());
    }
}

void AftermathCrashExample::onAftermathCrash(const void* data, const uint32_t dataSizeInBytes)
{
    // NOTE! This method can be called from *any* thread.
    const auto id = m_uniqueId++;

    // Dump out as a file
    Slang::StringBuilder filename;
    filename << "aftermath-dump-" << id << ".bin";

    File::writeAllBytes(filename, data, dataSizeInBytes);

    // SLANG_BREAKPOINT(0);
}

void AftermathCrashExample::onAftermathDebugInfo(
    const void* gpuCrashDump,
    const uint32_t gpuCrashDumpSize)
{
    const auto id = m_uniqueId++;

    // Dump out as a file
    Slang::StringBuilder filename;
    filename << "aftermath-debug-info-" << id << ".bin";

    File::writeAllBytes(filename, gpuCrashDump, gpuCrashDumpSize);
}

void AftermathCrashExample::onAftermathCrashDescription(
    PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription description)
{
    // Ignore for now
}

void AftermathCrashExample::onAftermathMarker(
    const void* marker,
    void** resolvedMarkerData,
    uint32_t* markerSize)
{
    // Ignore for now
}

struct FileSystemEntry
{
    SlangPathType type; ///< The type of the entry
    String path;        ///< The path to the entr
};

struct CompileProduct
{
    String fileName;         ///< The filename to write the compile product out to
    ComPtr<ISlangBlob> blob; ///< A blob holding the products contents
};

/* Currently the mechanism to access the contents of a compilation that might consist of many
products is through representing the contents as a "file system".

The file system is just a somewhat convenient/simple in memory representation of the compilation
products.

This function transverses the file system and adds everything found into outEntries.
*/
static SlangResult _findFileSystemContents(
    ISlangFileSystemExt* fileSystem,
    const char* rootPath,
    List<FileSystemEntry>& outEntries)
{
    {
        SlangPathType type;
        SLANG_RETURN_ON_FAIL(fileSystem->getPathType(rootPath, &type));
        outEntries.add(FileSystemEntry{type, rootPath});
    }

    // A context used to hold state, when using enumeratePathContents
    struct Context
    {
        List<FileSystemEntry>& entries; // The entries to be accumulated to
        String path;                    // The path being enumerated
    };

    for (Index i = outEntries.getCount() - 1; i < outEntries.getCount(); ++i)
    {
        const auto& entry = outEntries[i];

        // If it's a directory we want to traverse it's contents
        if (entry.type == SLANG_PATH_TYPE_DIRECTORY)
        {
            Context context{outEntries, entry.path};

            fileSystem->enumeratePathContents(
                entry.path.getBuffer(),
                [](SlangPathType pathType, const char* name, void* userData) -> void
                {
                    Context* context = reinterpret_cast<Context*>(userData);

                    const String path = Path::simplify(Path::combine(context->path, name));

                    context->entries.add({pathType, path});
                },
                &context);
        }
    }

    return SLANG_OK;
}

/* This function takes a compile results file system, and finds items that should be written out.

This is somewhat complicated because the names of products from different compilations might have
the same names. So a "prefix" is passed in, and for files that don't have unique names, they are
uniqified via the prefix.

The same product may appear in multiple compilations, for example obfuscated source maps so a
product is not added if there is already a product with the same name */
static SlangResult _addCompileProducts(
    ISlangFileSystemExt* fileSystem,
    const char* prefix,
    List<CompileProduct>& ioProducts)
{
    List<FileSystemEntry> fileSystemEntries;
    SLANG_RETURN_ON_FAIL(_findFileSystemContents(fileSystem, ".", fileSystemEntries));

    for (const auto& fileSystemEntry : fileSystemEntries)
    {
        if (fileSystemEntry.type != SLANG_PATH_TYPE_FILE)
        {
            continue;
        }

        const auto ext = Path::getPathExt(fileSystemEntry.path);

        String outFileName;

        // Some filenames need special handling, and their names are already unique
        // Others will be the same between differen fileSystem that represent the
        // compilation products.
        //
        // Source maps that are obfuscated are unique.
        {
            String inFileName = Path::getFileNameWithoutExt(fileSystemEntry.path);

            // If it's an obfuscated source map, it's name is already unique (it includes the hash)
            const bool isUniqueName =
                (ext == toSlice("map") && inFileName.endsWith(toSlice("-obfuscated")));

            StringBuilder buf;
            // If it's not a uniquename make it unique via the prefix
            if (!isUniqueName)
            {
                // Uniquify with the prefix
                buf << prefix << "-";
            }

            buf << inFileName << "." << ext;
            outFileName = buf;
        }

        // If we have an output filename
        if (outFileName.getLength())
        {
            // And that filename isn't already used
            if (ioProducts.findFirstIndex(
                    [&](const CompileProduct& product) -> bool
                    { return product.fileName == outFileName; }) < 0)
            {
                ComPtr<ISlangBlob> blob;
                SLANG_RETURN_ON_FAIL(
                    fileSystem->loadFile(fileSystemEntry.path.getBuffer(), blob.writeRef()));

                // Add to the results
                ioProducts.add(CompileProduct{outFileName, blob});
            }
        }
    }

    return SLANG_OK;
}

Result AftermathCrashExample::loadShaderProgram(IDevice* device, IShaderProgram** outProgram)
{
    ComPtr<slang::ISession> slangSession;
    slangSession = gDevice->getSlangSession();

    // This is a little bit of a work around.
    //
    // We want to set some options that are only available
    // via processCommandLineArguments, but we need a request to be able to set them up
    // The setting actually sets the parameters on the Linkage, so they will be used for the later
    // actual compilation
    {
        ComPtr<slang::ICompileRequest> request;

        SLANG_RETURN_ON_FAIL(slangSession->createCompileRequest(request.writeRef()));

        // Turn on obfuscation
        //
        // Turns on source map as the line directive, this will lead to an "emit source map"
        // and no #line directives in generated source.
        //
        // It isn't necessary to use the "source-map" line directive mode, and just use
        // #line directives, and have source locations to obfuscated source file directly embedded.
        //
        // To do this replace the line below with
        //
        // ```
        // const char* args[] = { "-obfuscate" };
        // ```
        const char* args[] = {"-obfuscate", "-line-directive-mode", "source-map"};

        request->processCommandLineArguments(args, SLANG_COUNT_OF(args));

        // Enable debug info
        request->setDebugInfoLevel(SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
    }

    ComPtr<slang::IBlob> diagnosticsBlob;
    Slang::String path = resourceBase.resolveResource("shaders.slang");
    slang::IModule* module = slangSession->loadModule(path.getBuffer(), diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!module)
        return SLANG_FAIL;

    // Find the entry points
    ComPtr<slang::IEntryPoint> vertexEntryPoint;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName("vertexMain", vertexEntryPoint.writeRef()));
    //
    ComPtr<slang::IEntryPoint> fragmentEntryPoint;
    SLANG_RETURN_ON_FAIL(
        module->findEntryPointByName("fragmentMain", fragmentEntryPoint.writeRef()));

    // At this point we have a few different Slang API objects that represent
    // pieces of our code: `module`, `vertexEntryPoint`, and `fragmentEntryPoint`.
    //
    // A single Slang module could contain many different entry points (e.g.,
    // four vertex entry points, three fragment entry points, and two compute
    // shaders), and before we try to generate output code for our target API
    // we need to identify which entry points we plan to use together.
    //
    // Modules and entry points are both examples of *component types* in the
    // Slang API. The API also provides a way to build a *composite* out of
    // other pieces, and that is what we are going to do with our module
    // and entry points.
    //
    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(module);

    // Later on when we go to extract compiled kernel code for our vertex
    // and fragment shaders, we will need to make use of their order within
    // the composition, so we will record the relative ordering of the entry
    // points here as we add them.
    int entryPointCount = 0;
    int vertexEntryPointIndex = entryPointCount++;
    componentTypes.add(vertexEntryPoint);

    int fragmentEntryPointIndex = entryPointCount++;
    componentTypes.add(fragmentEntryPoint);

    // Actually creating the composite component type is a single operation
    // on the Slang session, but the operation could potentially fail if
    // something about the composite was invalid (e.g., you are trying to
    // combine multiple copies of the same module), so we need to deal
    // with the possibility of diagnostic output.
    //
    ComPtr<slang::IComponentType> linkedProgram;
    SlangResult result = slangSession->createCompositeComponentType(
        componentTypes.getBuffer(),
        componentTypes.getCount(),
        linkedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    SLANG_RETURN_ON_FAIL(result);

    const Index targetIndex = 0;

    // Trigger compilation by requesting the code.
    // Normally gfx would compile as needed.
    {
        ComPtr<ISlangBlob> code;
        ComPtr<ISlangBlob> diagnostics;

        SLANG_RETURN_ON_FAIL(linkedProgram->getEntryPointCode(
            vertexEntryPointIndex,
            targetIndex,
            code.writeRef(),
            diagnostics.writeRef()));
        SLANG_RETURN_ON_FAIL(linkedProgram->getEntryPointCode(
            fragmentEntryPointIndex,
            targetIndex,
            code.writeRef(),
            diagnostics.writeRef()));
    }

    {
        // We want to find all the compilation products. In particular we want to get the emit
        // source map, and the obfuscated source maps

        List<CompileProduct> compileProducts;

        // The current mechanism for getting access to compilation products other than result
        // blob/diagnostics is to return it as a compilation result "file system".

        ComPtr<ISlangMutableFileSystem> vertexFileSystem;
        SLANG_RETURN_ON_FAIL(linkedProgram->getResultAsFileSystem(
            vertexEntryPointIndex,
            targetIndex,
            vertexFileSystem.writeRef()));

        ComPtr<ISlangMutableFileSystem> fragmentFileSystem;
        SLANG_RETURN_ON_FAIL(linkedProgram->getResultAsFileSystem(
            fragmentEntryPointIndex,
            targetIndex,
            fragmentFileSystem.writeRef()));

        // Add the contents of the compile result file systems into compileProducts
        // Some products might appear in both file systems, so compileProducts is just the unique
        // products. Additionally because some products may have the same name, we pass in a
        // "prefix" to make the products name unique.
        SLANG_RETURN_ON_FAIL(_addCompileProducts(vertexFileSystem, "vertex", compileProducts));
        SLANG_RETURN_ON_FAIL(_addCompileProducts(fragmentFileSystem, "fragment", compileProducts));

        // Now write all of the products out
        for (const auto& product : compileProducts)
        {
            SLANG_RETURN_ON_FAIL(File::writeAllBytes(
                product.fileName,
                product.blob->getBufferPointer(),
                product.blob->getBufferSize()));
        }
    }

    // Once we've described the particular composition of entry points
    // that we want to compile, we defer to the graphics API layer
    // to extract compiled kernel code and load it into the API-specific
    // program representation.
    //
    ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = linkedProgram;
    SLANG_RETURN_ON_FAIL(gDevice->createShaderProgram(programDesc, outProgram));

    return SLANG_OK;
}

static void GFSDK_AFTERMATH_CALL
_crashCallback(const void* gpuCrashDump, const uint32_t gpuCrashDumpSize, void* userData)
{
    reinterpret_cast<AftermathCrashExample*>(userData)->onAftermathCrash(
        gpuCrashDump,
        gpuCrashDumpSize);
}

static void GFSDK_AFTERMATH_CALL
_debugInfoCallback(const void* gpuCrashDump, const uint32_t gpuCrashDumpSize, void* userData)
{
    reinterpret_cast<AftermathCrashExample*>(userData)->onAftermathDebugInfo(
        gpuCrashDump,
        gpuCrashDumpSize);
}

static void GFSDK_AFTERMATH_CALL _crashDescriptionCallback(
    PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription,
    void* userData)
{
    reinterpret_cast<AftermathCrashExample*>(userData)->onAftermathCrashDescription(addDescription);
}

static void GFSDK_AFTERMATH_CALL _markerCallback(
    const void* marker,
    void* pUserData,
    void** resolvedMarkerData,
    uint32_t* markerSize)
{
    reinterpret_cast<AftermathCrashExample*>(pUserData)->onAftermathMarker(
        marker,
        resolvedMarkerData,
        markerSize);
}

void AftermathCrashExample::renderFrame(ITexture* texture)
{
    auto commandEncoder = gQueue->createCommandEncoder();

    ComPtr<ITextureView> textureView = gDevice->createTextureView(texture, {});
    RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = textureView;
    colorAttachment.loadOp = LoadOp::Clear;

    RenderPassDesc renderPass = {};
    renderPass.colorAttachments = &colorAttachment;
    renderPass.colorAttachmentCount = 1;

    auto renderEncoder = commandEncoder->beginRenderPass(renderPass);

    RenderState renderState = {};
    renderState.viewports[0] = Viewport::fromSize(windowWidth, windowHeight);
    renderState.viewportCount = 1;
    renderState.scissorRects[0] = ScissorRect::fromSize(windowWidth, windowHeight);
    renderState.scissorRectCount = 1;

    auto rootObject = renderEncoder->bindPipeline(m_renderPipeline);
    ShaderCursor rootCursor(rootObject);

    rootCursor["Uniforms"]["modelViewProjection"].setData(kIdentity, sizeof(float) * 16);

    // We are going to extra efforts to create a shader that we know will time
    // out because we *want* a GPU "crash", such we can capture via nsight aftermath.
    // The failCount is just a number that is large enough to make things take too long.
    int32_t failCount = 0x3fffffff;
    rootCursor["Uniforms"]["failCount"].setData(&failCount, sizeof(failCount));

    // We also need to set up a few pieces of fixed-function pipeline
    // state that are not bound by the pipeline state above.
    //
    renderState.vertexBuffers[0] = m_vertexBuffer;
    renderState.vertexBufferCount = 1;
    renderEncoder->setRenderState(renderState);

    // Finally, we are ready to issue a draw call for a single triangle.
    //
    DrawArguments drawArgs = {};
    drawArgs.vertexCount = 3;
    renderEncoder->draw(drawArgs);

    renderEncoder->end();
    gQueue->submit(commandEncoder->finish());

    if (!isTestMode())
    {
        // With that, we are done drawing for one frame, and ready for the next.
        //
        gSurface()->present();
    }

    // If the id changes means we have a capture and so can quit.
    // On D3D11, the first present *doesn't* appear to crash.
    if (m_uniqueId != 0)
    {
        platform::Application::quit();
    }
}

// This macro instantiates an appropriate main function to
// run the application defined above.
EXAMPLE_MAIN(innerMain<AftermathCrashExample>)
